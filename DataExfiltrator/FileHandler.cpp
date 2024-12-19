#include "FileHandler.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <cstdlib>

namespace fs = std::filesystem;

namespace FileHandler {
    std::string CreateZipArchive(const std::string& directoryPath, const std::string& zipPath) {
        std::string command = "powershell Compress-Archive -Path \"" + directoryPath + "\\*\" -DestinationPath \"" + zipPath + "\" -Force";
        int result = std::system(command.c_str());
        
        if (result == 0) {
            std::cout << "Archive created successfully: " << zipPath << std::endl;
            return zipPath;
        } else {
            std::cerr << "Failed to create archive." << std::endl;
            return "";
        }
    }
}
