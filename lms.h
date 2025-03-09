#ifndef LMS_H
#define LMS_H

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <ctime>
#include <map>
#include <iomanip>
#include <sstream>
#include <algorithm>
/*
Classes:
• Create at least four classes: User, Book, Account, and Library.
• Use inheritance to derive Student, Faculty, and Librarian from the User class.
2. Encapsulation:
• Use private attributes for sensitive information like user credentials and account details.
• Provide public methods for controlled access to private attributes. Each attribute of the class
must have a member function to either update. Each attribute of a class must have a member
function, that prints or shows the value of the attribute. the attribute or reset the value.
3. Polymorphism: Implement common methods like borrowBook() and returnBook() differently for
Student and Faculty. Use files to save data such as user details, book records, and borrowing
transactions.
4. File system:
• When the program starts, load data from files to initialize the library system.
• When the program shuts down, save the current state of the library system back to files.
• Use appropriate file handling techniques to serialize and deserialize the data.
*/

// Forward declarations
using namespace std;

class Book;
class User;
class Student;
class Faculty;
class Librarian;
class Account;
class Library;

class Book {
private:
    string title;
    string author;
    string publisher;
    int year;
    string ISBN;
    string status; // Available, Borrowed, Reserved
    int borrowerId; // ID of the user who borrowed the book (0 if not borrowed)
    time_t borrowDate; // Date when the book was borrowed
    time_t dueDate; // Date when the book is due to be returned

public:
    // Constructors
    Book();
    Book(const string& title, const string& author, const string& publisher, int year, const string& ISBN);

    // Getters
    string getTitle() const;
    string getAuthor() const;
    string getPublisher() const;
    int getYear() const;
    string getISBN() const;
    string getStatus() const;
    int getBorrowerId() const;
    time_t getBorrowDate() const;
    time_t getDueDate() const;

    // Setters
    void setTitle(const string& title);
    void setAuthor(const string& author);
    void setPublisher(const string& publisher);
    void setYear(int year);
    void setISBN(const string& ISBN);
    void setStatus(const string& status);
    void setBorrowerId(int id);
    void setBorrowDate(time_t date);
    void setDueDate(time_t date);

    // Display book details
    void displayDetails() const;

    // File I/O
    void saveToFile(ofstream& outFile) const;
    static Book loadFromFile(ifstream& inFile);
};

// User base class
class User {
protected:
    int id;
    string name;
    string email;
    string password;
    string role;

public:
    // Constructors
    User();
    User(int id, const string& name, const string& email, const string& password, const string& role);

    // Getters
    int getId() const;
    string getName() const;
    string getEmail() const;
    string getPassword() const;
    string getRole() const;

    // Setters
    void setId(int id);
    void setName(const string& name);
    void setEmail(const string& email);
    void setPassword(const string& password);
    void setRole(const string& role);

    // Display user details
    virtual void displayDetails() const;

    // Virtual functions for borrowing and returning books
    virtual bool borrowBook(Book& book, time_t currentDate) = 0;
    virtual bool returnBook(Book& book, time_t currentDate) = 0;

    // File I/O
    virtual void saveToFile(ofstream& outFile) const;
    static User* loadFromFile(ifstream& inFile);

    // Virtual destructor for proper cleanup of derived classes
    virtual ~User() {}
};

// Student class derived from User
class Student : public User {
private:
    static const int MAX_BOOKS = 3;
    static const int BORROW_PERIOD = 15 * 60; // 15 minutes in seconds for testing
    static const int FINE_RATE = 10; // rupees per day

public:
    // Constructors
    Student();
    Student(int id, const string& name, const string& email, const string& password);

    // Override borrowBook and returnBook functions
    bool borrowBook(Book& book, time_t currentDate) override;
    bool returnBook(Book& book, time_t currentDate) override;

    // Get borrow period in seconds
    static int getBorrowPeriod();

    // Get fine rate in rupees per day
    static int getFineRate();

    // Get maximum number of books that can be borrowed
    static int getMaxBooks();

    // File I/O
    void saveToFile(ofstream& outFile) const override;
    static Student* loadFromFile(ifstream& inFile);
};

// Faculty class derived from User
class Faculty : public User {
private:
    static const int MAX_BOOKS = 5;
    static const int BORROW_PERIOD = 30 * 60; // 30 minutes in seconds for testing
    static const int MAX_OVERDUE_DAYS = 60; // 60 minutes for testing

public:
    // Constructors
    Faculty();
    Faculty(int id, const string& name, const string& email, const string& password);

    // Override borrowBook and returnBook functions
    bool borrowBook(Book& book, time_t currentDate) override;
    bool returnBook(Book& book, time_t currentDate) override;

    // Get borrow period in seconds
    static int getBorrowPeriod();

    // Get maximum number of books that can be borrowed
    static int getMaxBooks();

    // Get maximum overdue days
    static int getMaxOverdueDays();

    // File I/O
    void saveToFile(ofstream& outFile) const override;
    static Faculty* loadFromFile(ifstream& inFile);

};

// Librarian class derived from User
class Librarian : public User {
public:
    // Constructors
    Librarian();
    Librarian(int id, const string& name, const string& email, const string& password);

    // Override borrowBook and returnBook functions (Librarians cannot borrow books)
    bool borrowBook(Book& book, time_t currentDate) override;
    bool returnBook(Book& book, time_t currentDate) override;

    // File I/O
    void saveToFile(ofstream& outFile) const override;
    static Librarian* loadFromFile(ifstream& inFile);
};

// Account class to track user activity
class Account {
private:
    int userId;
    vector<string> borrowedBooks; // ISBNs of currently borrowed books
    vector<string> borrowHistory; // ISBNs of previously borrowed books
    double fines;
    bool hasPaidFines;
    
    public:
    // Constructors
    Account();
    Account(int userId);

    // Getters
    int getUserId() const;
    vector<string> getBorrowedBooks() const;
    vector<string> getBorrowHistory() const;
    double getFines() const;
    bool getHasPaidFines() const;

    // Setters
    void setUserId(int userId);
    void setBorrowedBooks(const vector<string>& books);
    void setBorrowHistory(const vector<string>& history);
    void setFines(double fines);
    void setHasPaidFines(bool paid);

    // Account operations
    void addBorrowedBook(const string& ISBN);
    void removeBorrowedBook(const string& ISBN);
    void addToBorrowHistory(const string& ISBN);
    void addFine(double amount);
    void payFines();

    // Display account details
    void displayDetails() const;
    void displayBorrowedBooks(const map<string, Book>& books) const;
    void displayBorrowHistory(const map<string, Book>& books) const;

    // File I/O
    void saveToFile(ofstream& outFile) const;
    static Account loadFromFile(ifstream& inFile);
};

// Library class to manage the entire system
class Library{

private:
    map<int, User*> users;
    map<string, Book> books;
    map<int, Account> accounts;
    int currentUserId;
    string dataDirectory;

    // CLI helper methods
    void clearScreen();
    void displayHeader();
    void displayLoginMenu();
    void displayStudentMenu();
    void displayFacultyMenu();
    void displayLibrarianMenu();
    void displayBooksManagementMenu();
    void displayUserManagementMenu();
    void displaySystemReportsMenu();
    bool processLoginMenuChoice(const string& choice);
    void processStudentMenuChoice(const string& choice);
    void processFacultyMenuChoice(const string& choice);
    void processLibrarianMenuChoice(const string& choice);
    void processLibrarianBooksMenuChoice(const string& choice);
    void processLibrarianUsersMenuChoice(const string& choice);
    void processLibrarianReportsMenuChoice(const string& choice);
    // Helper methods
    void addInitialData();
    void saveData();
    void loadData();
    time_t getCurrentDate() const;
    string formatDate(time_t date) const;
    int calculateOverdueDays(time_t dueDate, time_t currentDate) const;

public:
    // Constructor and destructor
    Library(const string& dataDir = "data");
    ~Library();

    // Library management
    void addBook(const Book& book);
    void removeBook(const string& ISBN);
    void updateBook(const Book& book);
    void displayAllBooks() const;
    void searchBooks(const string& keyword) const;

    // User management
    void addUser(User* user);
    void removeUser(int userId);
    void displayAllUsers() const;
    User* findUser(int userId) const;
    Book* findBook(const string& ISBN);
    Account* findAccount(int userId);

    // Book operations
    bool borrowBook(int userId, const string& ISBN);
    bool returnBook(int userId, const string& ISBN);
    void checkOverdueBooks();
    void calculateFines();

    // Authentication
    bool login(int userId, const string& password);
    void logout();
    bool isLoggedIn() const;
    User* getCurrentUser() const;    

    // Account operations
    void displayUserAccount() const; // Remove userId parameter
    void settleFines(int userId);

    // Run the library system
    void run();
};

#endif
