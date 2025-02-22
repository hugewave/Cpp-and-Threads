#include <windows.h>
#include <iostream>
#include <thread>
using namespace std;

// Thread function to execute
DWORD WINAPI ThreadFunction(LPVOID lpParam) {
    std::cout << "Thread is running on dedicated CPU core." << std::endl;
    // Add your thread-specific code here
    return 0;
}

int main() {
    unsigned int numCores = std::thread::hardware_concurrency();
    if (numCores != 0) {
        std::cout << "Number of CPU cores: " << numCores << std::endl;
    } else {
        std::cout << "Unable to determine the number of CPU cores." << std::endl;
    }

    // Get the number of CPU cores on windows
    //SYSTEM_INFO sysInfo;
    //GetSystemInfo(&sysInfo);
    //std::cout << "Number of CPU cores: " << sysInfo.dwNumberOfProcessors << std::endl;

    // Create the thread
    HANDLE hThread = CreateThread(
        NULL,                   // Default security attributes
        0,                      // Default stack size
        ThreadFunction,         // Thread function
        NULL,                   // No parameters
        0,                      // Default creation flags
        NULL                    // No thread ID
    );

    // Check if thread creation succeeded
    if (hThread == NULL) {
        std::cerr << "Failed to create thread. Error: " << GetLastError() << std::endl;
        return 1;
    }

    // Set thread affinity to CPU core 0
    //00000001 - Core 0
    //00000010 - Core 1
    //00000100 - Core 2
    //00000011 - Core 1 and 0
    DWORD_PTR affinityMask = 1; // Core 0
    if (SetThreadAffinityMask(hThread, affinityMask) == 0) {
        std::cerr << "Failed to set thread affinity. Error: " << GetLastError() << std::endl;
        CloseHandle(hThread);
        return 1;
    }

    // Wait for the thread to finish
    WaitForSingleObject(hThread, INFINITE);

    // Clean up
    CloseHandle(hThread);

    return 0;
}