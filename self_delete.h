#ifndef SELF_DELETE_H
#define SELF_DELETE_H

#include <windows.h>
#include <string>
#include <fstream>
#include <iostream>

// Declare the registry path and value names as extern
extern const wchar_t* REGISTRY_PATH;
extern const wchar_t* REBOOT_COUNT_KEY;

const int MAX_REBOOTS = 2;
void SearchAndDeleteDynamicExe(const std::string& basePath);
// Function to log the number of reboots
void LogRebootCount();

// Function to delete the program after 3 reboots
void DeleteAfterReboots();

#endif