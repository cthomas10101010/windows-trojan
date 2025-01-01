#ifndef FTP_UPLOADER_H
#define FTP_UPLOADER_H

#include <string>
#include <vector>

// Upload files matching given patterns (or all files if patterns is empty)
// to an FTP server.
void uploadFTP(const std::string& username,
               const std::string& password,
               const std::string& server,
               const std::string& localPath,
               const std::string& remoteDir,
               const std::vector<std::string>& includePatterns);

#endif // FTP_UPLOADER_H
