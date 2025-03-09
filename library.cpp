#include "lms.h"
#include <ctime>
#include <filesystem>
#include <algorithm>
#include <limits>
#include <chrono>
#include <iomanip>

// Library class implementation
Library::Library(const std::string& dataDir) : currentUserId(0), dataDirectory(dataDir){
    std::filesystem::create_directories(dataDirectory);     // Create data directory if it doesn't exist.
    loadData();                                             // Load data from files if they exist.
}

// Destructor
Library::~Library() {
    saveData();                                             // Save data to files before exiting.
    for (auto& pair : users)
        delete pair.second;                                 // Free memory allocated for User objects
}

// Helper methods
void Library::clearScreen() {
    #ifdef _WIN32
        system("cls"); // Clear screen for Windows
    #else
        std::cout << "\033[2J\033[1;1H"; // Clear screen for Linux and MacOS
    #endif
}
void Library::saveData() {
    std::ofstream booksFile(dataDirectory + "/books.txt");
    std::ofstream usersFile(dataDirectory + "/users.txt");
    std::ofstream accountsFile(dataDirectory + "/accounts.txt");

    if (!booksFile || !usersFile || !accountsFile) {
        std::cerr << "Error: Unable to open files for saving data." << std::endl;
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
    std::ifstream booksFile(dataDirectory + "/books.txt");
    std::ifstream usersFile(dataDirectory + "/users.txt");
    std::ifstream accountsFile(dataDirectory + "/accounts.txt");

    if (!booksFile) std::cerr << "Warning: books.txt not found. Starting with empty library." << std::endl;
    if (!usersFile) std::cerr << "Warning: users.txt not found. Starting with no users." << std::endl;
    if (!accountsFile) std::cerr << "Warning: accounts.txt not found. Starting with no accounts." << std::endl;

    // Load Books, Users, and Accounts
    while (booksFile) {
        Book book = Book::loadFromFile(booksFile);
        if (booksFile) books[book.getISBN()] = book;
    }
    while (usersFile) {
        User* user = User::loadFromFile(usersFile);
        if (user) users[user->getId()] = user;
    }
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

    int librarianId = currentUserId; // Store the current librarian's ID

    users[user->getId()] = user;
    accounts[user->getId()] = Account(user->getId());
    std::cout << "User added successfully.\n";

    currentUserId = librarianId; // Restore the librarian's ID
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
        std::cout << "Access denied. Only librarians can display users.\n";
        return;
    }
    for (const auto& pair : users)
        pair.second->displayDetails();
}

User* Library::findUser(int userId) const { 
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

bool Library::borrowBook(int userId, const std::string& ISBN) {
    User* user = findUser(userId);
    Book* book = findBook(ISBN);
    Account* account = findAccount(userId);
    // Trivial errors : Invalid user or book, book not available, librarian borrowing.
    if (!user || !book || !account) {
        std::cerr << "Invalid user or book." << std::endl;
        return false;
    }
    if (book->getStatus() != "Available") {
        std::cout << "Book is not available for borrowing." << std::endl;
        return false;
    }
    if(user->getRole() == "Librarian") {
        std::cout << "Librarians cannot borrow books." << std::endl;
        return false;
    }
    else if (user->getRole() == "Faculty") {
        if(account->getBorrowedBooks().size() >= Faculty::getMaxBooks()) {
            std::cout << "Faculty members can borrow only " << Faculty::getMaxBooks() << " books at a time." << std::endl;
            return false;
        }
        // Check if faculty has overdue books
        for (const std::string& borrowedISBN : account->getBorrowedBooks()) {
            Book* borrowedBook = findBook(borrowedISBN);
            if (borrowedBook && calculateOverdueDays(borrowedBook->getDueDate(), getCurrentDate()) > Faculty::getMaxOverdueDays()) {
                std::cout << "Faculty members cannot borrow new books if they have overdue books for more than " << Faculty::getMaxOverdueDays() << " days." << std::endl;
                return false;
            }
        }
    }
    else if(user->getRole() == "Student") {
        if(account->getBorrowedBooks().size() >= Student::getMaxBooks()) {
            std::cout << "Students can borrow only " << Student::getMaxBooks() << " books at a time." << std::endl;
            return false;
        }
        if (account->getFines() > 0) {
            std::cout << "Please clear your outstanding fines before borrowing new books." << std::endl;
            return false;
        }
    }
    else {
        std::cerr << "Invalid user role." << std::endl;
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
    account->addToBorrowHistory(ISBN);
    book->setStatus("Available");
    
    if (fineApplicable) {
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
    clearScreen();
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
void Library::run(){
    bool running = true;
    std::string input;

    while(running) {
        // clearScreen();
        if (!isLoggedIn()){
            displayLoginMenu();
        } 
        else{
            User* currentUser = getCurrentUser();
            std::string role = currentUser->getRole();
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
        
        std::cout << "\nEnter your choice (or 'q' to quit): ";
        std::getline(std::cin, input);
        
        if (input == "q" || input == "Q"){
            running = false;
        }
        else if(isLoggedIn()){
            bool choiceflag = true;
            User* currentUser = getCurrentUser();
            std::string role = currentUser->getRole();
            if(role == "Librarian"){
                processLibrarianMenuChoice(input);
            }
            else{
            choiceflag = processMenuChoice(input);
            } 
            if(!choiceflag){
                running = false;
            }
        }
        else{
            processMenuChoice(input);
        }
    }
    // Save data before exiting
    saveData();
    std::cout << "Thank you for using the Library Management System. Goodbye!\n";

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
    std::cout << "==================================================\n";
    std::cout << "           LIBRARY MANAGEMENT SYSTEM              \n";
    std::cout << "==================================================\n";
    
    if (isLoggedIn()) {
        User* user = getCurrentUser();
        std::cout << "Logged in as: " << user->getName() << " (" << user->getRole() << ")\n";
        
        // For students, show fines if any
        if (user->getRole() == "Student") {
            Account* account = findAccount(user->getId());
            if (account && account->getFines() > 0) {
                std::cout << "Outstanding fines: Rs. " << account->getFines() << "\n";
            }
        }
    }
    
    std::cout << "--------------------------------------------------\n";
}

void Library::displayLoginMenu() {
    displayHeader();
    std::cout << "\n1. Login\n";
    std::cout << "2. Exit\n";
}

void Library::displayStudentMenu() {
    std::cout << "\nMAIN MENU\n";
    std::cout << "1. Browse books\n";
    std::cout << "2. Search books\n";
    std::cout << "3. My account\n";
    std::cout << "4. Borrow a book\n";
    std::cout << "5. Return a book\n";
    std::cout << "6. Pay fines\n";
    std::cout << "7. Logout\n";
}

void Library::displayFacultyMenu() {
    std::cout << "\nMAIN MENU\n";
    std::cout << "1. Browse books\n";
    std::cout << "2. Search books\n";
    std::cout << "3. My account\n";
    std::cout << "4. Borrow a book\n";
    std::cout << "5. Return a book\n";
    std::cout << "6. Logout\n";
}

void Library::displayLibrarianMenu() {
    std::cout << "\nMAIN MENU\n";
    std::cout << "1. Books management\n";
    std::cout << "2. User management\n";
    std::cout << "3. System reports\n";
    std::cout << "4. Logout\n";
}

void Library::displayBooksManagementMenu() {
    std::cout << "\nBOOKS MANAGEMENT\n";
    std::cout << "1. Display all books\n";
    std::cout << "2. Search books\n";
    std::cout << "3. Add a new book\n";
    std::cout << "4. Update an existing book\n";
    std::cout << "5. Remove a book\n";
    std::cout << "6. Back to main menu\n";
}

void Library::displayUserManagementMenu() {
    std::cout << "\nUSER MANAGEMENT\n";
    std::cout << "1. Display all users\n";
    std::cout << "2. Add a new user\n";
    std::cout << "3. Remove a user\n";
    std::cout << "4. Back to main menu\n";
}

void Library::displaySystemReportsMenu() {
    std::cout << "\nSYSTEM REPORTS\n";
    std::cout << "1. Overdue books report\n";
    std::cout << "2. User fines report\n";
    std::cout << "3. Most borrowed books\n";
    std::cout << "4. Back to main menu\n";
}

bool Library::processMenuChoice(const std::string& choice){
    if (!isLoggedIn()){
        if(choice == "2"){
            std::cout << "Exiting...\n";
            return false;
        }
        processLoginMenuChoice(choice);
        return true;
    }
    User* currentUser = getCurrentUser();
    std::string role = currentUser->getRole();
    
    if (role == "Student") {
        processStudentMenuChoice(choice);
    } else if (role == "Faculty") {
        processFacultyMenuChoice(choice);
    } else if (role == "Librarian") {
        processLibrarianMenuChoice(choice);
    }
    return true;
}

void Library::processLibrarianMenuChoice(const std::string& choice) {
        if (choice == "1") { // Books management
            clearScreen();
            displayHeader();
            displayBooksManagementMenu();
            std::string booksChoice;
            std::cout << "\nEnter your choice: ";
            std::getline(std::cin, booksChoice);
            processLibrarianBooksMenuChoice(booksChoice);
        } else if (choice == "2") { // User management
            clearScreen();
            displayHeader();
            displayUserManagementMenu();
            std::string usersChoice;
            std::cout << "\nEnter your choice: ";
            std::getline(std::cin, usersChoice);
            processLibrarianUsersMenuChoice(usersChoice);
        } else if (choice == "3") { // System reports
            clearScreen();
            displayHeader();
            displaySystemReportsMenu();
            std::string reportsChoice;
            std::cout << "\nEnter your choice: ";
            std::getline(std::cin, reportsChoice);
            processLibrarianReportsMenuChoice(reportsChoice);
        } else if (choice == "4") { // Logout
            logout();
        } else {
            std::cout << "Invalid choice. Please try again.\n";
        }
    // } else if (submenu == "books") {
    //     processLibrarianBooksMenuChoice(choice, submenu);
    // } else if (submenu == "users") {
    //     processLibrarianUsersMenuChoice(choice, submenu);
    // } else if (submenu == "reports") {
    //     processLibrarianReportsMenuChoice(choice, submenu);
    // }
}

void Library::processLoginMenuChoice(const std::string& choice) {
    if(choice == "1") { // Login
        int userId;
        std::string password;
        
        std::cout << "Enter user ID: ";
        while (!(std::cin >> userId)) {
            std::cin.clear(); // Clear the error flag
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard invalid input
            std::cout << "Invalid input. Please enter a valid user ID: ";
        }
        std::cin.ignore(); // Clear newline
        std::cout << "Enter password: ";
        std::getline(std::cin, password);
        bool loginflag = login(userId, password);
        if(!loginflag){
            clearScreen();
            std::cout << "Invalid user ID or password. Please try again.\n";
        }
    } 
    else if(choice != "2"){ // Not exit
        std::cout << "Invalid choice. Please try again.\n";
    }
    else{
        std::cout << "Exiting...\n";
    }
}

void Library::processStudentMenuChoice(const std::string& choice) {
    switch (std::stoi(choice)) {
        case 1: // Browse books
            clearScreen();
            displayHeader();
            std::cout << "\nALL BOOKS:\n";
            displayAllBooks();
            std::cout << "\nPress Enter to continue...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        case 2: { // Search books
            std::string keyword;
            std::cout << "Enter search keyword: ";
            std::getline(std::cin, keyword);
            clearScreen();
            displayHeader();
            std::cout << "\nSEARCH RESULTS FOR '" << keyword << "':\n";
            searchBooks(keyword);
            std::cout << "\nPress Enter to continue...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        }
        case 3: // My account
            clearScreen();
            displayHeader();
            displayUserAccount();
            std::cout << "\nPress Enter to continue...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        case 4: { // Borrow a book
            std::string ISBN;
            std::cout << "Enter ISBN of the book to borrow: ";
            std::getline(std::cin, ISBN);
            
            borrowBook(getCurrentUser()->getId(), ISBN);
            std::cout << "\nPress Enter to continue...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        }
        case 5: { // Return a book
            std::string ISBN;
            std::cout << "Enter ISBN of the book to return: ";
            std::getline(std::cin, ISBN);
            
            returnBook(getCurrentUser()->getId(), ISBN);
            std::cout << "\nPress Enter to continue...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        }
        case 6: // Pay fines
            payFines(getCurrentUser()->getId());
            std::cout << "\nPress Enter to continue...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        case 7: // Logout
            logout();
            break;
        default:
            std::cout << "Invalid choice. Please try again.\n";
            break;
    }
}

void Library::processFacultyMenuChoice(const std::string& choice) {
    switch (std::stoi(choice)) {
        case 1: // Browse books
            clearScreen();
            displayHeader();
            std::cout << "\nALL BOOKS:\n";
            displayAllBooks();
            std::cout << "\nPress Enter to continue...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        case 2: { // Search books
            std::string keyword;
            std::cout << "Enter search keyword: ";
            std::getline(std::cin, keyword);
            
            clearScreen();
            displayHeader();
            std::cout << "\nSEARCH RESULTS FOR '" << keyword << "':\n";
            searchBooks(keyword);
            std::cout << "\nPress Enter to continue...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        }
        case 3: // My account
            clearScreen();
            displayHeader();
            displayUserAccount();
            std::cout << "\nPress Enter to continue...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        case 4: { // Borrow a book
            std::string ISBN;
            std::cout << "Enter ISBN of the book to borrow: ";
            std::getline(std::cin, ISBN);
            
            borrowBook(getCurrentUser()->getId(), ISBN);
            std::cout << "\nPress Enter to continue...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        }
        case 5: { // Return a book
            std::string ISBN;
            std::cout << "Enter ISBN of the book to return: ";
            std::getline(std::cin, ISBN);
            
            returnBook(getCurrentUser()->getId(), ISBN);
            std::cout << "\nPress Enter to continue...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            break;
        }
        case 6: // Logout
            logout();
            break;
        default:
            std::cout << "Invalid choice. Please try again.\n";
            break;
    }
}

void Library::processLibrarianBooksMenuChoice(const std::string& choice) {
    if (choice == "1") { // Display all books
        clearScreen();
        displayHeader();
        std::cout << "\nALL BOOKS:\n";
        displayAllBooks();
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } else if (choice == "2") { // Search books
        std::string keyword;
        std::cout << "Enter search keyword: ";
        std::getline(std::cin, keyword);
        
        clearScreen();
        displayHeader();
        std::cout << "\nSEARCH RESULTS FOR '" << keyword << "':\n";
        searchBooks(keyword);
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } else if (choice == "3") { // Add a new book
        std::string title, author, publisher, ISBN;
        int year;
        
        std::cout << "Enter book title: ";
        std::getline(std::cin, title);
        
        std::cout << "Enter author: ";
        std::getline(std::cin, author);
        
        std::cout << "Enter publisher: ";
        std::getline(std::cin, publisher);
        
        std::cout << "Enter publication year: ";
        std::cin >> year;
        std::cin.ignore(); // Clear newline
        
        std::cout << "Enter ISBN: ";
        std::getline(std::cin, ISBN);
        
        Book newBook(title, author, publisher, year, ISBN);
        addBook(newBook);
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } else if (choice == "4") { // Update a book
        std::string ISBN;
        std::cout << "Enter ISBN of the book to update: ";
        std::getline(std::cin, ISBN);
        
        Book* book = findBook(ISBN);
        if (book) {
            std::string title, author, publisher;
            int year;
            
            std::cout << "Enter new title (or press Enter to keep current): ";
            std::getline(std::cin, title);
            if (!title.empty()) book->setTitle(title);
            
            std::cout << "Enter new author (or press Enter to keep current): ";
            std::getline(std::cin, author);
            if (!author.empty()) book->setAuthor(author);
            
            std::cout << "Enter new publisher (or press Enter to keep current): ";
            std::getline(std::cin, publisher);
            if (!publisher.empty()) book->setPublisher(publisher);
            
            std::cout << "Enter new publication year (or 0 to keep current): ";
            std::cin >> year;
            std::cin.ignore(); // Clear newline
            if (year != 0) book->setYear(year);
            
            updateBook(*book);
        } else {
            std::cout << "Book not found.\n";
        }
        
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } else if (choice == "5") { // Remove a book
        std::string ISBN;
        std::cout << "Enter ISBN of the book to remove: ";
        std::getline(std::cin, ISBN);
        
        removeBook(ISBN);
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } else if (choice == "6") { // Back to main menu
    } else {
        std::cout << "Invalid choice. Please try again.\n";
    }
}

void Library::processLibrarianUsersMenuChoice(const std::string& choice) {
    if (choice == "1") { // Display all users
        clearScreen();
        displayHeader();
        std::cout << "\nALL USERS:\n";
        displayAllUsers();
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } 
    else if (choice == "2") { // Add a new user
        int id, userType;
        std::string name, email, password;
        
        std::cout << "Enter user ID: ";
        std::cin >> id;
        std::cin.ignore(); // Clear newline
        
        std::cout << "Enter name: ";
        std::getline(std::cin, name);
        
        std::cout << "Enter email: ";
        std::getline(std::cin, email);
        
        std::cout << "Enter password: ";
        std::getline(std::cin, password);

        std::cout << "Select user type:\n";
        std::cout << "1. Student\n";
        std::cout << "2. Faculty\n";
        std::cout << "3. Librarian\n";
        std::cout << "Enter choice: ";
        std::cin >> userType;
        std::cin.ignore(); // Clear newline
        
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
                std::cout << "Invalid user type.\n";
                break;
        }
        
        if (newUser) {
            addUser(newUser);
        }
        
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } else if (choice == "3") { // Remove a user
        int userId;
        std::cout << "Enter ID of the user to remove: ";
        std::cin >> userId;
        std::cin.ignore(); // Clear newline
        
        removeUser(userId);
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } else if (choice == "4") { // Back to main menu
    } else {
        std::cout << "Invalid choice. Please try again.\n";
    }
}

void Library::processLibrarianReportsMenuChoice(const std::string& choice) {
    if (choice == "1") { // Overdue books report
        clearScreen();
        displayHeader();
        std::cout << "\nOVERDUE BOOKS REPORT:\n";
        checkOverdueBooks();
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } else if (choice == "2") { // User fines report
        clearScreen();
        displayHeader();
        std::cout << "\nUSER FINES REPORT:\n";
        
        bool foundFines = false;
        for (const auto& pair : accounts) {
            if (pair.second.getFines() > 0) {
                User* user = findUser(pair.first);
                if (user) {
                    std::cout << "User: " << user->getName() << " (ID: " << user->getId() 
                              << ") - Fine: Rs." << pair.second.getFines() << std::endl;
                    foundFines = true;
                }
            }
        }
        
        if (!foundFines) {
            std::cout << "No outstanding fines found.\n";
        }
        
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } else if (choice == "3") { // Most borrowed books
        // This would require additional tracking in the Book class
        clearScreen();
        displayHeader();
        std::cout << "\nMOST BORROWED BOOKS REPORT:\n";
        std::cout << "This feature is not yet implemented.\n";
        std::cout << "\nPress Enter to continue...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } else if (choice == "4") { // Back to main menu
        // submenu = "main";
    } else {
        std::cout << "Invalid choice. Please try again.\n";
    }
}