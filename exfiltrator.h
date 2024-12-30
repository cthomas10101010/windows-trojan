#ifndef EXFILTRATOR_H
#define EXFILTRATOR_H

#include <string>

// void exfiltrateFile(const std::string& targetURL);
void exfiltrateFile(const std::string& filePath, const std::string& targetURL);
void gatherFilesAndExfiltrate(const std::string& directoryPath, const std::string& targetURL);
void ManualDataExfil(const std::string& directoryPath, const std::string& targetURL);
#endif // EXFILTRATOR_H
