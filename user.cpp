#include "lms.h"

// User class implementation

User::User() : id(0) {}

User::User(int id, const std::string& name, const std::string& email, const std::string& password, const std::string& role)
    : id(id), name(name), email(email), password(password), role(role) {}

// Getters
int User::getId() const { return id; }
std::string User::getName() const { return name; }
std::string User::getEmail() const { return email; }
std::string User::getPassword() const { return password; }
std::string User::getRole() const { return role; }

// Setters
void User::setId(int id) { this->id = id; }
void User::setName(const std::string& name) { this->name = name; }
void User::setEmail(const std::string& email) { this->email = email; }
void User::setPassword(const std::string& password) { this->password = password; }
void User::setRole(const std::string& role) { this->role = role; }

// Display user details
void User::displayDetails() const {
    std::cout << "ID: " << id << std::endl;
    std::cout << "Name: " << name << std::endl;
    std::cout << "Email: " << email << std::endl;
    std::cout << "Role: " << role << std::endl;
    std::cout << std::endl;
}

// File I/O
void User::saveToFile(std::ofstream& outFile) const {
    outFile << role << std::endl;
    outFile << id << std::endl;
    outFile << name << std::endl;
    outFile << email << std::endl;
    outFile << password << std::endl;
}

User* User::loadFromFile(std::ifstream& inFile) {
    std::string role;
    std::getline(inFile, role);
    
    if (role == "Student") {
        return Student::loadFromFile(inFile);
    } else if (role == "Faculty") {
        return Faculty::loadFromFile(inFile);
    } else if (role == "Librarian") {
        return Librarian::loadFromFile(inFile);
    }
    
    return nullptr;
}

// Student class implementation

Student::Student() : User(0, "", "", "", "Student") {}

Student::Student(int id, const std::string& name, const std::string& email, const std::string& password)
    : User(id, name, email, password, "Student") {}

bool Student::borrowBook(Book& book, time_t currentDate) {
    if (book.getStatus() != "Available") {
        std::cout << "Book is not available for borrowing." << std::endl;
        return false;
    }
    
    book.setStatus("Borrowed");
    book.setBorrowerId(getId());
    book.setBorrowDate(currentDate);
    book.setDueDate(currentDate + getBorrowPeriod());
    
    return true;
}

bool Student::returnBook(Book& book, time_t currentDate) {
    if (book.getStatus() != "Borrowed" || book.getBorrowerId() != getId()) {
        std::cout << "This book was not borrowed by you." << std::endl;
        return false;
    }
    
    // Calculate overdue days
    time_t dueDate = book.getDueDate();
    int overdueDays = 0;
    
    if (currentDate > dueDate) {
        overdueDays = (currentDate - dueDate) / (24 * 60 * 60); // Convert seconds to days
    }
    
    // Update book status
    book.setStatus("Available");
    book.setBorrowerId(0);
    book.setBorrowDate(0);
    book.setDueDate(0);
    
    // Return fine amount if overdue
    if (overdueDays > 0) {
        return true;
    }
    
    return false;
}

int Student::getBorrowPeriod() {
    return BORROW_PERIOD;
}

int Student::getFineRate() {
    return FINE_RATE;
}

int Student::getMaxBooks() {
    return MAX_BOOKS;
}

void Student::saveToFile(std::ofstream& outFile) const {
    User::saveToFile(outFile);
}

Student* Student::loadFromFile(std::ifstream& inFile) {
    int id;
    std::string name, email, password;
    
    inFile >> id;
    inFile.ignore(); // Ignore newline after id
    std::getline(inFile, name);
    std::getline(inFile, email);
    std::getline(inFile, password);
    
    return new Student(id, name, email, password);
}

// Faculty class implementation

Faculty::Faculty() : User(0, "", "", "", "Faculty") {}

Faculty::Faculty(int id, const std::string& name, const std::string& email, const std::string& password)
    : User(id, name, email, password, "Faculty") {}

bool Faculty::borrowBook(Book& book, time_t currentDate) {
    if (book.getStatus() != "Available") {
        std::cout << "Book is not available for borrowing." << std::endl;
        return false;
    }
    
    book.setStatus("Borrowed");
    book.setBorrowerId(getId());
    book.setBorrowDate(currentDate);
    book.setDueDate(currentDate + getBorrowPeriod());
    
    return true;
}

bool Faculty::returnBook(Book& book, time_t currentDate) {
    if (book.getStatus() != "Borrowed" || book.getBorrowerId() != getId()) {
        std::cout << "This book was not borrowed by you." << std::endl;
        return false;
    }
    
    // Update book status
    book.setStatus("Available");
    book.setBorrowerId(0);
    book.setBorrowDate(0);
    book.setDueDate(0);
    
    // Faculty members don't pay fines
    return false;
}

int Faculty::getBorrowPeriod() {
    return BORROW_PERIOD;
}

int Faculty::getMaxBooks() {
    return MAX_BOOKS;
}

int Faculty::getMaxOverdueDays() {
    return MAX_OVERDUE_DAYS;
}

void Faculty::saveToFile(std::ofstream& outFile) const {
    User::saveToFile(outFile);
}

Faculty* Faculty::loadFromFile(std::ifstream& inFile) {
    int id;
    std::string name, email, password;
    
    inFile >> id;
    inFile.ignore(); // Ignore newline after id
    std::getline(inFile, name);
    std::getline(inFile, email);
    std::getline(inFile, password);
    
    return new Faculty(id, name, email, password);
}

// Librarian class implementation

Librarian::Librarian() : User(0, "", "", "", "Librarian") {}

Librarian::Librarian(int id, const std::string& name, const std::string& email, const std::string& password)
    : User(id, name, email, password, "Librarian") {}

bool Librarian::borrowBook(Book& book, time_t currentDate) {
    std::cout << "Librarians cannot borrow books." << std::endl;
    return false;
}

bool Librarian::returnBook(Book& book, time_t currentDate) {
    std::cout << "Librarians cannot return books." << std::endl;
    return false;
}

void Librarian::saveToFile(std::ofstream& outFile) const {
    User::saveToFile(outFile);
}

Librarian* Librarian::loadFromFile(std::ifstream& inFile) {
    int id;
    std::string name, email, password;
    
    inFile >> id;
    inFile.ignore(); // Ignore newline after id
    std::getline(inFile, name);
    std::getline(inFile, email);
    std::getline(inFile, password);
    
    return new Librarian(id, name, email, password);
}
