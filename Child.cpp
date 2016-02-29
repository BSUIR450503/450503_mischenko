#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#include <iostream>
#endif

#ifdef linux
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#endif

using namespace std;

int main(int argc, char* argv[])
{
#ifdef _WIN32

    HANDLE namedPipe = CreateFile(TEXT("\\\\.\\pipe\\Pipe"),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);

    if (namedPipe == INVALID_HANDLE_VALUE)
    {
        cout << "Cant create pipe. Error " << GetLastError() << endl;
    }

    DWORD numRead;
    DWORD numToWrite;
    double number;

    ReadFile(namedPipe, &number, sizeof(double), &numRead, NULL);

    number *= number;

    WriteFile(namedPipe, &number, sizeof(double), &numToWrite, NULL);

#endif

#ifdef __linux__

int fileDescriptor[2];
double number;

fileDescriptor[0] = atoi(argv[0]);
fileDescriptor[1] = atoi(argv[1]);

read(fileDescriptor[0], &number, sizeof(double));
close(fileDescriptor[0]);

number *= number;

write(fileDescriptor[1], &number, sizeof(double));
close(fileDescriptor[1]);

#endif
}
