#include "CreateThread.h"

int main(int argc, char *argv[])
{
    char key;
    int threadsCounter = 0;
    int currentPrinting = 1;
    MyThread* threads[10];

#ifdef _WIN32

    while (1)
    {
        key = getch_noblock();
        Sleep(1);
        switch (key)
        {
        case '+':
            if (threadsCounter < 10)
            {
                threadsCounter++;
                threads[threadsCounter - 1] = new MyThread(threadsCounter);
            }
            break;
        case '-':
            if (threadsCounter > 0)
            {
                threads[threadsCounter - 1]->closeThread();
                threadsCounter--;
            }
            break;
        case 'q':
            if (threadsCounter > 0)
            {
                for (int i = 0; i < threadsCounter; i++)
                    threads[i]->closeThread();
            }
            return 0;
        }

        if (threadsCounter && threads[currentPrinting - 1]->canPrint())
        {
            if (currentPrinting >= threadsCounter)
                currentPrinting = 1;
            else
                currentPrinting++;
            threads[currentPrinting - 1]->startPrint();
        }
    }

#elif __linux__

    initscr();
    clear();
    noecho();
    refresh();
    nodelay(stdscr, TRUE);

    while (1)
    {
        key = getch();
        usleep(1000);
        fflush(stdout);
        refresh();

        switch (key)
        {
        case '+':
            if (threadsCounter < 10)
            {
                threadsCounter++;
                threads[threadsCounter - 1] = new MyThread(threadsCounter);
            }
            break;

        case '-':
            if (threadsCounter > 0)
            {
                threads[threadsCounter - 1]->closeThread();
                threadsCounter--;
            }
            break;

        case 'q':
            if (threadsCounter > 0)
            {
                for (int i = 0; i < threadsCounter; i++)
                    threads[i]->closeThread();
            }
            clear();
            endwin();
            return 0;
        }

        if (threadsCounter && threads[currentPrinting - 1]->canPrint())
        {
            if (currentPrinting >= threadsCounter)
                currentPrinting = 1;
            else
                currentPrinting++;
            threads[currentPrinting - 1]->startPrint();
        }
        refresh();
    }
#endif
}