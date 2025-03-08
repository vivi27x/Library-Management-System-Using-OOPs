#include "lms.h"
#include <ctime>
#include <filesystem>
#include <algorithm>
#include <limits>
#include <chrono>
#include <iomanip>

// Library class implementation

Library::Library(const std::string& dataDir) : currentUserId(0), dataDirectory(dataDir) {
    // Create data directory if it doesn't exist
    std::filesystem::create_directories(dataDirectory);
    
    // Load data from files if they exist
    loadData();
    
    // If no data was loaded, add initial data
    if (books.empty()) {
        addInitialData();
    }
}

Library::~Library() {
    // Save data to files
    saveData();
    
    // Free memory allocated for User objects
    for (auto& pair : users) {
        delete pair.second;
    }
}

// Helper methods
// Helper method: Add initial data if files are not present
void Library::addInitialData() {
    // Add books
    addBook(Book("Introduction to Algorithms", "Thomas H. Cormen", "MIT Press", 2009, "9780262033848"));
    addBook(Book("Clean Code", "Robert C. Martin", "Prentice Hall", 2008, "9780132350884"));
    addBook(Book("Design Patterns", "Erich Gamma", "Addison-Wesley", 1994, "9780201633610"));
    addBook(Book("The Pragmatic Programmer", "Andrew Hunt", "Addison-Wesley", 1999, "9780201616224"));
    addBook(Book("Code Complete", "Steve McConnell", "Microsoft Press", 2004, "9780735619678"));
    addBook(Book("Refactoring", "Martin Fowler", "Addison-Wesley", 1999, "9780201485677"));
    addBook(Book("Head First Design Patterns", "Eric Freeman", "O'Reilly Media", 2004, "9780596007126"));
    addBook(Book("The C Programming Language", "Brian W. Kernighan", "Prentice Hall", 1988, "9780131103627"));
    addBook(Book("Effective C++", "Scott Meyers", "Addison-Wesley", 2005, "9780321334879"));
    addBook(Book("Programming Pearls", "Jon Bentley", "Addison-Wesley", 1999, "9780201657883"));
    
    // Add users (5 students, 3 faculty, 1 librarian)
    // Students
    addUser(new Student(1001, "John Smith", "john@example.com", "password1"));
    addUser(new Student(1002, "Emily Johnson", "emily@example.com", "password2"));
    addUser(new Student(1003, "Michael Brown", "michael@example.com", "password3"));
    addUser(new Student(1004, "Jessica Davis", "jessica@example.com", "password4"));
    addUser(new Student(1005, "Daniel Wilson", "daniel@example.com", "password5"));
    
    // Faculty
    addUser(new Faculty(2001, "Dr. Alan Turing", "turing@example.com", "password6"));
    addUser(new Faculty(2002, "Dr. Grace Hopper", "hopper@example.com", "password7"));
    addUser(new Faculty(2003, "Dr. Ada Lovelace", "ada@example.com", "password8"));
    
    // Librarian
    addUser(new Librarian(3001, "Laura Librarian", "laura@example.com", "password9"));
    
    // Create accounts for each user
    for (const auto& pair : users) {
        accounts[pair.first] = Account(pair.first);
    }
}

void Library::saveData() {
    std::ofstream booksFile(dataDirectory + "/books.txt");
    std::ofstream usersFile(dataDirectory + "/users.txt");
    std::ofstream accountsFile(dataDirectory + "/accounts.txt");

    if (!booksFile || !usersFile || !accountsFile) {
        std::cerr << "Error: Unable to open files for saving data." << std::endl;
        return;
    }

    // Save Books
    for (const auto& pair : books) {
        pair.second.saveToFile(booksFile);
    }

    // Save Users
    for (const auto& pair : users) {
        pair.second->saveToFile(usersFile);
    }

    // Save Accounts
    for (const auto& pair : accounts) {
        pair.second.saveToFile(accountsFile);
    }

    booksFile.close();
    usersFile.close();
    accountsFile.close();
}

void Library::loadData() {
    std::ifstream booksFile(dataDirectory + "/books.txt");
    std::ifstream usersFile(dataDirectory + "/users.txt");
    std::ifstream accountsFile(dataDirectory + "/accounts.txt");

    if (!booksFile) std::cerr << "Warning: books.txt not found. Starting with empty library." << std::endl;
    if (!usersFile) std::cerr << "Warning: users.txt not found. Starting with no users." << std::endl;
    if (!accountsFile) std::cerr << "Warning: accounts.txt not found. Starting with no accounts." << std::endl;

    // Load Books
    while (booksFile) {
        Book book = Book::loadFromFile(booksFile);
        if (booksFile) books[book.getISBN()] = book;
    }

    // Load Users
    while (usersFile) {
        User* user = User::loadFromFile(usersFile);
        if (user) users[user->getId()] = user;
    }

    // Load Accounts
    while (accountsFile) {
        Account account = Account::loadFromFile(accountsFile);
        if (accountsFile) accounts[account.getUserId()] = account;
    }

    booksFile.close();
    usersFile.close();
    accountsFile.close();
}   


// Utility: Get current time
time_t Library::getCurrentDate() const {
    return std::time(nullptr);
}

// Utility: Format time_t to readable string
std::string Library::formatDate(time_t date) const {
    auto dateChrono = std::chrono::system_clock::from_time_t(date);
    std::time_t dateC = std::chrono::system_clock::to_time_t(dateChrono);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&dateC), "%c %Z");
    return oss.str();
}

// Utility: Calculate number of overdue days
int Library::calculateOverdueDays(time_t dueDate, time_t currentDate) const {
    if (currentDate > dueDate)
        return (currentDate - dueDate) / (24 * 60 * 60);
    return 0;
}


// ----- Book Management -----

void Library::addBook(const Book& book) {
    if (!isLoggedIn() || getCurrentUser()->getRole() != "Librarian") {
        std::cout << "Access denied. Only librarians can add books.\n";
        return;
    }
    
    books[book.getISBN()] = book;
    std::cout << "Book added successfully.\n";
}

void Library::removeBook(const std::string& ISBN) {
    if (!isLoggedIn() || getCurrentUser()->getRole() != "Librarian") {
        std::cout << "Access denied. Only librarians can remove books.\n";
        return;
    }
    
    books.erase(ISBN);
    std::cout << "Book removed successfully.\n";
}
void Library::updateBook(const Book& book) {
    if (!isLoggedIn() || getCurrentUser()->getRole() != "Librarian") {
        std::cout << "Access denied. Only librarians can add books.\n";
        return;
    }
    books[book.getISBN()] = book;
    std::cout << "Book updated successfully.\n";
}

void Library::displayAllBooks() const {
    for (const auto& pair : books)
        pair.second.displayDetails();
}

void Library::searchBooks(const std::string& keyword) const {
    for (const auto& pair : books) {
        if (pair.second.getTitle().find(keyword) != std::string::npos ||
            pair.second.getAuthor().find(keyword) != std::string::npos ||
            pair.second.getISBN().find(keyword) != std::string::npos) {
            pair.second.displayDetails();
        }
    }
}


// ----- User Management -----

void Library::addUser(User* user) {
    if (!isLoggedIn() || getCurrentUser()->getRole() != "Librarian") {
        std::cout << "Access denied. Only librarians can add users.\n";
        return;
    }
    
    users[user->getId()] = user;
    accounts[user->getId()] = Account(user->getId());
    std::cout << "User added successfully.\n";
}


void Library::removeUser(int userId) {
    if (!isLoggedIn() || getCurrentUser()->getRole() != "Librarian") {
        std::cout << "Access denied. Only librarians can remove users.\n";
        return;
    }
    
    users.erase(userId);
    accounts.erase(userId);
    std::cout << "User removed successfully.\n";
}
void Library::displayAllUsers() const {
    if (!isLoggedIn() || getCurrentUser()->getRole() != "Librarian") {
        std::cout << "Access denied. Only librarians can remove users.\n";
        return;
    }
    for (const auto& pair : users)
        pair.second->displayDetails();
}

User* Library::findUser(int userId) const {  // Add 'const' here as well

    auto it = users.find(userId);
    return (it != users.end()) ? it->second : nullptr;
}

Book* Library::findBook(const std::string& ISBN) {
    auto it = books.find(ISBN);
    if (it != books.end())
        return &(it->second);
    return nullptr;
}

Account* Library::findAccount(int userId) {
    auto it = accounts.find(userId);
    if (it != accounts.end())
        return &(it->second);
    return nullptr;
}

// ----- Book Operations -----

bool Library::borrowBook(int userId, const std::string& ISBN) {
    User* user = findUser(userId);
    Book* book = findBook(ISBN);
    Account* account = findAccount(userId);
    if (!user || !book || !account) {
        std::cerr << "Invalid user or book." << std::endl;
        return false;
    }

    // Check if user has unpaid fines
    if (account->getFines() > 0) {
        std::cout << "Please clear your outstanding fines before borrowing new books." << std::endl;
        return false;
    }

    // Borrow the book (using polymorphism)
    time_t currentDate = getCurrentDate();
    if (user->borrowBook(*book, currentDate)) {
        account->addBorrowedBook(ISBN);
        account->addToBorrowHistory(ISBN);
        std::cout << "Book borrowed successfully." << std::endl;
        return true;
    }
    return false;
}

bool Library::returnBook(int userId, const std::string& ISBN) {
    User* user = findUser(userId);
    Book* book = findBook(ISBN);
    Account* account = findAccount(userId);
    if (!user || !book || !account) {
        std::cerr << "Invalid user or book." << std::endl;
        return false;
    }
    time_t currentDate = getCurrentDate();
    bool fineApplicable = user->returnBook(*book, currentDate);
    account->removeBorrowedBook(ISBN);
    if (fineApplicable) {
        // Calculate fine for overdue books (for students)
        int overdueDays = calculateOverdueDays(book->getDueDate(), currentDate);
        if (overdueDays > 0 && user->getRole() == "Student") {
            int fine = overdueDays * Student::getFineRate();
            account->addFine(fine);
            std::cout << "Book returned. Overdue by " << overdueDays 
                      << " days. Fine: Rs." << fine << std::endl;
        } else {
            std::cout << "Book returned successfully." << std::endl;
        }
    } else {
        std::cout << "Book returned successfully." << std::endl;
    }
    return true;
}

void Library::checkOverdueBooks() {
    time_t currentDate = getCurrentDate();
    for (auto& pair : books) {
        Book& book = pair.second;
        if (book.getStatus() == "Borrowed") {
            int overdueDays = calculateOverdueDays(book.getDueDate(), currentDate);
            if (overdueDays > 0) {
                std::cout << "Book \"" << book.getTitle() << "\" is overdue by " 
                          << overdueDays << " days." << std::endl;
            }
        }
    }
}

void Library::calculateFines() {
    time_t currentDate = getCurrentDate();
    for (auto& accPair : accounts) {
        Account& account = accPair.second;
        // Only students incur fines
        User* user = findUser(account.getUserId());
        if (user && user->getRole() == "Student") {
            // For each borrowed book, calculate overdue fine if any
            for (const std::string& isbn : account.getBorrowedBooks()) {
                Book* book = findBook(isbn);
                if (book) {
                    int overdueDays = calculateOverdueDays(book->getDueDate(), currentDate);
                    if (overdueDays > 0) {
                        int fine = overdueDays * Student::getFineRate();
                        account.addFine(fine);
                    }
                }
            }
        }
    }
}

// ----- Authentication -----
bool Library::login(int userId, const std::string& password) {
    User* user = findUser(userId);
    if (!user) {
        std::cout << "User ID not found.\n";
        return false;
    }
    
    if (user->getPassword() == password) {
        currentUserId = userId;
        std::cout << "Login successful. Welcome, " << user->getName() << "!\n";
        return true;
    } else {
        std::cout << "Incorrect password.\n";
        return false;
    }
}

void Library::logout() {
    currentUserId = 0;
    std::cout << "Logged out successfully.\n";
}

bool Library::isLoggedIn() const {
    return currentUserId != 0;
}

User* Library::getCurrentUser() const {
    return findUser(currentUserId);
}


// ----- Account Operations -----

void Library::displayUserAccount() const {
    if (!isLoggedIn()) {
        std::cout << "Please log in first.\n";
        return;
    }

    int userId = currentUserId;  // Use the logged-in user ID
    auto it = accounts.find(userId);
    
    if (it == accounts.end()) {
        std::cout << "No account found for this user.\n";
        return;
    }

    std::cout << "Account details for " << getCurrentUser()->getName() << ":\n";
    it->second.displayDetails();
}

void Library::payFines(int userId) {
    Account* account = findAccount(userId);
    if (account) {
        account->payFines();
        std::cout << "Fines paid successfully." << std::endl;
    } else {
        std::cout << "Account not found." << std::endl;
    }
}

// ----- Run the Library System (Simple CLI) -----

void Library::run() {
    int choice;
    while (true) {
        std::cout << "\nLibrary Management System\n";
        std::cout << "1. Display All Books\n";
        std::cout << "2. Search Books\n";
        std::cout << "3. Display All Users\n";
        std::cout << "4. Borrow Book\n";
        std::cout << "5. Return Book\n";
        std::cout << "6. Display User Account\n";
        std::cout << "7. Pay Fines\n";
        std::cout << "8. Check Overdue Books\n";
        std::cout << "9. Exit\n";
        std::cout << "Enter your choice: ";
        std::cin >> choice;
        std::cin.ignore(); // ignore newline

        if (choice == 9)
            break;

        int userId;
        std::string ISBN, keyword;
        switch (choice) {
            case 1:
                displayAllBooks();
                break;
            case 2:
                std::cout << "Enter keyword to search: ";
                std::getline(std::cin, keyword);
                searchBooks(keyword);
                break;
            case 3:
                displayAllUsers();
                break;
            case 4:
                std::cout << "Enter User ID: ";
                std::cin >> userId;
                std::cout << "Enter Book ISBN: ";
                std::cin >> ISBN;
                borrowBook(userId, ISBN);
                break;
            case 5:
                std::cout << "Enter User ID: ";
                std::cin >> userId;
                std::cout << "Enter Book ISBN: ";
                std::cin >> ISBN;
                returnBook(userId, ISBN);
                break;
            case 6:
                // std::cout << "Enter User ID: ";
                // std::cin >> userId;
                displayUserAccount();
                break;
            case 7:
                std::cout << "Enter User ID: ";
                std::cin >> userId;
                payFines(userId);
                break;
            case 8:
                checkOverdueBooks();
                break;
            default:
                std::cout << "Invalid choice. Try again." << std::endl;
                break;
        }
    }
}