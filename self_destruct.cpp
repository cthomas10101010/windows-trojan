#include "self_destruct.h"
#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

std::wstring getLastErrorAsString() {
    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0)
        return std::wstring(); // No error message has been recorded

    LPWSTR messageBuffer = nullptr;
    size_t size = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL);

    std::wstring message(messageBuffer, size);
    LocalFree(messageBuffer);
    return message;
}

bool secureDelete(const std::wstring& filePath) {
    // Overwrite the file with random data
    HANDLE hFile = CreateFileW(filePath.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::wcerr << L"[ERROR] Failed to open file for writing. Error: " << GetLastError() << " - " << getLastErrorAsString() << std::endl;
        return false;
    }

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) {
        std::wcerr << L"[ERROR] Failed to get file size. Error: " << GetLastError() << " - " << getLastErrorAsString() << std::endl;
        CloseHandle(hFile);
        return false;
    }

    std::vector<char> randomData(fileSize.QuadPart, 0);
    for (auto& byte : randomData) {
        byte = rand() % 256; // Fill with random data
    }

    DWORD bytesWritten;
    if (!WriteFile(hFile, randomData.data(), static_cast<DWORD>(randomData.size()), &bytesWritten, NULL)) {
        std::wcerr << L"[ERROR] Failed to write random data to file. Error: " << GetLastError() << " - " << getLastErrorAsString() << std::endl;
        CloseHandle(hFile);
        return false;
    }

    CloseHandle(hFile);
    std::wcout << L"[INFO] File overwritten successfully." << std::endl;

    // Schedule deletion on reboot
    if (!MoveFileExW(filePath.c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT)) {
        std::wcerr << L"[ERROR] Failed to schedule file for deletion. Error: " << GetLastError() << " - " << getLastErrorAsString() << std::endl;
        return false;
    }

    return true;
}

bool secureDeleteWithFallback(const std::wstring& filePath) {
    // Attempt direct deletion
    if (DeleteFileW(filePath.c_str())) {
        std::wcout << L"[INFO] File deleted directly: " << filePath << std::endl;
        return true;
    }

    std::wcerr << L"[ERROR] Direct file deletion failed. Error: " << GetLastError() << " - " << getLastErrorAsString() << std::endl;

    // Schedule deletion via helper process if direct deletion fails
    std::wstring cmd = L"cmd.exe /c del \"" + filePath + L"\"";
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    if (CreateProcessW(NULL, const_cast<wchar_t*>(cmd.c_str()), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        std::wcout << L"[INFO] File deletion scheduled using command prompt." << std::endl;
        return true;
    }

    std::wcerr << L"[ERROR] Fallback deletion failed. Error: " << GetLastError() << " - " << getLastErrorAsString() << std::endl;
    return false;
}

void selfDestruct(const std::wstring& filePath) {
    std::wcout << L"[INFO] Initiating self-destruction process..." << std::endl;

    if (secureDelete(filePath) || secureDeleteWithFallback(filePath)) {
        std::wcout << L"[INFO] Self-destruction completed successfully." << std::endl;
    } else {
        std::wcerr << L"[ERROR] Self-destruction failed." << std::endl;
    }

    HANDLE hProcess = GetCurrentProcess();
    TerminateProcess(hProcess, 0);
}


const wchar_t* regPath = L"Software\\MyApp"; // Example registry path
const wchar_t* regValueName = L"RebootCount2323aqwIee"; // Example registry value name

bool incrementRebootCounter(int threshold) {
    HKEY keyHandle;
    DWORD rebootCount = 0;
    DWORD dataSize = sizeof(rebootCount);

    // Open or create the registry key
    if (RegCreateKeyExW(HKEY_CURRENT_USER, regPath, 0, NULL, 0, KEY_READ | KEY_WRITE, NULL, &keyHandle, NULL) != ERROR_SUCCESS) {
        std::cerr << "[ERROR] Failed to access or create registry key." << std::endl;
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
        std::cerr << "[ERROR] Failed to update registry value." << std::endl;
    }

    RegCloseKey(keyHandle);

    std::cout << "[INFO] Current reboot count: " << rebootCount << std::endl;
    return rebootCount >= threshold;
}