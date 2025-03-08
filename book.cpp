#include "lms.h"
#include <chrono>
#include <iomanip>

// Book class implementation
/*
Books
Define a Book class to represent books in the library.
Attributes:
• title, author, publisher, year, and ISBN.
• Status: Tracks whether the book is available, borrowed, or reserved.
Constraints:
• Start with at least 5 books in the system.
• Books can only be borrowed if their status is ”Available.”
*/

Book::Book() : year(0), borrowerId(0), borrowDate(0), dueDate(0) {
    status = "Available";
}

Book::Book(const std::string& title, const std::string& author, const std::string& publisher, int year, const std::string& ISBN)
    : title(title), author(author), publisher(publisher), year(year), ISBN(ISBN), borrowerId(0), borrowDate(0), dueDate(0) {
    status = "Available";
}

// Getters
std::string Book::getTitle() const { return title; }
std::string Book::getAuthor() const { return author; }
std::string Book::getPublisher() const { return publisher; }
int Book::getYear() const { return year; }
std::string Book::getISBN() const { return ISBN; }
std::string Book::getStatus() const { return status; }
int Book::getBorrowerId() const { return borrowerId; }
time_t Book::getBorrowDate() const { return borrowDate; }
time_t Book::getDueDate() const { return dueDate; }

// Setters
void Book::setTitle(const std::string& title) { this->title = title; }
void Book::setAuthor(const std::string& author) { this->author = author; }
void Book::setPublisher(const std::string& publisher) { this->publisher = publisher; }
void Book::setYear(int year) { this->year = year; }
void Book::setISBN(const std::string& ISBN) { this->ISBN = ISBN; }
void Book::setStatus(const std::string& status) { this->status = status; }
void Book::setBorrowerId(int id) { this->borrowerId = id; }
void Book::setBorrowDate(time_t date) { this->borrowDate = date; }
void Book::setDueDate(time_t date) { this->dueDate = date; }

// Display book details
void Book::displayDetails() const {
    std::cout << "ISBN: " << ISBN << std::endl;
    std::cout << "Title: " << title << std::endl;
    std::cout << "Author: " << author << std::endl;
    std::cout << "Publisher: " << publisher << std::endl;
    std::cout << "Year: " << year << std::endl;
    std::cout << "Status: " << status << std::endl;
    
    if (status == "Borrowed") {
        // Convert time_t to readable format using chrono
        auto borrowDateChrono = std::chrono::system_clock::from_time_t(borrowDate);
        auto dueDateChrono = std::chrono::system_clock::from_time_t(dueDate);
        std::time_t borrowDateC = std::chrono::system_clock::to_time_t(borrowDateChrono);
        std::time_t dueDateC = std::chrono::system_clock::to_time_t(dueDateChrono);
        
        std::cout << "Borrowed by: " << borrowerId << std::endl;
        std::cout << "Borrow date: " << std::put_time(std::localtime(&borrowDateC), "%c %Z") << std::endl;
        std::cout << "Due date: " << std::put_time(std::localtime(&dueDateC), "%c %Z") << std::endl;
    }
    std::cout << std::endl;
}

// File I/O
void Book::saveToFile(std::ofstream& outFile) const {
    outFile << title << std::endl;
    outFile << author << std::endl;
    outFile << publisher << std::endl;
    outFile << year << std::endl;
    outFile << ISBN << std::endl;
    outFile << status << std::endl;
    outFile << borrowerId << std::endl;
    outFile << borrowDate << std::endl;
    outFile << dueDate << std::endl;
}

Book Book::loadFromFile(std::ifstream& inFile) {
    Book book;
    std::string line;
    
    std::getline(inFile, book.title);
    std::getline(inFile, book.author);
    std::getline(inFile, book.publisher);
    inFile >> book.year;
    inFile.ignore(); // Ignore newline after year
    std::getline(inFile, book.ISBN);
    std::getline(inFile, book.status);
    inFile >> book.borrowerId;
    inFile >> book.borrowDate;
    inFile >> book.dueDate;
    inFile.ignore(); // Ignore newline after dueDate
    
    return book;
}
