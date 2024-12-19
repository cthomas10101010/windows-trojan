#include "Utilities.h"
#include <iostream>

void displayHeader() {
    std::cout << "\n\033[1;36m=============================================\033[0m\n";
    std::cout << "        \033[1;36mSENSITIVE DOCUMENT ACCESS PORTAL\033[0m         \n";
    std::cout << "\033[1;36m=============================================\033[0m\n";
}

void displayVerificationPrompt() {
    std::cout << "\n\033[1;33mYou are accessing sensitive documents.\033[0m\n";
    std::cout << "Please verify your identity to proceed.\n";
    std::cout << "\033[1;36m---------------------------------------------\033[0m\n";
}

std::string getUserEmail() {
    std::string email;
    std::cout << "\n\033[1;34mEnter your email: \033[0m";
    std::getline(std::cin, email);
    return email;
}