#ifndef FILEHANDLER_H
#define FILEHANDLER_H

#include <string>

namespace FileHandler {
    std::string CreateZipArchive(const std::string& directoryPath, const std::string& zipPath);
}

#endif // FILEHANDLER_H
