#ifndef SELF_DESTRUCT_H
#define SELF_DESTRUCT_H

#include <string>
#include <windows.h>

// Function to retrieve detailed error messages
std::wstring getLastErrorAsString();

// Function to securely delete a file by overwriting and scheduling deletion on reboot
bool secureDelete(const std::wstring& filePath);

// Function to delete a file directly or schedule it using a fallback method
bool secureDeleteWithFallback(const std::wstring& filePath);

// Function to initiate self-destruction of the executable
void selfDestruct(const std::wstring& filePath);

// Registry constants for tracking reboot counts
extern const wchar_t* regPath;
extern const wchar_t* regValueName;

// Function to increment the reboot counter and check if the threshold is reached
bool incrementRebootCounter(int threshold);

#endif // SELF_DESTRUCT_H
