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
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

static const char* strings[] = { "1.First\n\r", "2.Second\n\r", "3.Third\n\r",
"4.Fourth\n\r", "5.Fifth\n\r", "6.Sixth\n\r", "7.Seventh\n\r", "8.Eighth\n\r",
"9.Ninth\n\r", "10.Tenth\n\r" };

using namespace std;

#ifdef _WIN32

DWORD WINAPI print(void*);
CRITICAL_SECTION sectionForExecute;

class MyThread
{
private:
    HANDLE threadHandle;
    CRITICAL_SECTION sectionForPrint;
    CRITICAL_SECTION sectionForClose;
public:
    MyThread(int);
    int threadIndex;
    void startPrint();
    void endPrint();
    bool canPrint();
    bool isWaitingForPrint();
    void closeThread();
    bool isClose();
};

MyThread::MyThread(int number)
{
    threadIndex = number;

    InitializeCriticalSection(&sectionForPrint);
    InitializeCriticalSection(&sectionForClose);

    if (number == 1)
    {
        InitializeCriticalSection(&sectionForExecute);
    }

    threadHandle = CreateThread(NULL, 100, print, (void*)this, 0, 0);
}

void MyThread::startPrint() {
    EnterCriticalSection(&sectionForPrint);
}

void MyThread::endPrint() {
    LeaveCriticalSection(&sectionForPrint);
    LeaveCriticalSection(&sectionForExecute);
}

bool MyThread::canPrint() {
    if (TryEnterCriticalSection(&sectionForExecute) == 0)
        return false;

    LeaveCriticalSection(&sectionForExecute);
    return true;
}

bool MyThread::isWaitingForPrint() {
    if (TryEnterCriticalSection(&sectionForPrint) == 0)
        return false;

    LeaveCriticalSection(&sectionForPrint);
    return true;
}

void MyThread::closeThread() {
    EnterCriticalSection(&sectionForClose);
}

bool MyThread::isClose() {
    if (TryEnterCriticalSection(&sectionForClose) == 0)
        return true;

    LeaveCriticalSection(&sectionForClose);
    return false;
}

DWORD WINAPI print(void* threadName)
{
    MyThread *thread = (MyThread*)threadName;

    while (true)
    {
        if (!(thread->isWaitingForPrint()))
        {
            EnterCriticalSection(&sectionForExecute);

            for (int i = 0; i < strlen(strings[thread->threadIndex - 1]); i++)
            {
                cout << strings[thread->threadIndex - 1][i];
                Sleep(50);
            }

            thread->endPrint();
        }
        else
            Sleep(1);

        if (thread->isClose()) {
            break;
        }
    }
    return 0;
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

void* print(void*);
pthread_mutex_t* executeMutex = new pthread_mutex_t();

class MyThread
{
private:
    pthread_t* thread = new pthread_t();
    pthread_mutex_t* printMutex = new pthread_mutex_t();
    pthread_mutex_t* closeMutex = new pthread_mutex_t();
public:
    MyThread(int);
    int threadIndex;
    void startPrint();
    void endPrint();
    bool canPrint();
    bool isWaitingForPrint();
    void closeThread();
    bool isClose();
};

MyThread::MyThread(int number)
{
    threadIndex = number;

    pthread_mutex_init(printMutex, 0);
    pthread_mutex_lock(printMutex);
    pthread_mutex_init(closeMutex, 0);

    if (number == 1)
    {
        pthread_mutex_init(executeMutex, NULL);
    }

    pthread_create(thread, NULL, print, (void*)this);
}

void MyThread::startPrint() {
    pthread_mutex_unlock(printMutex);
}

void MyThread::endPrint() {
    pthread_mutex_lock(printMutex);
    pthread_mutex_unlock(executeMutex);
}

bool MyThread::canPrint() {
    if (pthread_mutex_trylock(executeMutex) != 0)
        return false;

    pthread_mutex_unlock(executeMutex);
    return true;
}

bool MyThread::isWaitingForPrint() {
    if (pthread_mutex_trylock(printMutex) != 0)
        return true;

    pthread_mutex_unlock(printMutex);
    return false;
}

void MyThread::closeThread() {
    pthread_mutex_lock(closeMutex);
}

bool MyThread::isClose() {
    if (pthread_mutex_trylock(closeMutex) != 0)
        return true;

    pthread_mutex_unlock(closeMutex);
    return false;
}

void* print(void* threadName)
{
    MyThread *thread = (MyThread*)threadName;

    while (true)
    {
        if (!(thread->isWaitingForPrint()))
        {
            pthread_mutex_lock(executeMutex);

            for (int i = 0; i < strlen(strings[thread->threadIndex - 1]); i++)
            {
                cout << strings[thread->threadIndex - 1][i];
                usleep(50000);
                fflush(stdout);
                refresh();
            }

            thread->endPrint();
        }
        else
        {
            usleep(1000);
            fflush(stdout);
            refresh();
        }

        if (thread->isClose()) {
            break;
        }
    }
    return 0;
}
#endif
