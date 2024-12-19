#include "AnalysisEvasion.h"
#include <windows.h>
#include <iostream>
#include <psapi.h>

void AntiAnalysis::IntroduceDelay(DWORD milliseconds) {
    std::cout << "[INFO] IntroduceDelay: Delaying execution for " << milliseconds << " milliseconds." << std::endl;
    Sleep(milliseconds);
    std::cout << "[INFO] IntroduceDelay: Execution resumed after delay." << std::endl;
}

BOOL AntiAnalysis::CheckSystemUptime(DWORD minUptimeSeconds) {
    std::cout << "[INFO] CheckSystemUptime: Checking system uptime." << std::endl;

    ULONGLONG uptimeMillis = GetTickCount64();
    DWORD uptimeSeconds = static_cast<DWORD>(uptimeMillis / 1000);
    std::cout << "[INFO] CheckSystemUptime: Current system uptime (seconds): " << uptimeSeconds << std::endl;
    
    if (uptimeSeconds >= minUptimeSeconds) {
        std::cout << "[INFO] CheckSystemUptime: System uptime meets the minimum threshold." << std::endl;
        return TRUE;
    } else {
        std::cout << "[INFO] CheckSystemUptime: System uptime is below the minimum threshold." << std::endl;
        return FALSE;
    }
}

bool AntiAnalysis::CheckForDebugger() {
    // Check if a debugger is present using IsDebuggerPresent
    if (IsDebuggerPresent()) {
        std::cerr << "Debugger detected!" << std::endl;
        return true;
    }

    // Check for debugging by examining the PEB (Process Environment Block)
    BOOL isDebugged = FALSE;
    CheckRemoteDebuggerPresent(GetCurrentProcess(), &isDebugged);
    if (isDebugged) {
        std::cerr << "Remote debugger detected!" << std::endl;
        return true;
    }

    return false;
}

bool AntiAnalysis::DetectVirtualMachine() {
    // Check for common virtual machine indicators
    HKEY hKey;
    const char* vmwareRegKey = "HARDWARE\\DESCRIPTION\\System\\BIOS";
    const char* vmwareValue = "VMware";

    // Open registry and check for VMware keys
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, vmwareRegKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        char buffer[256];
        DWORD bufferSize = sizeof(buffer);
        if (RegQueryValueExA(hKey, "SystemManufacturer", NULL, NULL, (LPBYTE)buffer, &bufferSize) == ERROR_SUCCESS) {
            if (strstr(buffer, vmwareValue)) {
                std::cerr << "VMware virtual environment detected!" << std::endl;
                RegCloseKey(hKey);
                return true;
            }
        }
        RegCloseKey(hKey);
    }

    return false;
}

bool AntiAnalysis::EvadeSandbox() {
    // Detect short runtime as an indicator of a sandbox environment
    SYSTEMTIME time;
    GetSystemTime(&time);
    if (time.wYear < 2020) { // Example check for inaccurate date
        std::cerr << "[ERROR] Sandbox environment detected based on system time!" << std::endl;
        return false; // Indicate sandbox detected
    }

    // Check for limited memory, another common sandbox characteristic
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    if (statex.ullTotalPhys / (1024 * 1024) < 2048) { // Less than 2GB RAM
        std::cerr << "[ERROR] Sandbox environment detected based on memory size!" << std::endl;
        return false; // Indicate sandbox detected
    }

    // Add more checks if necessary, such as process count, screen resolution, etc.

    // If all checks pass, return true to indicate no sandbox detected
    return true;
}

bool AntiAnalysis::checkScreenResolution() {
    // Get the screen resolution
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    std::cout << "[*] Detected screen resolution: " << screenWidth << "x" << screenHeight << std::endl;

    if (screenWidth < 800 || screenHeight < 600) {
        std::cout << "[!] Screen resolution too low. Potential sandbox detected." << std::endl;
        return false;  // Fail the check only for extremely low resolutions
    }

    return true;  // Pass the check otherwise
}

bool AntiAnalysis::checkProcessCount() {
    DWORD processes[1024], needed, processCount;
    if (!EnumProcesses(processes, sizeof(processes), &needed)) {
        std::cerr << "[!] Failed to enumerate processes." << std::endl;
        return false;
    }

    processCount = needed / sizeof(DWORD);
    std::cout << "[*] Detected " << processCount << " running processes." << std::endl;

    if (processCount < 100) {
        std::cerr << "[!] Too few processes. Potential sandbox detected." << std::endl;
        return false;
    }
    return true;
}