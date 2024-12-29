// self_destruct.cpp
#include "self_destruct.h"
#include <windows.h>
#include <iostream>

const wchar_t* regPath = L"Software\\MyApp";
const wchar_t* regValueName = L"RebootCount1";

// Increment reboot counter and check if threshold is reached
bool incrementRebootCounter(int threshold) {
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
        rebootCount++;
    } else {
        rebootCount = 1; // Initialize the counter if not present
    }

    // Update the reboot count
    if (RegSetValueExW(keyHandle, regValueName, 0, REG_DWORD, (const BYTE*)&rebootCount, sizeof(rebootCount)) != ERROR_SUCCESS) {
        std::cerr << "[ERROR] Failed to update registry value.\n";
    }

    RegCloseKey(keyHandle);

    // Check if the threshold is reached
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
