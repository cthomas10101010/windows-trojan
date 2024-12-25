#ifndef AUTOSTART_CONFIG_H
#define AUTOSTART_CONFIG_H

#include <windows.h>
#include <tchar.h>
#include <string>
// Retrieves the dynamic application name from the executable path.
std::wstring getDynamicAppName();
bool configureAutoStart(const std::wstring& appName, const std::wstring& appPath);
//bool configureAutoStart(LPCTSTR appName, LPCTSTR appPath);
// bool checkAutoStartEnabled(LPCTSTR appName);
bool checkAutoStartEnabled(const std::wstring& appName);

int setupPersistence();
void setupAndRunScheduledTask();
bool copyToStartupFolder();
#endif 


