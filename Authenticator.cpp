#include "Authenticator.h"
#include <iostream>
#include <unordered_map>

// Preselected email-password pairs for verification
std::unordered_map<std::string, std::string> validUsers = {
    {"user1@example.com", "secure123"},
    {"admin@example.com", "secure123"},
    {"employee@company.com", "secure123"}
};

bool verifyUserIdentity(const std::string& email) {
    auto it = validUsers.find(email);
    if (it != validUsers.end()) {
        std::cout << "\nVerification Successful.\n";
        std::cout << "Your verification password: \033[1;34m" << it->second << "\033[0m\n";
        return true;
    }
    return false;
}