#ifdef _WIN32
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#endif

#ifdef linux
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

static const char* strings[] = { "1.First\n\r", "2.Second\n\r", "3.Third\n\r",
    "4.Fourth\n\r", "5.Fifth\n\r", "6.Sixth\n\r", "7.Seventh\n\r", "8.Eighth\n\r",
    "9.Ninth\n\r", "10.Tenth\n\r" };

using namespace std;

void print(int number);

#ifdef _WIN32
class MyProcess
{
private:
    PROCESS_INFORMATION processInfo;
    HANDLE printEvent;
    HANDLE closeEvent;
public:
    PROCESS_INFORMATION createNewProcess(int, char*);
    HANDLE getPrintEvent() {
        return printEvent;
    }
    HANDLE getCloseEvent() {
        return closeEvent;
    }
};

PROCESS_INFORMATION MyProcess::createNewProcess(int number, char* name)
{
    STARTUPINFO startupInfo;
    PROCESS_INFORMATION processInfo;

    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);
    ZeroMemory(&processInfo, sizeof(processInfo));

    char printEventName[10];
    char closeEventName[10];
    char commandLine[10] = "-n ";

    _itoa(number, printEventName, 10);
    _itoa(number+10, closeEventName, 10);

    strcat(commandLine, printEventName);
    strcat(commandLine, " ");
    strcat(commandLine, closeEventName);

    if (!CreateProcess(name,
        commandLine,
        NULL,
        NULL,
        FALSE,
        NULL,
        NULL,
        NULL,
        &startupInfo,
        &processInfo))
    {
        cout << "Can't create process. Error " << GetLastError();
    }

    printEvent = CreateEvent(NULL, TRUE, FALSE, printEventName);
    closeEvent = CreateEvent(NULL, FALSE, FALSE, closeEventName);

    return processInfo;
}

int getch_noblock()
{
    if (_kbhit())
        return _getch();
    else
        return -1;
}
#endif

#ifdef __linux__
int printFlag = 0;
int endFlag = 1;

struct sigaction pringSignal;
struct sigaction endSignal;

void setPrint(int sig) 
{
    printFlag = 1;
}

void setEnd(int sig) 
{
    endFlag = 1;
}
#endif

void print(int number)
{
    for (int i = 0; i < strlen(strings[number - 1]); i++)
    {
        cout << strings[number - 1][i];
#ifdef _WIN32
        Sleep(50);
#elif __linux__
        usleep(1000);
        fflush(stdout);
        refresh();
#endif
    }
    return;
}

