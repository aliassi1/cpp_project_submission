#include <iostream>
#include "headers/base_interface.h"
#include "headers/interface.hpp"
#include "headers/member_interface.hpp"
#include "headers/user_table.h"
#include "headers/sha256.h"
#include <conio.h>  // For masking password input (Windows only)
#include <algorithm>
#include "headers/cust_table.h"
#include "headers/cust_table.hpp"
#include "headers/user.h"

// Function to get hidden input (password) from user
std::string get_hidden_input() {
    std::string input;
    char ch;
    while ((ch = _getch()) != '\r') {  // until Enter key
        if (ch == '\b') {
            if (!input.empty()) {
                std::cout << "\b \b";
                input.pop_back();
            }
        } else {
            input += ch;
            std::cout << '*';
        }
    }
    std::cout << std::endl;
    return input;
}

// Function that demonstrates polymorphism by using a base class pointer
void run_interface(BaseInterface* ui) {
    ui->show_interface();
}

int main() {
    try {
        // Initialize the customer table with SQLite database
        cust_table table("test.db");
        UserTable user_table;

        while (true) {
            try {
                std::string username, phone;

                std::cout << "\n+==================================================+\n";
                std::cout << "|                   GYM LOGIN PORTAL              |\n";
                std::cout << "+==================================================+\n\n";

                // Ask if user wants to log in as admin or member
                std::string login_type;
                std::cout << "Enter 'admin' to log in as admin or 'member' to log in as a member: ";
                std::cin >> login_type;

                // Base User pointer to demonstrate polymorphism
                User* user = nullptr;
                BaseInterface* ui = nullptr;

                if (login_type == "admin") {
                    std::string adminUsername = "admin"; // Default admin username
                    std::string admin_password = "adminpass";
                    std::string password;
                    
                    std::cout << "Enter admin password: ";
                    password = get_hidden_input();
                    
                    // Simple hash for example purposes
                    std::string password_hash = sha256(password);
                    std::string stored_hash = sha256(admin_password);

                    // Create Admin user object
                    Admin admin(adminUsername, stored_hash);
                    
                    if (admin.checkPassword(password_hash)) {
                        std::cout << "Admin login successful!\n";
                        user = new Admin(adminUsername, stored_hash);
                        ui = new interface(table);
                    } else {
                        std::cout << "❌ Invalid admin password.\n";
                    }
                } else if (login_type == "member") {
                    std::cout << "Enter phone number: ";
                    std::cin >> phone;
                    std::cout << "Enter password: ";
                    std::string password = get_hidden_input();

                    if (user_table.login(phone, password)) {
                        std::cout << "Member login successful!\n";
                        // Get username from phone (could retrieve from database)
                        username = "member_" + phone;
                        std::string password_hash = sha256(password);
                        
                        // Create Member user object
                        user = new Member(username, password_hash, phone);
                        ui = new member_interface(table, phone);
                    } else {
                        std::cout << "❌ Invalid member credentials.\n";
                    }
                } else {
                    std::cout << "Invalid login type. Please enter 'admin' or 'member'.\n";
                }

                // If login successful, run the appropriate interface and user
                if (ui && user) {
                    // We can use user type polymorphically here if needed
                    std::cout << "Logged in as: " << user->getUserType() << "\n";
                    
                    // Run the appropriate interface
                    run_interface(ui);
                    
                    // Clean up
                    delete ui;
                    delete user;
                    ui = nullptr;
                    user = nullptr;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error during login: " << e.what() << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Critical error: " << e.what() << std::endl;
        return 1;
    }
}