#include "lms.h"
#include <chrono>
#include <iomanip>

// Account class implementation

Account::Account() : userId(0), fines(0), hasPaidFines(true) {}

Account::Account(int userId) : userId(userId), fines(0), hasPaidFines(true) {}

// Getters
int Account::getUserId() const { return userId; }
std::vector<std::string> Account::getBorrowedBooks() const { return borrowedBooks; }
std::vector<std::string> Account::getBorrowHistory() const { return borrowHistory; }
double Account::getFines() const { return fines; }
bool Account::getHasPaidFines() const { return hasPaidFines; }

// Setters
void Account::setUserId(int userId) { this->userId = userId; }
void Account::setBorrowedBooks(const std::vector<std::string>& books) { this->borrowedBooks = books; }
void Account::setBorrowHistory(const std::vector<std::string>& history) { this->borrowHistory = history; }
void Account::setFines(double fines) { this->fines = fines; }
void Account::setHasPaidFines(bool paid) { this->hasPaidFines = paid; }

// Account operations
void Account::addBorrowedBook(const std::string& ISBN) {
    borrowedBooks.push_back(ISBN);
}

void Account::removeBorrowedBook(const std::string& ISBN) {
    for (auto it = borrowedBooks.begin(); it != borrowedBooks.end(); ++it) {
        if (*it == ISBN) {
            borrowedBooks.erase(it);
            break;
        }
    }
}

void Account::addToBorrowHistory(const std::string& ISBN) {
    // Add to history only if it's not already there
    if (std::find(borrowHistory.begin(), borrowHistory.end(), ISBN) == borrowHistory.end()) {
        borrowHistory.push_back(ISBN);
    }
}

void Account::addFine(double amount) {
    fines += amount;
    if (fines > 0) {
        hasPaidFines = false;
    }
}

void Account::payFines() {
    fines = 0;
    hasPaidFines = true;
}

// Display account details
void Account::displayDetails() const {
    std::cout << "User ID: " << userId << std::endl;
    std::cout << "Number of borrowed books: " << borrowedBooks.size() << std::endl;
    std::cout << "Total books in history: " << borrowHistory.size() << std::endl;
    std::cout << "Outstanding fines: Rs. " << std::fixed << std::setprecision(2) << fines << std::endl;
    std::cout << "Fines paid: " << (hasPaidFines ? "Yes" : "No") << std::endl;
    std::cout << std::endl;
}

void Account::displayBorrowedBooks(const std::map<std::string, Book>& books) const {
    if (borrowedBooks.empty()) {
        std::cout << "No books currently borrowed." << std::endl;
        return;
    }
    
    std::cout << "Currently borrowed books:" << std::endl;
    std::cout << "-----------------------" << std::endl;
    
    for (const auto& isbn : borrowedBooks) {
        auto it = books.find(isbn);
        if (it != books.end()) {
            std::cout << "ISBN: " << it->second.getISBN() << std::endl;
            std::cout << "Title: " << it->second.getTitle() << std::endl;
            
            // Convert time_t to readable format using chrono
            auto dueDate = std::chrono::system_clock::from_time_t(it->second.getDueDate());
            std::time_t dueDateC = std::chrono::system_clock::to_time_t(dueDate);
            std::cout << "Due date: " << std::put_time(std::localtime(&dueDateC), "%c %Z") << std::endl;
        }
    }
    std::cout << std::endl;
}

void Account::displayBorrowHistory(const std::map<std::string, Book>& books) const {
    if (borrowHistory.empty()) {
        std::cout << "No borrowing history." << std::endl;
        return;
    }
    
    std::cout << "Borrowing history:" << std::endl;
    std::cout << "-----------------" << std::endl;
    
    for (const auto& isbn : borrowHistory) {
        auto it = books.find(isbn);
        if (it != books.end()) {
            std::cout << "ISBN: " << it->second.getISBN() << std::endl;
            std::cout << "Title: " << it->second.getTitle() << std::endl;
            std::cout << std::endl;
        }
    }
}

// File I/O
void Account::saveToFile(std::ofstream& outFile) const {
    outFile << userId << std::endl;
    outFile << fines << std::endl;
    outFile << (hasPaidFines ? "1" : "0") << std::endl;
    
    // Save borrowed books
    outFile << borrowedBooks.size() << std::endl;
    for (const auto& isbn : borrowedBooks) {
        outFile << isbn << std::endl;
    }
    
    // Save borrow history
    outFile << borrowHistory.size() << std::endl;
    for (const auto& isbn : borrowHistory) {
        outFile << isbn << std::endl;
    }
}

Account Account::loadFromFile(std::ifstream& inFile) {
    Account account;
    int numBooks;
    std::string line;
    
    inFile >> account.userId;
    inFile >> account.fines;
    
    int paid;
    inFile >> paid;
    account.hasPaidFines = (paid == 1);
    
    // Load borrowed books
    inFile >> numBooks;
    inFile.ignore(); // Ignore newline after numBooks
    
    for (int i = 0; i < numBooks; ++i) {
        std::getline(inFile, line);
        account.borrowedBooks.push_back(line);
    }
    
    // Load borrow history
    inFile >> numBooks;
    inFile.ignore(); // Ignore newline after numBooks
    
    for (int i = 0; i < numBooks; ++i) {
        std::getline(inFile, line);
        account.borrowHistory.push_back(line);
    }
    
    return account;
}
