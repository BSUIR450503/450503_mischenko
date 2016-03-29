#include "CreateProcess.h"

int main(int argc, char *argv[])
{
#ifdef _WIN32
    if (argc == 3)
    {
        HANDLE printEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, argv[1]);
        HANDLE closeEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, argv[2]);

        while (1)
        {
            if (WaitForSingleObject(printEvent, 1) == WAIT_OBJECT_0)
            {
                print(atoi(argv[1]));
                ResetEvent(printEvent);
            }
            if (WaitForSingleObject(closeEvent, 0) == WAIT_OBJECT_0)
            {
                CloseHandle(closeEvent);
                CloseHandle(printEvent);
                return 0;
            }
        }
    }
    else
    {
        char key;
        int processCounter = 0;
        int currentPrinting = 1;
        MyProcess process[10];

        while (1)
        {
            key = getch_noblock();
            Sleep(1);
            switch (key)
            {
            case '+':
                if (processCounter < 10)
                {
                    processCounter++;
                    process[processCounter - 1].createNewProcess(processCounter, argv[0]);
                }
                break;
            case '-':
                if (processCounter > 0)
                {
                    SetEvent(process[--processCounter].getCloseEvent());
                }
                break;
            case 'q':
                if (processCounter > 0)
                {
                    for (int i = 0; i < processCounter; i++)
                        SetEvent(process[i].getCloseEvent());
                }
                return 0;
            }

            if (processCounter && WaitForSingleObject(process[currentPrinting - 1].getPrintEvent(), 0) == WAIT_TIMEOUT)
            {
                if (currentPrinting >= processCounter)
                    currentPrinting = 1;
                else
                    currentPrinting++;
                SetEvent(process[currentPrinting - 1].getPrintEvent());
            }
        }
    }

#elif __linux__
    pid_t process[10];
    char key;
    int processCounter = 0;
    int currentPrinting = 1;

    initscr();
    clear();
    noecho();
    refresh();

    pringSignal.sa_handler = setPrint;
    sigaction(SIGUSR1, &pringSignal, NULL);

    endSignal.sa_handler = setEnd;
    sigaction(SIGUSR2, &endSignal, NULL);

    while (1)
    {
        key = getchar();
        usleep(1000);
        fflush(stdout);
        refresh();

        switch (key)
        {
        case '+':
            if (processCounter < 10)
            {
                process[processCounter] = fork();
                processCounter++;

                switch (process[processCounter - 1])
                {
                case 0:
                {
                    endFlag = 0;
                    while (!endFlag)
                    {
                        usleep(1000);
                        fflush(stdout);
                        refresh();
                        if (printFlag)
                        {
                            print(processCounter);
                            refresh();
                            printFlag = 0;
                            kill(getppid(), SIGUSR2);
                        }
                    }
                    return 0;
                    break;
                }
                default:
                    break;
                }
            }
            break;

        case '-':
            if (processCounter > 0)
            {
                kill(process[--processCounter], SIGUSR2);
                waitpid(process[processCounter], NULL, 0);
                kill(process[processCounter], SIGKILL);
                waitpid(process[processCounter], NULL, 0);
            }
            break;

        case 'q':
            if (processCounter > 0)
            {
                for (; processCounter > 0; processCounter--)
                {
                    kill(process[processCounter - 1], SIGUSR2);
                    waitpid(process[processCounter - 1], NULL, 0);
                    kill(process[processCounter - 1], SIGKILL);
                    waitpid(process[processCounter - 1], NULL, 0);
                }
            }
            clear();
            endwin();
            return 0;
        }

        if (processCounter && endFlag)
        {
            endFlag = 0;
            if (currentPrinting >= processCounter)
                currentPrinting = 1;
            else
                currentPrinting++;
            kill(process[currentPrinting - 1], SIGUSR1);
        }
        refresh();
    }
#endif
}