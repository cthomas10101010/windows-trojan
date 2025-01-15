#ifndef GATHERER_H
#define GATHERER_H

#include <string>
#include <vector>

class Gatherer {
public:
    static void gatherFilesByExtension(const std::string& directoryPath, const std::vector<std::string>& extensions);
    static void gatherTextFiles(const std::string& directoryPath);
    static void gatherDocumentFiles(const std::string& directoryPath);
    static void gatherPdfFiles(const std::string& directoryPath);
    static void gatherAllFileTypes(const std::string& directoryPath);
    static bool isExcluded(const std::string& path);
};

#endif // GATHERER_H