#include "lms.h"
#include <chrono>
#include <iomanip>
/*
Account
Create an Account class to track user activity. Each user has one account. The account must:
• Maintain a record of currently borrowed books.
• Track overdue books and calculate fines.
• Store the borrowing history of the user.
• Keep a record of fines and whether they have been paid.

*/
// Account class implementation
using namespace std;

Account::Account() : userId(0), fines(0), hasPaidFines(true) {}

Account::Account(int userId) : userId(userId), fines(0), hasPaidFines(true) {}

// Getters
int Account::getUserId() const { return userId; }
vector<string> Account::getBorrowedBooks() const { return borrowedBooks; }
vector<string> Account::getBorrowHistory() const { return borrowHistory; }
double Account::getFines() const { return fines; }
bool Account::getHasPaidFines() const { return hasPaidFines; }

// Setters
void Account::setUserId(int userId) { this->userId = userId; }
void Account::setBorrowedBooks(const vector<string>& books) { this->borrowedBooks = books; }
void Account::setBorrowHistory(const vector<string>& history) { this->borrowHistory = history; }
void Account::setFines(double fines) { this->fines = fines; }
void Account::setHasPaidFines(bool paid) { this->hasPaidFines = paid; }

// Account operations
void Account::addBorrowedBook(const string& ISBN) {
    borrowedBooks.push_back(ISBN);
}
void Account::removeBorrowedBook(const string& ISBN) {
    for (auto it = borrowedBooks.begin(); it != borrowedBooks.end(); ++it) {
        if (*it == ISBN) {
            borrowedBooks.erase(it);
            break;
        }
    }
}
void Account::addToBorrowHistory(const string& ISBN) {
    // Add to history only if it's not already there
    if (find(borrowHistory.begin(), borrowHistory.end(), ISBN) == borrowHistory.end()) {
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
    cout << "User ID: " << userId << endl;
    cout << "Number of borrowed books: " << borrowedBooks.size() << endl;
    cout << "Total books in history: " << borrowHistory.size() << endl;
    cout << "Outstanding fines: Rs. " << fixed << setprecision(2) << fines << endl;
    cout << "Fines paid: " << (hasPaidFines ? "Yes" : "No") << endl;
    cout << endl;
}
void Account::displayBorrowedBooks(const map<string, Book>& books) const {
    if (borrowedBooks.empty()) {
        cout << "No books currently borrowed." << endl;
        return;
    }
    
    cout << "Currently borrowed books:" << endl;
    cout << "-----------------------" << endl;
    
    for (const auto& isbn : borrowedBooks) {
        auto it = books.find(isbn);
        if (it != books.end()) {
            cout << "ISBN: " << it->second.getISBN() << endl;
            cout << "Title: " << it->second.getTitle() << endl;
            
            // Convert time_t to readable format using chrono
            auto dueDate = chrono::system_clock::from_time_t(it->second.getDueDate());
            time_t dueDateC = chrono::system_clock::to_time_t(dueDate);
            cout << "Due date: " << put_time(localtime(&dueDateC), "%c %Z") << endl;
        }
    }
    cout << endl;
}
void Account::displayBorrowHistory(const map<string, Book>& books) const {
    if (borrowHistory.empty()) {
        cout << "No borrowing history." << endl;
        return;
    }
    
    cout << "Borrowing history:" << endl;
    cout << "-----------------" << endl;
    
    for (const auto& isbn : borrowHistory) {
        auto it = books.find(isbn);
        if (it != books.end()) {
            cout << "ISBN: " << it->second.getISBN() << endl;
            cout << "Title: " << it->second.getTitle() << endl;
            cout << endl;
        }
    }
}

// File I/O
void Account::saveToFile(ofstream& outFile) const {
    outFile << userId << endl;
    outFile << fines << endl;
    outFile << (hasPaidFines ? "1" : "0") << endl;
    
    // Save borrowed books
    outFile << borrowedBooks.size() << endl;
    for (const auto& isbn : borrowedBooks) {
        outFile << isbn << endl;
    }
    
    // Save borrow history
    outFile << borrowHistory.size() << endl;
    for (const auto& isbn : borrowHistory) {
        outFile << isbn << endl;
    }
}

Account Account::loadFromFile(ifstream& inFile) {
    Account account;
    int numBooks = 0;
    string line;
    account.userId = 0;
    account.fines = 0.0;
    int paid = 0;
    
    inFile >> account.userId;
    inFile >> account.fines;
    inFile >> paid;
    account.hasPaidFines = (paid == 1);
    
    // Load borrowed books
    inFile >> numBooks;
    inFile.ignore(); // Ignore newline after numBooks
    
    for (int i = 0; i < numBooks; ++i) {
        getline(inFile, line);
        account.borrowedBooks.push_back(line);
    }
    
    // Load borrow history
    inFile >> numBooks;
    inFile.ignore(); // Ignore newline after numBooks
    
    for (int i = 0; i < numBooks; ++i) {
        getline(inFile, line);
        account.borrowHistory.push_back(line);
    }
    
    return account;
}
