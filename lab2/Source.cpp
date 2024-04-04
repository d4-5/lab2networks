#include <iostream>
#include <windows.h>
#include <fstream>
#include <string>

using namespace std;

HANDLE semaphore;
CRITICAL_SECTION cs;
HANDLE eventHandle;

DWORD WINAPI WriteNumbersNoSync(LPVOID lpParam) {
    int threadType = *(int*)lpParam;
    WaitForSingleObject(semaphore, INFINITE);
    ofstream outFile("numbers.txt", ios::app); 

    if (threadType == 1) {
        for (int i = 1; i <= 500; i++) {
            cout << to_string(i) + "\n";
            outFile << to_string(i) + "\n";
        }
    }
    else {
        for (int i = -1; i >= -500; i--) {
            cout << to_string(i) + "\n";
            outFile << to_string(i) + "\n";
        }
    }

    outFile.close();
    ReleaseSemaphore(semaphore, 1, NULL);
    return 0;
}

DWORD WINAPI WriteNumbersEventSync(LPVOID lpParam) {
    int threadType = *(int*)lpParam;
    WaitForSingleObject(semaphore, INFINITE);
    WaitForSingleObject(eventHandle, INFINITE);
    ResetEvent(eventHandle);
    
    ofstream outFile("numbers.txt", ios::app);

    if (threadType == 1) {
        for (int i = 1; i <= 500; i++) {
            cout << to_string(i) + "\n";
            outFile << to_string(i) + "\n";
        }
    }
    else {
        for (int i = -1; i >= -500; i--) {
            cout << to_string(i) + "\n";
            outFile << to_string(i) + "\n";
        }
    }

    outFile.close();
    SetEvent(eventHandle);
    ReleaseSemaphore(semaphore, 1, NULL);
    return 0;
}

DWORD WINAPI WriteNumbersCriticalSectionSync(LPVOID lpParam) {
    int threadType = *(int*)lpParam;
    WaitForSingleObject(semaphore, INFINITE);
    EnterCriticalSection(&cs);
    ofstream outFile("numbers.txt", ios::app);

    if (threadType == 1) {
        for (int i = 1; i <= 500; i++) {
            cout << to_string(i) + "\n";
            outFile << to_string(i) + "\n";
        }
    }
    else {
        for (int i = -1; i >= -500; i--) {
            cout << to_string(i) + "\n";
            outFile << to_string(i) + "\n";
        }
    }

    outFile.close();
    LeaveCriticalSection(&cs);
    ReleaseSemaphore(semaphore, 1, NULL);
    return 0;
}

int main() {
    InitializeCriticalSection(&cs);
    eventHandle = CreateEvent(NULL, TRUE, TRUE, NULL);
    semaphore = CreateSemaphore(NULL, 2, 2, NULL);

    HANDLE hThreads[6];
    int threadTypes[6] = { 1, 0, 1, 0, 1, 0 };
    int priorities[6] = { THREAD_PRIORITY_LOWEST, THREAD_PRIORITY_BELOW_NORMAL, THREAD_PRIORITY_NORMAL,
                          THREAD_PRIORITY_ABOVE_NORMAL, THREAD_PRIORITY_HIGHEST, THREAD_PRIORITY_IDLE };

    hThreads[0] = CreateThread(NULL, 0, WriteNumbersNoSync, &threadTypes[0], CREATE_SUSPENDED, NULL);
    hThreads[1] = CreateThread(NULL, 0, WriteNumbersNoSync, &threadTypes[1], CREATE_SUSPENDED, NULL);

    hThreads[2] = CreateThread(NULL, 0, WriteNumbersEventSync, &threadTypes[2], CREATE_SUSPENDED, NULL);
    hThreads[3] = CreateThread(NULL, 0, WriteNumbersEventSync, &threadTypes[3], CREATE_SUSPENDED, NULL);

    hThreads[4] = CreateThread(NULL, 0, WriteNumbersCriticalSectionSync, &threadTypes[4], CREATE_SUSPENDED, NULL);
    hThreads[5] = CreateThread(NULL, 0, WriteNumbersCriticalSectionSync, &threadTypes[5], CREATE_SUSPENDED, NULL);

    for (int i = 0; i < 6; ++i) {
        ResumeThread(hThreads[i]);
    }

    WaitForMultipleObjects(6, hThreads, TRUE, INFINITE);

    for (int i = 0; i < 6; ++i) {
        CloseHandle(hThreads[i]);
    }

    CloseHandle(eventHandle);
    DeleteCriticalSection(&cs);
    CloseHandle(semaphore);

    return 0;
}
