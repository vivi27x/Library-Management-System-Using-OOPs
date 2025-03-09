#include "lms.h"
using namespace std;
// User class implementation
User::User() : id(0) {}
User::User(int id, const string& name, const string& email, const string& password, const string& role)
    : id(id), name(name), email(email), password(password), role(role) {}

// Getters
int User::getId() const { return id; }
string User::getName() const { return name; }
string User::getEmail() const { return email; }
string User::getPassword() const { return password; }
string User::getRole() const { return role; }

// Setters
void User::setId(int id) { this->id = id; }
void User::setName(const string& name) { this->name = name; }
void User::setEmail(const string& email) { this->email = email; }
void User::setPassword(const string& password) { this->password = password; }
void User::setRole(const string& role) { this->role = role; }

// Display user details
void User::displayDetails() const {
    cout << "ID: " << id << endl;
    cout << "Name: " << name << endl;
    cout << "Email: " << email << endl;
    cout << "Role: " << role << endl;
    cout << endl;
}

// File I/O
void User::saveToFile(ofstream& outFile) const {
    outFile << role << endl;
    outFile << id << endl;
    outFile << name << endl;
    outFile << email << endl;
    outFile << password << endl;
}

User* User::loadFromFile(ifstream& inFile) {
    string role;
    getline(inFile, role);
    
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

Student::Student(int id, const string& name, const string& email, const string& password)
    : User(id, name, email, password, "Student") {}

bool Student::borrowBook(Book& book, time_t currentDate) {
    if (book.getStatus() != "Available") {
        cout << "Book is not available for borrowing." << endl;
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
        cout << "This book was not borrowed by you." << endl;
        return false;
    }   
    // Calculate overdue days (1 minute = 1 day)
    time_t dueDate = book.getDueDate();
    int overdueDays = 0;
    if (currentDate > dueDate) {
        overdueDays = (currentDate - dueDate) / 60; // Convert seconds to minutes
    }
    // Update book status
    book.setStatus("Available");
    book.setBorrowerId(0);
    book.setBorrowDate(0);
    book.setDueDate(0);
    // Return fine amount if overdue
    if (overdueDays > 0) {
        cout << "You have " << overdueDays << " overdue days. Please pay the fine." << endl;
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

void Student::saveToFile(ofstream& outFile) const {
    User::saveToFile(outFile);
}

Student* Student::loadFromFile(ifstream& inFile) {
    int id;
    string name, email, password;
    inFile >> id;
    inFile.ignore(); // Ignore newline after id
    getline(inFile, name);
    getline(inFile, email);
    getline(inFile, password);
    return new Student(id, name, email, password);
}

// Faculty class implementation
Faculty::Faculty() : User(0, "", "", "", "Faculty") {}

Faculty::Faculty(int id, const string& name, const string& email, const string& password)
    : User(id, name, email, password, "Faculty") {}

bool Faculty::borrowBook(Book& book, time_t currentDate) {
    if (book.getStatus() != "Available") {
        cout << "Book is not available for borrowing." << endl;
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
        cout << "This book was not borrowed by you." << endl;
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
void Faculty::saveToFile(ofstream& outFile) const {
    User::saveToFile(outFile);
}

Faculty* Faculty::loadFromFile(ifstream& inFile) {
    int id;
    string name, email, password;
    
    inFile >> id;
    inFile.ignore(); // Ignore newline after id
    getline(inFile, name);
    getline(inFile, email);
    getline(inFile, password);
    
    return new Faculty(id, name, email, password);
}

// Librarian class implementation
Librarian::Librarian() : User(0, "", "", "", "Librarian") {}

Librarian::Librarian(int id, const string& name, const string& email, const string& password)
    : User(id, name, email, password, "Librarian") {}

bool Librarian::borrowBook(Book& book, time_t currentDate) {
    cout << "Librarians cannot borrow books." << endl;
    return false;
}
bool Librarian::returnBook(Book& book, time_t currentDate) {
    cout << "Librarians cannot return books." << endl;
    return false;
}
void Librarian::saveToFile(ofstream& outFile) const {
    User::saveToFile(outFile);
}
Librarian* Librarian::loadFromFile(ifstream& inFile) {
    int id;
    string name, email, password;
    
    inFile >> id;
    inFile.ignore(); // Ignore newline after id
    getline(inFile, name);
    getline(inFile, email);
    getline(inFile, password);
    
    return new Librarian(id, name, email, password);
}
