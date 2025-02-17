// self_destruct.cpp
#include "self_destruct.h"
#include <windows.h>
#include <iostream>

const wchar_t* regPath = L"Software\\MyApp";
const wchar_t* regValueName = L"RebootCount";

bool incrementRebootCounter(int threshold) {
    std::cout << "[DEBUG] incrementRebootCounter called." << std::endl;

    HKEY keyHandle;
    DWORD rebootCount = 0;
    DWORD dataSize = sizeof(rebootCount);

    // Open or create the registry key
    if (RegCreateKeyExW(HKEY_CURRENT_USER, regPath, 0, NULL, 0, KEY_READ | KEY_WRITE, NULL, &keyHandle, NULL) != ERROR_SUCCESS) {
        std::cerr << "[ERROR] Failed to access or create registry key.\n";
        return false;
    }

    // Read the current reboot count
    if (RegQueryValueExW(keyHandle, regValueName, NULL, NULL, (LPBYTE)&rebootCount, &dataSize) == ERROR_SUCCESS) {
        std::cout << "[DEBUG] Current Reboot Count (Before Increment): " << rebootCount << std::endl;
        rebootCount++;
    } else {
        rebootCount = 1; // Initialize if not found
        std::cout << "[DEBUG] Reboot Count Initialized to: " << rebootCount << std::endl;
    }

    // Update the reboot count
    if (RegSetValueExW(keyHandle, regValueName, 0, REG_DWORD, (const BYTE*)&rebootCount, sizeof(rebootCount)) != ERROR_SUCCESS) {
        std::cerr << "[ERROR] Failed to update registry value.\n";
    } else {
        std::cout << "[DEBUG] Reboot Count Updated to: " << rebootCount << std::endl;
    }

    RegCloseKey(keyHandle);

    return rebootCount >= threshold;
}
// Schedule file deletion on next reboot
bool scheduleFileDeletion(const std::wstring& filePath) {
    if (MoveFileExW(filePath.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT)) {
        std::wcout << L"[INFO] File scheduled for deletion on reboot: " << filePath << std::endl;
        return true;
    } else {
        std::wcerr << L"[ERROR] Failed to schedule file for deletion. Error: " << GetLastError() << std::endl;
        return false;
    }
}

// Self-destruct function
void selfDestruct(const std::wstring& filePath, int threshold) {
    if (incrementRebootCounter(threshold)) {
        std::wcout << L"[INFO] Reboot threshold reached. Initiating self-destruction..." << std::endl;

        // Schedule the executable for deletion
        if (scheduleFileDeletion(filePath)) {
            // Optionally clean up other artifacts
            RegDeleteKeyW(HKEY_CURRENT_USER, regPath); // Delete the registry key
            std::wcout << L"[INFO] Registry entries cleaned up." << std::endl;
        }
    } else {
        std::wcout << L"[INFO] Reboot threshold not yet reached. Reboot counter incremented." << std::endl;
    }
}
