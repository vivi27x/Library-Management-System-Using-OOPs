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
using namespace std;

Book::Book() : year(0), borrowerId(0), borrowDate(0), dueDate(0) {
    status = "Available";
}
Book::Book(const string& title, const string& author, const string& publisher, int year, const string& ISBN)
    : title(title), author(author), publisher(publisher), year(year), ISBN(ISBN), borrowerId(0), borrowDate(0), dueDate(0) {
    status = "Available";
}

// Getters
string Book::getTitle() const { return title; }
string Book::getAuthor() const { return author; }
string Book::getPublisher() const { return publisher; }
int Book::getYear() const { return year; }
string Book::getISBN() const { return ISBN; }
string Book::getStatus() const { return status; }
int Book::getBorrowerId() const { return borrowerId; }
time_t Book::getBorrowDate() const { return borrowDate; }
time_t Book::getDueDate() const { return dueDate; }

// Setters
void Book::setTitle(const string& title) { this->title = title; }
void Book::setAuthor(const string& author) { this->author = author; }
void Book::setPublisher(const string& publisher) { this->publisher = publisher; }
void Book::setYear(int year) { this->year = year; }
void Book::setISBN(const string& ISBN) { this->ISBN = ISBN; }
void Book::setStatus(const string& status) { this->status = status; }
void Book::setBorrowerId(int id) { this->borrowerId = id; }
void Book::setBorrowDate(time_t date) { this->borrowDate = date; }
void Book::setDueDate(time_t date) { this->dueDate = date; }

// Display book details
void Book::displayDetails() const {
    cout << "ISBN: " << ISBN << endl;
    cout << "Title: " << title << endl;
    cout << "Author: " << author << endl;
    cout << "Publisher: " << publisher << endl;
    cout << "Year: " << year << endl;
    cout << "Status: " << status << endl;
    
    if (status == "Borrowed") {
        // Convert time_t to readable format using chrono
        auto borrowDateChrono = chrono::system_clock::from_time_t(borrowDate);
        auto dueDateChrono = chrono::system_clock::from_time_t(dueDate);
        time_t borrowDateC = chrono::system_clock::to_time_t(borrowDateChrono);
        time_t dueDateC = chrono::system_clock::to_time_t(dueDateChrono);
        
        cout << "Borrowed by: " << borrowerId << endl;
        cout << "Borrow date: " << put_time(localtime(&borrowDateC), "%c %Z") << endl;
        cout << "Due date: " << put_time(localtime(&dueDateC), "%c %Z") << endl;
    }
    cout << endl;
}

// File I/O
void Book::saveToFile(ofstream& outFile) const {
    outFile << title << endl;
    outFile << author << endl;
    outFile << publisher << endl;
    outFile << year << endl;
    outFile << ISBN << endl;
    outFile << status << endl;
    outFile << borrowerId << endl;
    outFile << borrowDate << endl;
    outFile << dueDate << endl;
}

Book Book::loadFromFile(ifstream& inFile) {
    Book book;
    string line;
    
    getline(inFile, book.title);
    getline(inFile, book.author);
    getline(inFile, book.publisher);
    inFile >> book.year;
    inFile.ignore(); // Ignore newline after year
    getline(inFile, book.ISBN);
    getline(inFile, book.status);
    inFile >> book.borrowerId;
    inFile >> book.borrowDate;
    inFile >> book.dueDate;
    inFile.ignore(); // Ignore newline after dueDate
    
    return book;
}
