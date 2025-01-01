#ifndef HTTP_UPLOADER_H
#define HTTP_UPLOADER_H

#include <string>
#include <vector>

// Upload files matching given patterns (or all files if patterns is empty)
// to an HTTP server on port 80, using WinINet or other native APIs.
void uploadHTTP(const std::string& username,
                const std::string& password,
                const std::string& server,
                const std::string& localPath,
                const std::string& remoteEndpoint,
                const std::vector<std::string>& includePatterns);

#endif // HTTP_UPLOADER_H
