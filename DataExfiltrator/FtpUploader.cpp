#include "FtpUploader.h"
#include <windows.h>
#include <wininet.h>
#include <stdexcept>
#include <iostream>
#include "Utils.h"

#pragma comment(lib, "wininet.lib")

void uploadFTP(const std::string& username,
               const std::string& password,
               const std::string& server,
               const std::string& localPath,
               const std::string& remoteDir,
               const std::vector<std::string>& includePatterns)
{
    // 1) Get a list of local files to upload
    std::vector<std::string> files = listFiles(localPath, includePatterns);

    if (files.empty()) {
        std::cout << "[!] No files found in '" << localPath << "' that match the pattern(s).\n";
        return;
    }

    // 2) Initialize WinINet for FTP session
    HINTERNET hInternet = InternetOpenA(
        "ChatGPT-FTP-Client",
        INTERNET_OPEN_TYPE_DIRECT,  // direct connection
        NULL,
        NULL,
        0
    );

    if (!hInternet) {
        throw std::runtime_error("Failed to open Internet handle (InternetOpen).");
    }

    // We assume the server is on port 21 if not specified. If you want to handle :port, parse it out.
    HINTERNET hFtpSession = InternetConnectA(
        hInternet,
        server.c_str(),
        INTERNET_DEFAULT_FTP_PORT,
        username.c_str(),
        password.c_str(),
        INTERNET_SERVICE_FTP,
        INTERNET_FLAG_PASSIVE,
        0
    );

    if (!hFtpSession) {
        InternetCloseHandle(hInternet);
        throw std::runtime_error("Failed to connect to FTP server (InternetConnect).");
    }

    // 3) Ensure the remote directory exists (or just attempt to set it)
    if (!FtpSetCurrentDirectoryA(hFtpSession, remoteDir.c_str())) {
        // Attempt to create the directory
        if (!FtpCreateDirectoryA(hFtpSession, remoteDir.c_str())) {
            // If it fails for any reason (e.g., already exists), we just ignore it
        }
        // Attempt again
        FtpSetCurrentDirectoryA(hFtpSession, remoteDir.c_str());
    }

    // 4) Upload each file
    for (const auto& filePath : files) {
        // Extract filename from path
        std::string filename = getFilenameFromPath(filePath);

        // Actually do the FTP upload
        BOOL result = FtpPutFileA(
            hFtpSession,
            filePath.c_str(),    // local file
            filename.c_str(),    // remote name
            FTP_TRANSFER_TYPE_BINARY,
            0
        );

        if (!result) {
            std::cerr << "[-] Error uploading file: " << filePath << ". Error code: " << GetLastError() << "\n";
        } else {
            std::cout << "[+] Uploaded: " << filePath << " -> " << remoteDir << "/" << filename << "\n";
        }
    }

    // 5) Cleanup
    InternetCloseHandle(hFtpSession);
    InternetCloseHandle(hInternet);
}
