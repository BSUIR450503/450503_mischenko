#ifdef _WIN32
#include <windows.h>
#include <process.h>
#include <stdio.h>
#include <iostream>
#endif

#ifdef __linux__
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <dirent.h>
#include <aio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <errno.h> 
#endif

#define READ 0
#define EXIT 1
#define WRITE 2

#ifdef _WIN32
using namespace std;

struct OperationInfo
{
    HANDLE hFile;
    DWORD bytesCount;
    CHAR  buffer[100];
    DWORD  fileOffset;
    OVERLAPPED overlapped;
} operationInfo;

HINSTANCE library;
HANDLE events[3];

DWORD WINAPI reader(PVOID path);
DWORD WINAPI writer(PVOID path);

int main(int argc, char *argv[])
{
    HANDLE hEvent;
    HANDLE hThread[2];
    char filePath[MAX_PATH];

    library = LoadLibrary("Library.dll");

    hEvent = CreateEvent(NULL, FALSE, TRUE, TEXT("event"));

    events[WRITE] = CreateEvent(NULL, FALSE, TRUE, NULL);
    events[READ] = CreateEvent(NULL, FALSE, FALSE, NULL);
    events[EXIT] = CreateEvent(NULL, TRUE, FALSE, NULL);

    operationInfo.bytesCount = sizeof(operationInfo.buffer);
    operationInfo.fileOffset = 0;
    operationInfo.overlapped.Offset = 0;
    operationInfo.overlapped.OffsetHigh = 0;
    operationInfo.overlapped.hEvent = hEvent;

    printf("Enter path ");
    gets_s(filePath, MAX_PATH);

    hThread[0] = CreateThread(NULL, 0, reader, (LPVOID)filePath, 0, NULL);
    hThread[1] = CreateThread(NULL, 0, writer, (LPVOID)filePath, 0, NULL);

    WaitForMultipleObjects(2, hThread, TRUE, INFINITE);

    CloseHandle(hThread[0]);
    CloseHandle(hThread[1]);
    CloseHandle(events[WRITE]);
    CloseHandle(events[READ]);
    CloseHandle(events[EXIT]);
    CloseHandle(hEvent);

    FreeLibrary(library);
    system("Pause");
    return 0;
}

DWORD WINAPI reader(PVOID path)
{
    string folderName((const char*)path);
    folderName.append("\\");

    string fileName = folderName + "?.txt";
    char currentFileName[MAX_PATH];

    BOOL(*readFunction)(OperationInfo*) = (BOOL(*)(OperationInfo*))GetProcAddress(library, "readFromFile");

    WIN32_FIND_DATA findData;
    HANDLE hFile, hFileToRead;

    hFile = FindFirstFile(fileName.c_str(), &findData);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("Path error");
        return 0;
    }

    while (1)
    {
        WaitForSingleObject(events[WRITE], INFINITE);

        strcpy(currentFileName, folderName.c_str());
        strcat(currentFileName, findData.cFileName);
        hFileToRead = CreateFile(currentFileName, GENERIC_READ, 0, NULL,
            OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

        operationInfo.hFile = hFileToRead;

        printf("\nReading from %s\n", currentFileName);

        (readFunction)(&operationInfo);
        SetEvent(events[READ]);

        if (FindNextFile(hFile, &findData))
        {
            CloseHandle(hFileToRead);
            continue;
        }
        else
        {
            break;
        }
    }

    FindClose(hFile);
    CloseHandle(hFileToRead);
    SetEvent(events[EXIT]);
    return 0;
}

DWORD WINAPI writer(PVOID path)
{
    string fileName((const char*)path);
    fileName.append("\\Result.txt");

    HANDLE hFile = CreateFile(fileName.c_str(), GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_FLAG_OVERLAPPED, NULL);

    HANDLE eventsForWait[2] = { events[READ], events[EXIT] };

    BOOL(*writeFunction)(OperationInfo*) = (BOOL(*)(OperationInfo*))GetProcAddress(library, "writeToFile");

    while (1)
    {
        int event = WaitForMultipleObjects(2, eventsForWait, FALSE, INFINITE);

        if (event == EXIT)
        {
            break;
        }

        operationInfo.hFile = hFile;
        printf("Writing to %s\n", fileName.c_str());
        (writeFunction)(&operationInfo);
        SetEvent(events[WRITE]);
    }

    CloseHandle(hFile);
    return 0;
}

#elif __linux__

struct OperationInfo
{
    char readFileName[500];
    char writeFileName[500];
    char buffer[300];
    struct aiocb readCb;
    struct aiocb writeCb;
    int bytesRead;
    int bytesWritten;
} operationInfo;

char fileNames[20][100];
int numberOfFiles = 0;
pthread_t hReader;
pthread_t hWriter;
pthread_mutex_t hMutex;

void(*writeFunction)(struct OperationInfo *operationInfo);
void(*readFunction)(struct OperationInfo *operationInfo);

void *reader(void *arg)
{
    for (int i = 0; i < numberOfFiles; i++)
    {
        pthread_mutex_lock(&hMutex);

        strcpy(operationInfo.readFileName, fileNames[i]);
        (*readFunction)(&operationInfo);

        pthread_mutex_unlock(&hMutex);
        usleep(100000);
    }
    return NULL;
}

void *writer(void *arg)
{
    strcpy(operationInfo.writeFileName, "./Result.txt");

    for (int i = 0; i < numberOfFiles; i++)
    {
        usleep(10000);
        pthread_mutex_lock(&hMutex);
        (*writeFunction)(&operationInfo);
        pthread_mutex_unlock(&hMutex);
        usleep(100000);
    }
    return NULL;
}

int main()
{
    if (pthread_mutex_init(&hMutex, NULL))
    {
        printf("Can't create mutex");
        return 0;
    }

    remove("./Result.txt");

    void *library;
    library = dlopen("./Library.so", RTLD_NOW);
    readFunction = (void(*)(struct OperationInfo *operationInfo))dlsym(library, "readFromFile");
    writeFunction = (void(*)(struct OperationInfo *operationInfo))dlsym(library, "writeToFile");

    struct dirent *dp;
    DIR *dir;

    dir = opendir("./");
    puts("Files in directory: ");
    while ((dp = readdir(dir)) != NULL)
    {
        if (strstr(dp->d_name, ".txt") != NULL)
        {
            strcpy(fileNames[numberOfFiles], dp->d_name);
            numberOfFiles++;
            puts(dp->d_name);
        }
    }

    pthread_create(&hReader, NULL, reader, NULL);
    pthread_create(&hWriter, NULL, writer, NULL);

    pthread_join(hReader, NULL);
    pthread_join(hWriter, NULL);

    return 0;
}

#endif
