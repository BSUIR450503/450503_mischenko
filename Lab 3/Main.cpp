#include "CreateProcess.h"

int main(int argc, char *argv[])
{
#ifdef _WIN32
    DWORD numRead;
    DWORD numToWrite;

    if (argc != 2)
    {
        HANDLE namedPipe = CreateNamedPipe(TEXT("\\\\.\\pipe\\Pipe"),
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            1024, 1024,
            NMPWAIT_USE_DEFAULT_WAIT,
            NULL);

        if (namedPipe == INVALID_HANDLE_VALUE)
        {
            cout << "Can't create pipe. Error. Press any key to exit " << GetLastError();
            _getch();
            exit(0);
        }

        HANDLE serverSemaphore = CreateSemaphore(NULL, 0, 1, TEXT("serverSemaphore"));
        HANDLE clientSemaphore = CreateSemaphore(NULL, 0, 1, TEXT("clientSemaphore"));

        MyProcess *clientProcess = new MyProcess(argv[0]);

        ConnectNamedPipe(namedPipe, NULL);

        WaitForSingleObject(serverSemaphore, INFINITE);

        char *buffer = NULL;
        buffer = (char *)malloc(sizeof(char) * 1024);

        printf("Server process\n");

        if (!WriteFile(namedPipe, "Ready", 1024, &numToWrite, NULL))
            return 0;

        while (1)
        {
            ReleaseSemaphore(clientSemaphore, 1, NULL);
            WaitForSingleObject(serverSemaphore, INFINITE);

            if (ReadFile(namedPipe, buffer, 1024, &numRead, NULL))
                printf("Client message: %s", buffer);

            if (!strcmp(buffer, "exit"))
            {
                CloseHandle(namedPipe);
                CloseHandle(serverSemaphore);
                free(buffer);
                return 0;
            }

            printf("\nInput message to client: ");
            fflush(stdin);
            gets_s(buffer, 1024);

            if (!WriteFile(namedPipe, buffer, 1024, &numToWrite, NULL))
                break;

            ReleaseSemaphore(clientSemaphore, 1, NULL);

            if (!strcmp(buffer, "exit"))
            {
                CloseHandle(namedPipe);
                CloseHandle(serverSemaphore);

                free(buffer);
                return 0;
            }
        }
        return 0;
    }
    else
    {
        HANDLE serverSemaphore = OpenSemaphore(EVENT_ALL_ACCESS, FALSE, TEXT("serverSemaphore"));
        HANDLE clientSemaphore = OpenSemaphore(EVENT_ALL_ACCESS, FALSE, TEXT("clientSemaphore"));

        HANDLE namedPipe = CreateFile(TEXT("\\\\.\\pipe\\Pipe"),
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);

        if (namedPipe == INVALID_HANDLE_VALUE)
        {
            printf("Can't create pipe. Error. Press any key to exit", GetLastError());
            _getch();
            exit(0);
        }

        ReleaseSemaphore(serverSemaphore, 1, NULL);

        char *buffer = NULL;
        buffer = (char *)malloc(sizeof(char) * 1024);

        printf("Client process\n");

        while (1)
        {
            WaitForSingleObject(clientSemaphore, INFINITE);

            if (ReadFile(namedPipe, buffer, 1024, &numRead, NULL))
            {
                if (!strcmp(buffer, "exit"))
                {
                    CloseHandle(clientSemaphore);
                    free(buffer);
                    return 0;
                }

                printf("Server message: %s", buffer);

                char input[1024] = { '\0' };
                cout << "\nInput message to server: ";
                fflush(stdin);
                gets_s(input, 1024);

                if (!WriteFile(namedPipe, input, 1024, &numToWrite, NULL))
                    break;

                if (!strcmp(input, "exit"))
                {
                    ReleaseSemaphore(serverSemaphore, 1, NULL);
                    CloseHandle(clientSemaphore);
                    free(buffer);
                    return 0;
                }
            }
            ReleaseSemaphore(serverSemaphore, 1, NULL);
        }
        return 0;
    }

#elif __linux__
    pid_t pid;
    key_t key = ftok("/home/", 0);

    struct sembuf serverSemaphore;
    struct sembuf clientSemaphore;
    int semaphoreId;

    int sharedMemoryId;
    char *segmentPtr;

    if (argc != 2)
    {
        switch (pid = fork())
        {
        case -1:
            printf("Can't fork process %d\n", pid);
            break;
        case 0:
            execlp("gnome-terminal", "gnome-terminal", "-x", argv[0], "1", NULL);
        default:
            if ((sharedMemoryId = shmget(key, 1024, IPC_CREAT | IPC_EXCL | 0660)) == -1)
            {
                if ((sharedMemoryId = shmget(key, 1024, 0)) == -1)
                {
                    printf("error\n");
                    exit(1);
                }
            }

            if ((segmentPtr = (char*)shmat(sharedMemoryId, NULL, 0)) == (char*)(-1))
            {
                printf("Can't attach shared memory\n");
                exit(1);
            }

            semaphoreId = semget(key, 1, 0666 | IPC_CREAT);

            if (semaphoreId < 0)
            {
                printf("Can't get semaphore\n");
                exit(EXIT_FAILURE);
            }

            if (semctl(semaphoreId, 0, SETVAL, (int)0) < 0)
            {
                printf("Can't initialize semaphore\n");
                exit(EXIT_FAILURE);
            }

            serverSemaphore.sem_num = 0;
            serverSemaphore.sem_op = 0;
            serverSemaphore.sem_flg = 0;

            printf("Server process: ");

            while (1) {
                char *message = NULL;
                message = (char*)malloc(1024 * sizeof(char));

                semop(semaphoreId, &serverSemaphore, 1);

                printf("\nInput message to client: ");
                fflush(stdin);
                cin >> message;

                strcpy(segmentPtr, message);

                serverSemaphore.sem_op = 3;
                semop(semaphoreId, &serverSemaphore, 1);

                serverSemaphore.sem_op = 0;
                semop(semaphoreId, &serverSemaphore, 1);

                if (!strcmp("exit", message))
                    return 0;

                strcpy(message, segmentPtr);
                printf("Client message: %s\n", message);
            }
            return 0;
        }
    }
    else
    {
        if ((sharedMemoryId = shmget(key, 1024, IPC_CREAT | IPC_EXCL | 0660)) == -1)
        {
            if ((sharedMemoryId = shmget(key, 1024, 0)) == -1)
            {
                printf("Shmget error\n");
                exit(1);
            }
        }

        if ((segmentPtr = (char*)shmat(sharedMemoryId, NULL, 0)) == (char*)(-1))
        {
            printf("Can't attach shared memory\n");
            exit(1);
        }


        semaphoreId = semget(key, 1, 0666 | IPC_CREAT);

        if (semaphoreId < 0)
        {
            printf("Semget error");
            exit(EXIT_FAILURE);
        }

        if (semctl(semaphoreId, 0, SETVAL, (int)0) < 0)
        {
            printf("Semctl error");
            exit(EXIT_FAILURE);
        }

        if ((sharedMemoryId = shmget(key, 1024, 0)) == -1)
        {
            printf("Shmget error");
            exit(1);
        }

        printf("Client process: ");

        clientSemaphore.sem_op = -2;

        while (1) {
            char *message = NULL;
            message = (char*)malloc(1024 * sizeof(char));

            semop(semaphoreId, &clientSemaphore, 1);

            strcpy(message, segmentPtr);

            if (!strcmp("exit", message))
            {
                clientSemaphore.sem_op = -1;
                semop(semaphoreId, &clientSemaphore, 1);
                return 0;
            }

            printf("\nServer message: %s", message);

            printf("\nInput message to server: ");
            fflush(stdin);
            cin >> message;

            strcpy(segmentPtr, message);

            clientSemaphore.sem_op = -1;
            semop(semaphoreId, &clientSemaphore, 1);

            clientSemaphore.sem_op = -2;
        }
        return 0;
    }
#endif
}
