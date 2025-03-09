#include "lms.h"
#include <ctime>
#include <filesystem>
#include <algorithm>
#include <limits>
#include <chrono>
#include <iomanip>

using namespace std;

// Library class implementation
Library::Library(const string& dataDir) : currentUserId(0), dataDirectory(dataDir){
    filesystem::create_directories(dataDirectory);     // Create data directory if it doesn't exist.
    loadData();                                             // Load data from files if they exist.
}

// Destructor
Library::~Library() {
    saveData();                                             // Save data to files before exiting.
    for (auto& pair : users)
        delete pair.second;                                 // Free memory allocated for User objects
    books.clear();
    users.clear();
    accounts.clear();
}

// Helper methods
void Library::clearScreen() {
    #ifdef _WIN32
        system("cls"); // Clear screen for Windows
    #else
        system("clear"); // Clear screen for Linux and MacOS
    #endif
}
void Library::saveData() {
    ofstream booksFile(dataDirectory + "/books.txt");
    ofstream usersFile(dataDirectory + "/users.txt");
    ofstream accountsFile(dataDirectory + "/accounts.txt");

    if (!booksFile || !usersFile || !accountsFile) {
        cerr << "Error: Unable to open files for saving data." << endl;
        return;
    }
    for (const auto& pair : books) {
        pair.second.saveToFile(booksFile);
    }
    for (const auto& pair : users) {
        pair.second->saveToFile(usersFile);
    }
    for (const auto& pair : accounts) {
        pair.second.saveToFile(accountsFile);
    }
    booksFile.close();
    usersFile.close();
    accountsFile.close();
}
void Library::loadData() {
    ifstream booksFile(dataDirectory + "/books.txt");
    ifstream usersFile(dataDirectory + "/users.txt");
    ifstream accountsFile(dataDirectory + "/accounts.txt");

    if (!booksFile) cerr << "Warning: books.txt not found. Starting with empty library." << endl;
    if (!usersFile) cerr << "Warning: users.txt not found. Starting with no users." << endl;
    if (!accountsFile) cerr << "Warning: accounts.txt not found. Starting with no accounts." << endl;

    // Load Books, Users, and Accounts
    while (!booksFile.eof()) {
        Book book = Book::loadFromFile(booksFile);
        if (booksFile) books[book.getISBN()] = book;
    }
    while (!usersFile.eof()) {
        User* user = User::loadFromFile(usersFile);
        if (user) users[user->getId()] = user;
    }
    while (!accountsFile.eof()) {
        Account account = Account::loadFromFile(accountsFile);
        if (accountsFile) accounts[account.getUserId()] = account;
    }
    booksFile.close();
    usersFile.close();
    accountsFile.close();
}   
// Utility: Get current time
time_t Library::getCurrentDate() const {
    return time(nullptr);
}
// Utility: Format time_t to readable string
string Library::formatDate(time_t date) const {
    auto dateChrono = chrono::system_clock::from_time_t(date);
    time_t dateC = chrono::system_clock::to_time_t(dateChrono);
    ostringstream oss;
    oss << put_time(localtime(&dateC), "%c %Z");
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
        cout << "Access denied. Only librarians can add books.\n";
        return;
    }
    books[book.getISBN()] = book;
    cout << "Book added successfully.\n";
}

void Library::removeBook(const string& ISBN) {
    if (!isLoggedIn() || getCurrentUser()->getRole() != "Librarian") {
        cout << "Access denied. Only librarians can remove books.\n";
        return;
    }
    books.erase(ISBN);
    cout << "Book removed successfully.\n";
}
void Library::updateBook(const Book& book) {
    if (!isLoggedIn() || getCurrentUser()->getRole() != "Librarian") {
        cout << "Access denied. Only librarians can add books.\n";
        return;
    }
    books[book.getISBN()] = book;
    cout << "Book updated successfully.\n";
}

void Library::displayAllBooks() const {
    for (const auto& pair : books)
        pair.second.displayDetails();
}
void Library::searchBooks(const string& keyword) const {
    for (const auto& pair : books) {
        if (pair.second.getTitle().find(keyword) != string::npos ||
            pair.second.getAuthor().find(keyword) != string::npos ||
            pair.second.getISBN().find(keyword) != string::npos) {
            pair.second.displayDetails();
        }
    }
}

// ----- User Management -----
void Library::addUser(User* user) {
    if (!isLoggedIn() || getCurrentUser()->getRole() != "Librarian") {
        cout << "Access denied. Only librarians can add users.\n";
        return;
    }

    int librarianId = currentUserId; // Store the current librarian's ID

    users[user->getId()] = user;
    accounts[user->getId()] = Account(user->getId());
    cout << "User added successfully.\n";

    currentUserId = librarianId; // Restore the librarian's ID
}
void Library::removeUser(int userId) {
    if (!isLoggedIn() || getCurrentUser()->getRole() != "Librarian") {
        cout << "Access denied. Only librarians can remove users.\n";
        return;
    }    
    users.erase(userId);
    accounts.erase(userId);
    cout << "User removed successfully.\n";
}
void Library::displayAllUsers() const {
    if (!isLoggedIn() || getCurrentUser()->getRole() != "Librarian") {
        cout << "Access denied. Only librarians can display users.\n";
        return;
    }
    for (const auto& pair : users)
        pair.second->displayDetails();
}

User* Library::findUser(int userId) const { 
    auto it = users.find(userId);
    return (it != users.end()) ? it->second : nullptr;
}
Book* Library::findBook(const string& ISBN) {
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

/*
• Borrowing Rules:
• Students and faculty can borrow books based on their role constraints.
• If a user tries to borrow more than the allowed number of books, the system should
  deny the request.
• If a user has unpaid fines, borrowing new books should not be allowed until the fines
  are cleared.
• The system should provide an option for users to simulate payment of fines.
• Users can view their total outstanding fines and mark them as paid through a dedi-
  cated menu option.
• Once the payment is made, the fine amount should reset to zero, and borrowing
  restrictions should be lifted.
*/

bool Library::borrowBook(int userId, const string& ISBN) {
    User* user = findUser(userId);
    Book* book = findBook(ISBN);
    Account* account = findAccount(userId);
    // Trivial errors : Invalid user or book, book not available, librarian borrowing.
    if (!user || !book || !account) {
        cerr << "Invalid user or book." << endl;
        return false;
    }
    if (book->getStatus() != "Available") {
        cout << "Book is not available for borrowing." << endl;
        return false;
    }
    if(user->getRole() == "Librarian") {
        cout << "Librarians cannot borrow books." << endl;
        return false;
    }
    else if (user->getRole() == "Faculty") {
        if(account->getBorrowedBooks().size() >= static_cast<size_t>(Faculty::getMaxBooks())) {
            cout << "Faculty members can borrow only " << Faculty::getMaxBooks() << " books at a time." << endl;
            return false;
        }
        // Check if faculty has overdue books
        for (const string& borrowedISBN : account->getBorrowedBooks()) {
            Book* borrowedBook = findBook(borrowedISBN);
            if (borrowedBook && calculateOverdueDays(borrowedBook->getDueDate(), getCurrentDate()) > Faculty::getMaxOverdueDays()) {
                cout << "Faculty members cannot borrow new books if they have overdue books for more than " << Faculty::getMaxOverdueDays() << " days." << endl;
                return false;
            }
        }
    }
    else if(user->getRole() == "Student") {
        if(account->getBorrowedBooks().size() >= static_cast<size_t>(Student::getMaxBooks())) {
            cout << "Students can borrow only " << Student::getMaxBooks() << " books at a time." << endl;
            return false;
        }
        if (account->getFines() > 0) {
            cout << "Please clear your outstanding fines before borrowing new books." << endl;
            return false;
        }
    }
    else {
        cerr << "Invalid user role." << endl;
        return false;
    }

    // Borrow the book (using polymorphism)
    time_t currentDate = getCurrentDate();
    if (user->borrowBook(*book, currentDate)) {
        account->addBorrowedBook(ISBN);
        account->addToBorrowHistory(ISBN);
        cout << "Book borrowed successfully." << endl;
        return true;
    }
    return false;
}
/*
• Returning and Updating Rules:
• Update Book Status:
    Upon the return of a book, its status should be updated to “Available” in the
    system.
• Fine Calculation:
    ∗ For Students:
        Fine = Days Overdue ×10 rupees/day.
    ∗ For Faculty:
        · No fine for overdue books.
        · Faculty members cannot borrow additional books if they (a) Have already reached
        the limit of 5 borrowed books, or (b) Have an overdue book for more than 60
        days.
• Overdue Check:
    If the book is returned after the borrowing period (15 days for students, 30 days for
    faculty), the system should:
    ∗ Calculate the overdue period.
    ∗ Display to User Side
• User Account Update:
    ∗ Remove the book from the current borrow list in the user’s account.
    ∗ Add the book to the borrowing history.
    • Borrowing Eligibility:
    ∗ If fines exist, prevent further borrowing until the fine is cleared.
    ∗ If the user has overdue books, prevent borrowing until the overdue books are
      returned.
*/
bool Library::returnBook(int userId, const string& ISBN) {
    User* user = findUser(userId);
    Book* book = findBook(ISBN);
    Account* account = findAccount(userId);
    if (!user || !book || !account) {
        cerr << "Invalid user or book." << endl;
        return false;
    }
    time_t currentDate = getCurrentDate();
    bool fineApplicable = user->returnBook(*book, currentDate);
    account->removeBorrowedBook(ISBN);
    account->addToBorrowHistory(ISBN);
    book->setStatus("Available");
    
    if (fineApplicable) {
        int overdueDays = calculateOverdueDays(book->getDueDate(), currentDate);
        if (overdueDays > 0 && user->getRole() == "Student") {
            int fine = overdueDays * Student::getFineRate();
            account->addFine(fine);
            cout << "Book returned. Overdue by " << overdueDays 
                      << " days. Fine: Rs." << fine << endl;
        } else {
            cout << "Book returned successfully." << endl;
        }
    } else {
        cout << "Book returned successfully." << endl;
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
                cout << "Book \"" << book.getTitle() << "\" is overdue by " 
                          << overdueDays << " days." << endl;
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
            for (const string& isbn : account.getBorrowedBooks()) {
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

bool Library::login(int userId, const string& password) {
    User* user = findUser(userId);
    if (!user) {
        cout << "User ID not found.\n";
        return false;
    }
    if (user->getPassword() == password) {
        currentUserId = userId;
        cout << "Login successful. Welcome, " << user->getName() << "!\n";
        return true;
    } else {
        cout << "Incorrect password.\n";
        return false;
    }
}
void Library::logout() {
    currentUserId = 0;
    clearScreen();
    cout << "Logged out successfully.\n";
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
        cout << "Please log in first.\n";
        return;
    }
    int userId = currentUserId;  // Use the logged-in user ID
    auto it = accounts.find(userId);

    if (it == accounts.end()) {
        cout << "No account found for this user.\n";
        return;
    }
    cout << "Account details for " << getCurrentUser()->getName() << ":\n";
    it->second.displayDetails();
}

void Library::settleFines(int userId) {
    Account* account = findAccount(userId);
    if (account) {
        account->payFines();
        cout << "Fines settled successfully for user ID: " << userId << endl;
    } else {
        cout << "Account not found for user ID: " << userId << endl;
    }
}

// ----- Run the Library System (Simple CLI) -----
void Library::run(){
    bool running = true;
    string input;

    while(running) {
        // clearScreen();
        if (!isLoggedIn()){
            displayLoginMenu();
        } 
        else{
            User* currentUser = getCurrentUser();
            string role = currentUser->getRole();
            clearScreen();
            displayHeader();
            if (role == "Student") {
                displayStudentMenu();
            } else if (role == "Faculty") {
                displayFacultyMenu();
            } else if (role == "Librarian") {
                displayLibrarianMenu();
            }
        }
        
        cout << "\nEnter your choice (or 'q' to quit): ";
        getline(cin, input);
        
        if (input == "q" || input == "Q"){
            running = false;
        }
        else if(isLoggedIn()){
            User* currentUser = getCurrentUser();
            string role = currentUser->getRole();
            if(role == "Librarian"){
                processLibrarianMenuChoice(input);
            } else if(role == "Student"){
                processStudentMenuChoice(input);
            } else if(role == "Faculty"){
                processFacultyMenuChoice(input);
            }
        }
        else{
            bool choiceflag = processLoginMenuChoice(input);
            if(!choiceflag){
                running = false;
            }
        }
    }
    // Save data before exiting
    saveData();
    cout << "Thank you for using the Library Management System. Goodbye!\n";

}

// void Library::clearScreen() {
//     // #ifdef _WIN32
//     //     system("cls");
//     // #else
//     //     system("clear");
//     // #endif
//     system("clear");
// }

void Library::displayHeader() {
    cout << "==================================================\n";
    cout << "           LIBRARY MANAGEMENT SYSTEM              \n";
    cout << "==================================================\n";
    
    if (isLoggedIn()) {
        User* user = getCurrentUser();
        cout << "Logged in as: " << user->getName() << " (" << user->getRole() << ")\n";
        
        // For students, show fines if any
        if (user->getRole() == "Student") {
            Account* account = findAccount(user->getId());
            if (account && account->getFines() > 0) {
                cout << "Outstanding fines: Rs. " << account->getFines() << "\n";
            }
        }
    }
    
    cout << "--------------------------------------------------\n";
}

void Library::displayLoginMenu() {
    displayHeader();
    cout << "\n1. Login\n";
    cout << "2. Exit\n";
}
void Library::displayStudentMenu() {
    cout << "\nMAIN MENU\n";
    cout << "1. Browse books\n";
    cout << "2. Search books\n";
    cout << "3. My account\n";
    cout << "4. Borrow a book\n";
    cout << "5. Return a book\n";
    cout << "6. Logout\n"; // Removed "Pay fines" option
}

void Library::displayFacultyMenu() {
    cout << "\nMAIN MENU\n";
    cout << "1. Browse books\n";
    cout << "2. Search books\n";
    cout << "3. My account\n";
    cout << "4. Borrow a book\n";
    cout << "5. Return a book\n";
    cout << "6. Logout\n";
}

void Library::displayLibrarianMenu() {
    cout << "\nMAIN MENU\n";
    cout << "1. Books management\n";
    cout << "2. User management\n";
    cout << "3. System reports\n";
    cout << "4. Settle user fines\n"; // Added option to settle fines
    cout << "5. Logout\n";
}

void Library::displayBooksManagementMenu() {
    cout << "\nBOOKS MANAGEMENT\n";
    cout << "1. Display all books\n";
    cout << "2. Search books\n";
    cout << "3. Add a new book\n";
    cout << "4. Update an existing book\n";
    cout << "5. Remove a book\n";
    cout << "6. Back to main menu\n";
}

void Library::displayUserManagementMenu() {
    cout << "\nUSER MANAGEMENT\n";
    cout << "1. Display all users\n";
    cout << "2. Add a new user\n";
    cout << "3. Remove a user\n";
    cout << "4. Back to main menu\n";
}

void Library::displaySystemReportsMenu() {
    cout << "\nSYSTEM REPORTS\n";
    cout << "1. Overdue books report\n";
    cout << "2. User fines report\n";
    cout << "3. Most borrowed books\n";
    cout << "4. Back to main menu\n";
}

void Library::processLibrarianMenuChoice(const string& choice) {
    if (choice == "1") { // Books management
        clearScreen();
        displayHeader();
        displayBooksManagementMenu();
        string booksChoice;
        cout << "\nEnter your choice: ";
        getline(cin, booksChoice);
        processLibrarianBooksMenuChoice(booksChoice);
    } else if (choice == "2") { // User management
        clearScreen();
        displayHeader();
        displayUserManagementMenu();
        string usersChoice;
        cout << "\nEnter your choice: ";
        getline(cin, usersChoice);
        processLibrarianUsersMenuChoice(usersChoice);
    } else if (choice == "3") { // System reports
        clearScreen();
        displayHeader();
        displaySystemReportsMenu();
        string reportsChoice;
        cout << "\nEnter your choice: ";
        getline(cin, reportsChoice);
        processLibrarianReportsMenuChoice(reportsChoice);
    } else if (choice == "4") { // Settle user fines
        int userId;
        cout << "Enter user ID to settle fines: ";
        cin >> userId;
        cin.ignore(); // Clear newline
        settleFines(userId);
        cout << "\nPress Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    } else if (choice == "5") { // Logout
        logout();
    } else {
        cout << "Invalid choice. Please try again.\n";
    }
}

bool Library::processLoginMenuChoice(const string& choice) {
    if(choice == "1") { // Login
        int userId;
        string password;
        
        cout << "Enter user ID: ";
        while (!(cin >> userId)) {
            cin.clear(); // Clear the error flag
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Discard invalid input
            cout << "Invalid input. Please enter a valid user ID: ";
        }
        cin.ignore(); // Clear newline
        cout << "Enter password: ";
        getline(cin, password);
        bool loginflag = login(userId, password);
        if(!loginflag){
            clearScreen();
            cout << "Invalid user ID or password. Please try again.\n";
        }
    } 
    else if(choice != "2"){ // Not exit
        cout << "Invalid choice. Please try again.\n";
    }
    else{
        cout << "Exiting...\n";
        return false;
    }
    return true;
}

void Library::processStudentMenuChoice(const string& choice) {
    switch (stoi(choice)) {
        case 1: // Browse books
            clearScreen();
            displayHeader();
            cout << "\nALL BOOKS:\n";
            displayAllBooks();
            cout << "\nPress Enter to continue...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break;
        case 2: { // Search books
            string keyword;
            cout << "Enter search keyword: ";
            getline(cin, keyword);
            clearScreen();
            displayHeader();
            cout << "\nSEARCH RESULTS FOR '" << keyword << "':\n";
            searchBooks(keyword);
            cout << "\nPress Enter to continue...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break;
        }
        case 3: // My account
            clearScreen();
            displayHeader();
            displayUserAccount();
            cout << "\nPress Enter to continue...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break;
        case 4: { // Borrow a book
            string ISBN;
            cout << "Enter ISBN of the book to borrow: ";
            getline(cin, ISBN);
            
            borrowBook(getCurrentUser()->getId(), ISBN);
            cout << "\nPress Enter to continue...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break;
        }
        case 5: { // Return a book
            string ISBN;
            cout << "Enter ISBN of the book to return: ";
            getline(cin, ISBN);
            
            returnBook(getCurrentUser()->getId(), ISBN);
            cout << "\nPress Enter to continue...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break;
        }
        case 6: // Logout
            logout();
            break;
        default:
            cout << "Invalid choice. Please try again.\n";
            break;
    }
}

void Library::processFacultyMenuChoice(const string& choice) {
    switch (stoi(choice)) {
        case 1: // Browse books
            clearScreen();
            displayHeader();
            cout << "\nALL BOOKS:\n";
            displayAllBooks();
            cout << "\nPress Enter to continue...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break;
        case 2: { // Search books
            string keyword;
            cout << "Enter search keyword: ";
            getline(cin, keyword);
            
            clearScreen();
            displayHeader();
            cout << "\nSEARCH RESULTS FOR '" << keyword << "':\n";
            searchBooks(keyword);
            cout << "\nPress Enter to continue...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break;
        }
        case 3: // My account
            clearScreen();
            displayHeader();
            displayUserAccount();
            cout << "\nPress Enter to continue...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break;
        case 4: { // Borrow a book
            string ISBN;
            cout << "Enter ISBN of the book to borrow: ";
            getline(cin, ISBN);
            
            borrowBook(getCurrentUser()->getId(), ISBN);
            cout << "\nPress Enter to continue...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break;
        }
        case 5: { // Return a book
            string ISBN;
            cout << "Enter ISBN of the book to return: ";
            getline(cin, ISBN);
            
            returnBook(getCurrentUser()->getId(), ISBN);
            cout << "\nPress Enter to continue...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            break;
        }
        case 6: // Logout
            logout();
            break;
        default:
            cout << "Invalid choice. Please try again.\n";
            break;
    }
}

void Library::processLibrarianBooksMenuChoice(const string& choice) {
    if (choice == "1") { // Display all books
        clearScreen();
        displayHeader();
        cout << "\nALL BOOKS:\n";
        displayAllBooks();
        cout << "\nPress Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    } else if (choice == "2") { // Search books
        string keyword;
        cout << "Enter search keyword: ";
        getline(cin, keyword);
        
        clearScreen();
        displayHeader();
        cout << "\nSEARCH RESULTS FOR '" << keyword << "':\n";
        searchBooks(keyword);
        cout << "\nPress Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    } else if (choice == "3") { // Add a new book
        string title, author, publisher, ISBN;
        int year;
        
        cout << "Enter book title: ";
        getline(cin, title);
        
        cout << "Enter author: ";
        getline(cin, author);
        
        cout << "Enter publisher: ";
        getline(cin, publisher);
        
        cout << "Enter publication year: ";
        cin >> year;
        cin.ignore(); // Clear newline
        
        cout << "Enter ISBN: ";
        getline(cin, ISBN);
        
        Book newBook(title, author, publisher, year, ISBN);
        addBook(newBook);
        cout << "\nPress Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    } else if (choice == "4") { // Update a book
        string ISBN;
        cout << "Enter ISBN of the book to update: ";
        getline(cin, ISBN);
        
        Book* book = findBook(ISBN);
        if (book) {
            string title, author, publisher;
            int year;
            
            cout << "Enter new title (or press Enter to keep current): ";
            getline(cin, title);
            if (!title.empty()) book->setTitle(title);
            
            cout << "Enter new author (or press Enter to keep current): ";
            getline(cin, author);
            if (!author.empty()) book->setAuthor(author);
            
            cout << "Enter new publisher (or press Enter to keep current): ";
            getline(cin, publisher);
            if (!publisher.empty()) book->setPublisher(publisher);
            
            cout << "Enter new publication year (or 0 to keep current): ";
            cin >> year;
            cin.ignore(); // Clear newline
            if (year != 0) book->setYear(year);
            
            updateBook(*book);
        } else {
            cout << "Book not found.\n";
        }
        
        cout << "\nPress Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    } else if (choice == "5") { // Remove a book
        string ISBN;
        cout << "Enter ISBN of the book to remove: ";
        getline(cin, ISBN);
        
        removeBook(ISBN);
        cout << "\nPress Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    } else if (choice == "6") { // Back to main menu
    } else {
        cout << "Invalid choice. Please try again.\n";
    }
}

void Library::processLibrarianUsersMenuChoice(const string& choice) {
    if (choice == "1") { // Display all users
        clearScreen();
        displayHeader();
        cout << "\nALL USERS:\n";
        displayAllUsers();
        cout << "\nPress Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    } 
    else if (choice == "2") { // Add a new user
        int id, userType;
        string name, email, password;
        
        cout << "Enter user ID: ";
        cin >> id;
        cin.ignore(); // Clear newline
        
        cout << "Enter name: ";
        getline(cin, name);
        
        cout << "Enter email: ";
        getline(cin, email);
        
        cout << "Enter password: ";
        getline(cin, password);

        cout << "Select user type:\n";
        cout << "1. Student\n";
        cout << "2. Faculty\n";
        cout << "3. Librarian\n";
        cout << "Enter choice: ";
        cin >> userType;
        cin.ignore(); // Clear newline
        
        User* newUser = nullptr;
        switch (userType) {
            case 1:
                newUser = new Student(id, name, email, password);
                break;
            case 2:
                newUser = new Faculty(id, name, email, password);
                break;
            case 3:
                newUser = new Librarian(id, name, email, password);
                break;
            default:
                cout << "Invalid user type.\n";
                break;
        }
        
        if (newUser) {
            addUser(newUser);
        }
        
        cout << "\nPress Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    } else if (choice == "3") { // Remove a user
        int userId;
        cout << "Enter ID of the user to remove: ";
        cin >> userId;
        cin.ignore(); // Clear newline
        
        removeUser(userId);
        cout << "\nPress Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    } else if (choice == "4") { // Back to main menu
    } else {
        cout << "Invalid choice. Please try again.\n";
    }
}

void Library::processLibrarianReportsMenuChoice(const string& choice) {
    if (choice == "1") { // Overdue books report
        clearScreen();
        displayHeader();
        cout << "\nOVERDUE BOOKS REPORT:\n";
        checkOverdueBooks();
        cout << "\nPress Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    } else if (choice == "2") { // User fines report
        clearScreen();
        displayHeader();
        cout << "\nUSER FINES REPORT:\n";
        
        bool foundFines = false;
        for (const auto& pair : accounts) {
            if (pair.second.getFines() > 0) {
                User* user = findUser(pair.first);
                if (user) {
                    cout << "User: " << user->getName() << " (ID: " << user->getId() 
                              << ") - Fine: Rs." << pair.second.getFines() << endl;
                    foundFines = true;
                }
            }
        }
        
        if (!foundFines) {
            cout << "No outstanding fines found.\n";
        }
        
        cout << "\nPress Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    } else if (choice == "3") { // Most borrowed books
        // This would require additional tracking in the Book class
        clearScreen();
        displayHeader();
        cout << "\nMOST BORROWED BOOKS REPORT:\n";
        cout << "This feature is not yet implemented.\n";
        cout << "\nPress Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    } else if (choice == "4") { // Back to main menu
        // submenu = "main";
    } else {
        cout << "Invalid choice. Please try again.\n";
    }
}