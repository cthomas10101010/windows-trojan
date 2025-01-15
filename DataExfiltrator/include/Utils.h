#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

// Helper function: split string by delimiter
std::vector<std::string> splitString(const std::string& str, char delimiter);

// Get a list of files from a local path matching patterns. If patterns is empty,
// return all files in the directory.
std::vector<std::string> listFiles(const std::string& localPath,
                                   const std::vector<std::string>& patterns);

// Extract just the filename (with extension) from a full path.
std::string getFilenameFromPath(const std::string& fullPath);

// Simple pattern match (e.g. "*.txt", "*.doc?").
bool wildcardMatch(const std::string& pattern, const std::string& text);

#endif // UTILS_H
