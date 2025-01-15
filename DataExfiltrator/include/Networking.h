// Networking.h
#pragma once
#include <string>

class Networking {
public:
    /**
     * @brief Uploads a file to the specified target URL using the WinHTTP API.
     * 
     * @param filePath The absolute path to the file to be uploaded.
     * @param targetURL The target URL where the file will be uploaded.
     * @return true If the file was uploaded successfully.
     * @return false If the upload failed.
     */
    static bool uploadFile(const std::string& filePath, const std::string& targetURL);
};
