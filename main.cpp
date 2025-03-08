#include "lms.h"
#include <iostream>
#include <string>


int main() {
    Library library;

    int userId;
    std::string password;
    
    std::cout << "Enter user ID: ";
    std::cin >> userId;
    std::cout << "Enter password: ";
    std::cin >> password;
    
    if (!library.login(userId, password)) {
        std::cout << "Login failed. Exiting...\n";
        return 1;
    }

    library.run(); // Start interactive menu

    return 0;
}
