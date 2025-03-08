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
In the main code() first build the library with at least 10 books, 5 students , 3 faculty and 1 librarian.
Ensure that permissions to access methods are granted based on the role of the user account in Account
class.
The classes suggested here are just exemplary. Feel free to be a bit creative by adding your own data
members or member functions such that the system is implemented in a more efficient way.
*/


// Forward declarations
class Book;
class User;
class Student;
class Faculty;
class Librarian;
class Account;
class Library;

// Book class to represent books in the library
class Book {
private:
    std::string title;
    std::string author;
    std::string publisher;
    int year;
    std::string ISBN;
    std::string status; // Available, Borrowed, Reserved
    int borrowerId; // ID of the user who borrowed the book (0 if not borrowed)
    time_t borrowDate; // Date when the book was borrowed
    time_t dueDate; // Date when the book is due to be returned

public:
    // Constructors
    Book();
    Book(const std::string& title, const std::string& author, const std::string& publisher, int year, const std::string& ISBN);

    // Getters
    std::string getTitle() const;
    std::string getAuthor() const;
    std::string getPublisher() const;
    int getYear() const;
    std::string getISBN() const;
    std::string getStatus() const;
    int getBorrowerId() const;
    time_t getBorrowDate() const;
    time_t getDueDate() const;

    // Setters
    void setTitle(const std::string& title);
    void setAuthor(const std::string& author);
    void setPublisher(const std::string& publisher);
    void setYear(int year);
    void setISBN(const std::string& ISBN);
    void setStatus(const std::string& status);
    void setBorrowerId(int id);
    void setBorrowDate(time_t date);
    void setDueDate(time_t date);

    // Display book details
    void displayDetails() const;

    // File I/O
    void saveToFile(std::ofstream& outFile) const;
    static Book loadFromFile(std::ifstream& inFile);
};

// User base class
class User {
protected:
    int id;
    std::string name;
    std::string email;
    std::string password;
    std::string role;

public:
    // Constructors
    User();
    User(int id, const std::string& name, const std::string& email, const std::string& password, const std::string& role);

    // Getters
    int getId() const;
    std::string getName() const;
    std::string getEmail() const;
    std::string getPassword() const;
    std::string getRole() const;

    // Setters
    void setId(int id);
    void setName(const std::string& name);
    void setEmail(const std::string& email);
    void setPassword(const std::string& password);
    void setRole(const std::string& role);

    // Display user details
    virtual void displayDetails() const;

    // Virtual functions for borrowing and returning books
    virtual bool borrowBook(Book& book, time_t currentDate) = 0;
    virtual bool returnBook(Book& book, time_t currentDate) = 0;

    // File I/O
    virtual void saveToFile(std::ofstream& outFile) const;
    static User* loadFromFile(std::ifstream& inFile);

    // Virtual destructor for proper cleanup of derived classes
    virtual ~User() {}
};

// Student class derived from User
class Student : public User {
private:
    static const int MAX_BOOKS = 3;
    static const int BORROW_PERIOD = 15 * 24 * 60 * 60; // 15 days in seconds
    static const int FINE_RATE = 10; // rupees per day

public:
    // Constructors
    Student();
    Student(int id, const std::string& name, const std::string& email, const std::string& password);

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
    void saveToFile(std::ofstream& outFile) const override;
    static Student* loadFromFile(std::ifstream& inFile);
};

// Faculty class derived from User
class Faculty : public User {
private:
    static const int MAX_BOOKS = 5;
    static const int BORROW_PERIOD = 30 * 24 * 60 * 60; // 30 days in seconds
    static const int MAX_OVERDUE_DAYS = 60; // Maximum overdue days before blocking

public:
    // Constructors
    Faculty();
    Faculty(int id, const std::string& name, const std::string& email, const std::string& password);

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
    void saveToFile(std::ofstream& outFile) const override;
    static Faculty* loadFromFile(std::ifstream& inFile);
};

// Librarian class derived from User
class Librarian : public User {
public:
    // Constructors
    Librarian();
    Librarian(int id, const std::string& name, const std::string& email, const std::string& password);

    // Override borrowBook and returnBook functions (Librarians cannot borrow books)
    bool borrowBook(Book& book, time_t currentDate) override;
    bool returnBook(Book& book, time_t currentDate) override;

    // File I/O
    void saveToFile(std::ofstream& outFile) const override;
    static Librarian* loadFromFile(std::ifstream& inFile);
};

// Account class to track user activity
class Account {
private:
    int userId;
    std::vector<std::string> borrowedBooks; // ISBNs of currently borrowed books
    std::vector<std::string> borrowHistory; // ISBNs of previously borrowed books
    double fines;
    bool hasPaidFines;

public:
    // Constructors
    Account();
    Account(int userId);

    // Getters
    int getUserId() const;
    std::vector<std::string> getBorrowedBooks() const;
    std::vector<std::string> getBorrowHistory() const;
    double getFines() const;
    bool getHasPaidFines() const;

    // Setters
    void setUserId(int userId);
    void setBorrowedBooks(const std::vector<std::string>& books);
    void setBorrowHistory(const std::vector<std::string>& history);
    void setFines(double fines);
    void setHasPaidFines(bool paid);

    // Account operations
    void addBorrowedBook(const std::string& ISBN);
    void removeBorrowedBook(const std::string& ISBN);
    void addToBorrowHistory(const std::string& ISBN);
    void addFine(double amount);
    void payFines();

    // Display account details
    void displayDetails() const;
    void displayBorrowedBooks(const std::map<std::string, Book>& books) const;
    void displayBorrowHistory(const std::map<std::string, Book>& books) const;

    // File I/O
    void saveToFile(std::ofstream& outFile) const;
    static Account loadFromFile(std::ifstream& inFile);
};

// Library class to manage the entire system
class Library{

private:
    std::map<int, User*> users;
    std::map<std::string, Book> books;
    std::map<int, Account> accounts;
    int currentUserId;
    std::string dataDirectory;

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
    void processMenuChoice(const std::string& choice);
    void processLoginMenuChoice(const std::string& choice);
    void processStudentMenuChoice(const std::string& choice);
    void processFacultyMenuChoice(const std::string& choice);
    void processLibrarianBooksMenuChoice(const std::string& choice, std::string& submenu);
    void processLibrarianUsersMenuChoice(const std::string& choice, std::string& submenu);
    void processLibrarianReportsMenuChoice(const std::string& choice, std::string& submenu);
    // Helper methods
    void addInitialData();
    void saveData();
    void loadData();
    time_t getCurrentDate() const;
    std::string formatDate(time_t date) const;
    int calculateOverdueDays(time_t dueDate, time_t currentDate) const;

public:
    // Constructor and destructor
    Library(const std::string& dataDir = "data");
    ~Library();

    // Library management
    void addBook(const Book& book);
    void removeBook(const std::string& ISBN);
    void updateBook(const Book& book);
    void displayAllBooks() const;
    void searchBooks(const std::string& keyword) const;

    // User management
    void addUser(User* user);
    void removeUser(int userId);
    void displayAllUsers() const;
    User* findUser(int userId) const;
    Book* findBook(const std::string& ISBN);
    Account* findAccount(int userId);

    // Book operations
    bool borrowBook(int userId, const std::string& ISBN);
    bool returnBook(int userId, const std::string& ISBN);
    void checkOverdueBooks();
    void calculateFines();

    // Authentication
    bool login(int userId, const std::string& password);
    void logout();
    bool isLoggedIn() const;
    User* getCurrentUser() const;    

    // Account operations
    void displayUserAccount() const; // Remove userId parameter
    void payFines(int userId);

    // Run the library system
    void run();
};

#endif
