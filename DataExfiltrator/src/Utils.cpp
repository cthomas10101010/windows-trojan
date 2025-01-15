#include "Utils.h"
#include <windows.h>
#include <Shlwapi.h>  // For PathMatchSpecA (if you like) or implement your own
#include <iostream>
#include <algorithm>
#include <sstream>

#pragma comment(lib, "Shlwapi.lib")

std::vector<std::string> splitString(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

// We'll do a naive approach: if localPath is a directory, gather files via FindFirstFile/FindNextFile.
// If localPath is a single file, return just that. Then filter by patterns if provided.
std::vector<std::string> listFiles(const std::string& localPath,
                                   const std::vector<std::string>& patterns)
{
    std::vector<std::string> result;

    // Check if localPath is a directory or file
    // Attempt to find all files in localPath if it is a directory
    // Use a wildcard like localPath\\* to list all files
    // If user provided something like "C:\\path\\file.txt" it might match just that file.

    // Construct a search path
    std::string searchPath = localPath;
    // Ensure trailing slash/backslash for directories
    if (!searchPath.empty() &&
        searchPath.back() != '\\' &&
        searchPath.back() != '/' )
    {
        searchPath += "\\";
    }
    searchPath += "*";  // wildcard

    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        // Perhaps it's a single file or invalid path.
        // If it's an actual file, we add it directly:
        DWORD attrs = GetFileAttributesA(localPath.c_str());
        if ((attrs != INVALID_FILE_ATTRIBUTES) &&
            !(attrs & FILE_ATTRIBUTE_DIRECTORY))
        {
            // localPath is an actual file
            // We'll check pattern match later
            result.push_back(localPath);
        }
        return result;
    }

    do {
        // Skip directories
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            continue;
        }
        std::string fullPath = localPath;
        // ensure slash
        if (!fullPath.empty() &&
            fullPath.back() != '\\' &&
            fullPath.back() != '/' )
        {
            fullPath += "\\";
        }
        fullPath += findData.cFileName;
        result.push_back(fullPath);
    } while (FindNextFileA(hFind, &findData));
    FindClose(hFind);

    // If no patterns, return all
    if (patterns.empty()) {
        return result;
    }

    // Filter by patterns
    std::vector<std::string> filtered;
    for (auto& f : result) {
        std::string filename = getFilenameFromPath(f);
        for (auto& pat : patterns) {
            if (wildcardMatch(pat, filename)) {
                filtered.push_back(f);
                break; // no need to check other patterns
            }
        }
    }
    return filtered;
}

std::string getFilenameFromPath(const std::string& fullPath)
{
    // Find the last slash or backslash
    size_t pos1 = fullPath.find_last_of('\\');
    size_t pos2 = fullPath.find_last_of('/');
    size_t pos  = (pos1 == std::string::npos) ? pos2
                 : (pos2 == std::string::npos ? pos1 : std::max(pos1, pos2));

    if (pos == std::string::npos) {
        // No slashes found
        return fullPath; 
    }
    return fullPath.substr(pos + 1);
}

bool wildcardMatch(const std::string& pattern, const std::string& text)
{
    // A quick approach: use PathMatchSpecA from Shlwapi
    // Otherwise, you could implement your own naive wildcard matching.
    return PathMatchSpecA(text.c_str(), pattern.c_str()) == TRUE;
}
