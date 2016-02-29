#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#include <iostream>
#endif

#ifdef linux
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#endif

using namespace std;

class MyProcess
{
public:
    MyProcess(int, char**);
};

MyProcess::MyProcess(int argc, char* argv[])
{
#ifdef _WIN32

    STARTUPINFO startupInfo;
    PROCESS_INFORMATION processInfo;
    TCHAR CommandLine[] = TEXT("ChildProcess");

    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);
    ZeroMemory(&processInfo, sizeof(processInfo));

    if (!CreateProcess(NULL,
        CommandLine,				
        NULL,						
        NULL,						
        FALSE,						
        0,							
        NULL,						
        NULL,						
        &startupInfo,
        &processInfo))	
    {
        cout << "Can't create process. Error " << GetLastError();
    }


    HANDLE namedPipe = CreateNamedPipe(TEXT("\\\\.\\pipe\\Pipe"),
        PIPE_ACCESS_DUPLEX | PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
        PIPE_WAIT,
        1,
        1024,
        1024,
        NMPWAIT_USE_DEFAULT_WAIT,
        NULL);

    if (namedPipe == INVALID_HANDLE_VALUE)
    {
        cout << "Can't create pipe. Error " << GetLastError();
    }

    DWORD numWritten;
    DWORD numToRead;

    ConnectNamedPipe(namedPipe, NULL);

    double number;
    cout << "Enter x number: ";
    cin >> number;

    WriteFile(namedPipe, &number, sizeof(double), &numWritten, NULL);

    WaitForSingleObject(processInfo.hProcess, INFINITE);

    ReadFile(namedPipe, &number, sizeof(double), &numToRead, NULL);

    cout << "x^2 = " << number << endl;

    Sleep(5000);

    CloseHandle(namedPipe);
    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);
#endif

#ifdef linux

    int fileDescriptor[2];
    double number;

    if (pipe(fileDescriptor) == -1)
    {
        cout << "Can't open pipe" << endl;
    }

    pid_t pid = fork();

    if (pid < 0)
    {
        cout << "Can't fork process" << endl;
    }

    else if (pid == 0)
    {
        char readDescriptor[4], writeDescriptor[4];
        sprintf(readDescriptor, "%d", fileDescriptor[0]);
        sprintf(writeDescriptor, "%d", fileDescriptor[1]);
        execl("ChildProcess", readDescriptor, writeDescriptor, NULL);
    }

    else if (pid > 0)
    {

        cout << "Enter x number: ";
        cin >> number;

        write(fileDescriptor[1], &number, sizeof(double));
        close(fileDescriptor[1]);
        wait(NULL);
        read(fileDescriptor[0], &number, sizeof(double));
        close(fileDescriptor[0]);
        cout << "x^2 = " << number << endl;

        sleep(5);
    }

#endif
}
