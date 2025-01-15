#include "HttpUploader.h"
#include <windows.h>
#include <wininet.h>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Utils.h"

// #include "HttpUploader.h"
// #include <windows.h>
// #include <wininet.h>
// #include <stdexcept>
// #include <iostream>
// #include <fstream>
// #include <sstream>
// #include "Utils.h"

#pragma comment(lib, "wininet.lib")

/*
    Simple example of uploading each file's contents as a POST request to:
      http://<server><remoteEndpoint>
    e.g. if server = "192.168.1.100" and remoteEndpoint = "/upload"
      -> "http://192.168.1.100/upload"

    We send each file with a simple "multipart/form-data" or a basic payload.
    This is a simplified approach; in production code, you'd want to handle
    boundaries, chunked uploads, MIME types, etc. thoroughly.
*/
#include "HttpUploader.h"
#include <windows.h>
#include <wininet.h>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include "Utils.h"

// Function to perform HTTP upload using multipart/form-data
void uploadHTTP(const std::string& username,
               const std::string& password,
               const std::string& server,
               const std::string& localPath,
               const std::string& remoteEndpoint,
               const std::vector<std::string>& includePatterns)
{
    // 1) Get a list of local files
    std::vector<std::string> files = listFiles(localPath, includePatterns);
    if (files.empty()) {
        std::cout << "[!] No files found in '" << localPath << "' that match pattern(s).\n";
        return;
    }

    // 2) Initialize WinINet for HTTP
    HINTERNET hInternet = InternetOpenA(
        "ChatGPT-HTTP-Client",
        INTERNET_OPEN_TYPE_DIRECT,
        NULL,
        NULL,
        0
    );
    if (!hInternet) {
        throw std::runtime_error("Failed to open internet handle for HTTP (InternetOpen).");
    }

    // 3) Create an HTTP connection handle
    HINTERNET hConnect = InternetConnectA(
        hInternet,
        server.c_str(),
        INTERNET_DEFAULT_HTTP_PORT,
        username.c_str(),
        password.c_str(),
        INTERNET_SERVICE_HTTP,
        0,
        0
    );
    if (!hConnect) {
        InternetCloseHandle(hInternet);
        throw std::runtime_error("Failed to connect to HTTP server (InternetConnect).");
    }

    // 4) Iterate over each file and upload
    for (const auto& filePath : files) {
        std::string filename = getFilenameFromPath(filePath);
        std::string boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";

        // Construct the full path of the resource:
        // e.g., "/upload"
        HINTERNET hRequest = HttpOpenRequestA(
            hConnect,
            "POST",
            remoteEndpoint.c_str(),  // Object name
            NULL,                    // Version
            NULL,                    // Referrer
            NULL,                    // Accept types
            INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE,
            0
        );

        if (!hRequest) {
            std::cerr << "[-] Failed to open HTTP request for: " << filePath << "\n";
            continue; // skip this file
        }

        // Read the file data into memory
        std::ifstream ifs(filePath, std::ios::binary);
        if (!ifs.good()) {
            std::cerr << "[-] Cannot open file: " << filePath << "\n";
            InternetCloseHandle(hRequest);
            continue;
        }
        std::vector<char> fileData((std::istreambuf_iterator<char>(ifs)),
                                   std::istreambuf_iterator<char>());

        // Construct the multipart/form-data body
        std::ostringstream oss;
        oss << "--" << boundary << "\r\n";
        oss << "Content-Disposition: form-data; name=\"file\"; filename=\"" << filename << "\"\r\n";
        oss << "Content-Type: application/octet-stream\r\n\r\n";
        std::string header = oss.str();

        // Ending boundary
        std::string ending = "\r\n--" + boundary + "--\r\n";

        // Total body size
        size_t totalSize = header.size() + fileData.size() + ending.size();

        // Allocate memory for the entire body
        std::vector<char> body;
        body.reserve(totalSize);
        body.insert(body.end(), header.begin(), header.end());
        body.insert(body.end(), fileData.begin(), fileData.end());
        body.insert(body.end(), ending.begin(), ending.end());

        // Set headers
        std::string headers = "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n";

        // Send the request with headers and body
        BOOL result = HttpSendRequestA(
            hRequest,
            headers.c_str(),
            static_cast<DWORD>(headers.size()),
            (LPVOID)body.data(),
            static_cast<DWORD>(body.size())
        );

        if (!result) {
            std::cerr << "[-] Failed to send HTTP request for file: " << filePath 
                      << ". Error code: " << GetLastError() << "\n";
        } else {
            std::cout << "[+] Uploaded via HTTP: " << filePath 
                      << " -> http://" << server << remoteEndpoint << "\n";
        }

        InternetCloseHandle(hRequest);
    }

    // Cleanup
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
}

// void uploadHTTP(const std::string& username,
//                 const std::string& password,
//                 const std::string& server,
//                 const std::string& localPath,
//                 const std::string& remoteEndpoint,
//                 const std::vector<std::string>& includePatterns)
// {
//     // 1) Get a list of local files
//     std::vector<std::string> files = listFiles(localPath, includePatterns);
//     if (files.empty()) {
//         std::cout << "[!] No files found in '" << localPath << "' that match pattern(s).\n";
//         return;
//     }

//     // 2) Initialize WinINet for HTTP
//     HINTERNET hInternet = InternetOpenA(
//         "ChatGPT-HTTP-Client",
//         INTERNET_OPEN_TYPE_DIRECT,
//         NULL,
//         NULL,
//         0
//     );
//     if (!hInternet) {
//         throw std::runtime_error("Failed to open internet handle for HTTP (InternetOpen).");
//     }

//     // 3) Create an HTTP connection handle
//     //    By default, we assume port 80 unless user included :port in 'server'.
//     //    If user typed "server:8080", parse that out (exercise for the reader).
//     HINTERNET hConnect = InternetConnectA(
//         hInternet,
//         server.c_str(),
//         INTERNET_DEFAULT_HTTP_PORT,
//         username.c_str(),
//         password.c_str(),
//         INTERNET_SERVICE_HTTP,
//         0,
//         0
//     );
//     if (!hConnect) {
//         InternetCloseHandle(hInternet);
//         throw std::runtime_error("Failed to connect to HTTP server (InternetConnect).");
//     }

//     // We'll use a simple loop to upload each file using a POST request.
//     for (const auto& filePath : files) {
//         std::string filename = getFilenameFromPath(filePath);

//         // Construct the full path of the resource:
//         // e.g. "/upload" or "/myendpoint"
//         // We'll use HttpOpenRequest for POST.
//         HINTERNET hRequest = HttpOpenRequestA(
//             hConnect,
//             "POST",
//             remoteEndpoint.c_str(),  // Object name
//             NULL,                    // Version
//             NULL,                    // Referrer
//             NULL,                    // Accept types
//             INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE,
//             0
//         );

//         if (!hRequest) {
//             std::cerr << "[-] Failed to open HTTP request for: " << filePath << "\n";
//             continue; // skip this file
//         }

//         // Read the file data into memory
//         std::ifstream ifs(filePath, std::ios::binary);
//         if (!ifs.good()) {
//             std::cerr << "[-] Cannot open file: " << filePath << "\n";
//             InternetCloseHandle(hRequest);
//             continue;
//         }
//         std::vector<char> fileData((std::istreambuf_iterator<char>(ifs)),
//                                    std::istreambuf_iterator<char>());

//         // Create a simple payload. Here we'll do a naive "multipart/form-data" or raw binary post.
//         // For demonstration, let's do raw binary with a custom header:
//         std::string headers = "Content-Type: application/octet-stream\r\n";
        
//         // Send the request
//         BOOL result = HttpSendRequestA(
//             hRequest,
//             headers.c_str(),
//             static_cast<DWORD>(headers.size()),
//             (LPVOID)fileData.data(),
//             static_cast<DWORD>(fileData.size())
//         );

//         if (!result) {
//             std::cerr << "[-] Failed to send HTTP request for file: " << filePath 
//                       << ". Error code: " << GetLastError() << "\n";
//         } else {
//             std::cout << "[+] Uploaded via HTTP: " << filePath 
//                       << " -> http://" << server << remoteEndpoint << "\n";
//         }

//         InternetCloseHandle(hRequest);
//     }

//     // Cleanup
//     InternetCloseHandle(hConnect);
//     InternetCloseHandle(hInternet);
// }
