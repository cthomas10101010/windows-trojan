#ifndef GATHERER_H
#define GATHERER_H

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>

extern std::queue<std::string> fileQueue;
extern std::mutex exfilMutex;
extern std::condition_variable cv;
extern bool doneGathering;

void gatherFiles(const std::string& directoryPath, const std::vector<std::string>& extensions);
bool hasExtension(const std::string& filePath, const std::vector<std::string>& extensions);

#endif // GATHERER_H
