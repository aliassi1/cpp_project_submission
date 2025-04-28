#ifndef INTERFACE_HPP
#define INTERFACE_HPP

#include "interface.h"
#include <limits>
#include <iomanip>
#include <regex>
#include <stdexcept>
#include "sha256.h"
#include <optional>
#include <vector>
#include "../sqlite3.h"

// Input validation functions (unchanged)
bool is_valid_name(const std::string& name) {
    if (name.empty()) {
        std::cout << "Name cannot be empty.\n";
        return false;
    }
    std::regex name_pattern("^[a-zA-Z\\s\\-']+$");
    if (!std::regex_match(name, name_pattern)) {
        std::cout << "Name can only contain letters, spaces, hyphens, and apostrophes.\n";
        return false;
    }
    return true;
}
bool is_valid_phone(const std::string& phone) {
    if (phone.empty()) {
        std::cout << "Phone number cannot be empty.\n";
        return false;
    }
    std::regex phone_pattern("^[0-9\\s\\-()]+$");
    if (!std::regex_match(phone, phone_pattern)) {
        std::cout << "Phone number can only contain digits, spaces, hyphens, and parentheses.\n";
        return false;
    }
    return true;
}
bool is_valid_city(const std::string& city) {
    if (city.empty()) {
        std::cout << "City cannot be empty.\n";
        return false;
    }
    std::regex city_pattern("^[a-zA-Z\\s\\-]+$");
    if (!std::regex_match(city, city_pattern)) {
        std::cout << "City can only contain letters, spaces, and hyphens.\n";
        return false;
    }
    return true;
}
bool is_valid_date(const std::string& date) {
    if (date.empty()) {
        std::cout << "Date cannot be empty.\n";
        return false;
    }
    std::regex date_pattern("^\\d{4}-(?:0[1-9]|1[0-2])-(?:0[1-9]|[12]\\d|3[01])$");
    if (!std::regex_match(date, date_pattern)) {
        std::cout << "Date must be in YYYY-MM-DD format.\n";
        return false;
    }
    return true;
}
bool is_valid_status(const std::string& status) {
    if (status.empty()) {
        std::cout << "Status cannot be empty.\n";
        return false;
    }
    return (status == "Active" || status == "Inactive" || status == "Paused");
}

// Add a new customer
void interface::handle_add_cust() {
    try {
        std::string name, phone, city, expiry_date, status;
        int sessions_purchased;
        float total_paid;
        int last_id = customer_table.get_max_id();

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        do {
            std::cout << "Enter member full name: ";
            std::getline(std::cin, name);
        } while (!is_valid_name(name));
        do {
            std::cout << "Enter phone number: ";
            std::getline(std::cin, phone);
        } while (!is_valid_phone(phone));
        do {
            std::cout << "Enter city: ";
            std::getline(std::cin, city);
        } while (!is_valid_city(city));
        do {
            std::cout << "Enter membership expiry date (YYYY-MM-DD): ";
            std::getline(std::cin, expiry_date);
        } while (!is_valid_date(expiry_date));
        do {
            std::cout << "Enter number of sessions purchased: ";
            if (!(std::cin >> sessions_purchased) || sessions_purchased <= 0) {
                std::cout << "Sessions purchased must be a positive number.\n";
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            break;
        } while (true);
        std::cin.ignore();
        do {
            std::cout << "Enter membership status (Active/Inactive/Paused): ";
            std::getline(std::cin, status);
        } while (!is_valid_status(status));
        do {
            std::cout << "Enter total money paid: $";
            if (!(std::cin >> total_paid) || total_paid < 0) {
                std::cout << "Total paid must be a non-negative number.\n";
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            break;
        } while (true);

        customer new_cust(last_id + 1, name, phone, city, expiry_date, sessions_purchased, status, total_paid);
        if (customer_table.add_customer(new_cust)) {
            sqlite3* DB;
            sqlite3_open("test.db", &DB);
            std::string sql_insert = "INSERT INTO user_credentials (phone) VALUES (?);";
            sqlite3_stmt* stmt;
            sqlite3_prepare_v2(DB, sql_insert.c_str(), -1, &stmt, nullptr);
            sqlite3_bind_text(stmt, 1, phone.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
            sqlite3_close(DB);
            std::cout << "✅ Member added successfully. Please have them set a password during login.\n";
        } else {
            std::cout << "❌ Failed to add member to the database.\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error adding member: " << e.what() << std::endl;
    }
}

// Delete a customer
void interface::handle_delete_cust() {
    try {
        int delete_id;
        std::cout << "Enter customer ID to delete: ";
        if (!(std::cin >> delete_id) || delete_id <= 0) {
            std::cout << "❌ Invalid customer ID.\n";
            return;
        }
        auto cust_opt = customer_table.get_customer_by_id(delete_id);
        if (cust_opt) {
            const customer& c = *cust_opt;
            std::cout << "Found member:\n";
            std::cout << "Name: " << c.name << "\n";
            std::cout << "Phone: " << c.phone << "\n";
            std::cout << "City: " << c.city << "\n";
            std::cout << "Expiry: " << c.expiry_date << "\n";
            std::cout << "Sessions Purchased: " << c.sessions_purchased << "\n";
            std::cout << "Sessions Used: " << c.sessions_used << "\n";
            std::cout << "Status: " << c.status << "\n";
            std::cout << "Total Paid: $" << c.total_paid << "\n";
            std::string confirm;
            do {
                std::cout << "Are you sure you want to delete this member? (Y/N): ";
                std::cin >> confirm;
            } while (confirm != "Y" && confirm != "y" && confirm != "N" && confirm != "n");
            if (confirm == "Y" || confirm == "y") {
                if (customer_table.delete_customer_by_id(delete_id)) {
                    // No need to manually delete from user_credentials if ON DELETE CASCADE is set.
                    std::cout << "✅ Member deleted.\n";
                } else {
                    std::cout << "❌ Failed to delete member from the database.\n";
                }
            } else {
                std::cout << "Deletion cancelled.\n";
            }
        } else {
            std::cout << "❌ No member with ID " << delete_id << " found.\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error deleting member: " << e.what() << std::endl;
    }
}

// Update a customer
void interface::handle_update_cust() {
    try {
        int update_id;
        std::cout << "Enter customer ID to update: ";
        if (!(std::cin >> update_id) || update_id <= 0) {
            std::cout << "❌ Invalid customer ID.\n";
            return;
        }
        auto cust_opt = customer_table.get_customer_by_id(update_id);
        if (cust_opt) {
            customer c = *cust_opt;
            std::string name, phone, city, expiry_date, status;
            int sessions_purchased;
            double total_paid;
            std::string old_phone = c.phone;
            std::cout << "Found member ID: " << update_id << "\n";
            std::cout << "Current member information:\n";
            std::cout << "Name: " << c.name << "\n";
            std::cout << "Phone: " << c.phone << "\n";
            std::cout << "City: " << c.city << "\n";
            std::cout << "Expiry Date: " << c.expiry_date << "\n";
            std::cout << "Status: " << c.status << "\n";
            std::cout << "Sessions Purchased: " << c.sessions_purchased << "\n";
            std::cout << "Sessions Used: " << c.sessions_used << "\n";
            std::cout << "Total Paid: $" << c.total_paid << "\n\n";
            std::cout << "Select update option:\n";
            std::cout << "1 - Update Member Information (name, phone, city, expiry date, status)\n";
            std::cout << "2 - Update Sessions and Payment (sessions purchased, total paid)\n";
            std::cout << "3 - Cancel\n";
            std::cout << "Enter your choice (1-3): ";
            int choice;
            if (!(std::cin >> choice) || choice < 1 || choice > 3) {
                std::cout << "❌ Invalid choice. Update cancelled.\n";
                return;
            }
            std::cin.ignore();
            if (choice == 1) {
                do {
                    std::cout << "Enter new name (press Enter to keep current): ";
                    std::getline(std::cin, name);
                    if (name.empty()) break;
                } while (!is_valid_name(name));
                if (!name.empty()) c.name = name;
                do {
                    std::cout << "Enter new phone (press Enter to keep current): ";
                    std::getline(std::cin, phone);
                    if (phone.empty()) break;
                } while (!is_valid_phone(phone));
                if (!phone.empty()) c.phone = phone;
                do {
                    std::cout << "Enter new city (press Enter to keep current): ";
                    std::getline(std::cin, city);
                    if (city.empty()) break;
                } while (!is_valid_city(city));
                if (!city.empty()) c.city = city;
                do {
                    std::cout << "Enter new expiry date YYYY-MM-DD (press Enter to keep current): ";
                    std::getline(std::cin, expiry_date);
                    if (expiry_date.empty()) break;
                } while (!is_valid_date(expiry_date));
                if (!expiry_date.empty()) c.expiry_date = expiry_date;
                do {
                    std::cout << "Enter new status (Active/Inactive/Paused) (press Enter to keep current): ";
                    std::getline(std::cin, status);
                    if (status.empty()) break;
                } while (!is_valid_status(status));
                if (!status.empty()) c.status = status;
                if (customer_table.update_customer(c)) {
                    std::cout << "✅ Member information updated successfully.\n";
                } else {
                    std::cout << "❌ Failed to update member in the database.\n";
                }
            } else if (choice == 2) {
                std::cout << "\nUpdate Sessions and Payment:\n";
                std::cout << "Current sessions purchased: " << c.sessions_purchased << "\n";
                do {
                    std::cout << "Enter number of additional sessions to add: ";
                    if (!(std::cin >> sessions_purchased) || sessions_purchased <= 0) {
                        std::cout << "Sessions must be a positive number.\n";
                        std::cin.clear();
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        continue;
                    }
                    break;
                } while (true);
                c.sessions_purchased += sessions_purchased;
                std::cout << "New total sessions: " << c.sessions_purchased << "\n";
                do {
                    std::cout << "Enter payment amount for the new sessions: $";
                    if (!(std::cin >> total_paid) || total_paid < 0) {
                        std::cout << "Payment amount must be a non-negative number.\n";
                        std::cin.clear();
                        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                        continue;
                    }
                    break;
                } while (true);
                c.total_paid += total_paid;
                if (customer_table.update_customer(c)) {
                    std::cout << "✅ Sessions and payment updated successfully.\n";
                    std::cout << "New total paid: $" << c.total_paid << "\n";
                } else {
                    std::cout << "❌ Failed to update member in the database.\n";
                }
            } else if (choice == 3) {
                std::cout << "Update cancelled.\n";
                return;
            }
        } else {
            std::cout << "❌ No member with ID " << update_id << " found.\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error updating member: " << e.what() << std::endl;
    }
}

// ... rest of the file remains unchanged (show_options, show_interface, search_customer, show_member_view, use_session) ...



// Show admin options (unchanged)
void interface::show_options() {
    std::cout << "\n+===============================================================+" << std::endl;
    std::cout << "|                    Fitness CUSTOMER MANAGEMENT SYSTEM           |" << std::endl;
    std::cout << "+===============================================================+" << std::endl;
    std::cout << "|                                                               |" << std::endl;
    std::cout << "|  [1] > Add New Customer                                      |" << std::endl;
    std::cout << "|  [2] > Update Customer Information                           |" << std::endl;
    std::cout << "|  [3] > Display Customer Table                               |" << std::endl;
    std::cout << "|  [4] > Delete Customer                                       |" << std::endl;
    std::cout << "|  [5] > View Total Sales                                      |" << std::endl;
    std::cout << "|  [6] > Search Customer by Name                              |" << std::endl;
    std::cout << "|  [7] > Exit Program                                          |" << std::endl;
    std::cout << "|                                                               |" << std::endl;
    std::cout << "+===============================================================+" << std::endl;
    std::cout << "\nEnter your choice (1-7): ";
}

// Admin interface loop (update display logic to use DB)
bool interface::show_interface() {
    while (true) {
        display_header();
        show_options();
        
        int choice;
        std::cin >> choice;
        
        switch (choice) {
            case 1:
                handle_add_cust();
                break;
            case 2:
                handle_update_cust();
                break;
            case 3: {
                // Display customer table
                auto customers = customer_table.get_all_customers();
                std::cout << "\n+==================================================+\n";
                std::cout << "|                 CUSTOMER TABLE                   |\n";
                std::cout << "+==================================================+\n\n";
                std::cout << "| ID |       Name       |    Phone    |   Status   | Sessions Used/Total |\n";
                std::cout << "+----+------------------+-------------+------------+--------------------+\n";
                for (const auto& cust : customers) {
                    std::cout << "| " << std::setw(2) << cust.id << " | " 
                              << std::setw(16) << cust.name << " | " 
                              << std::setw(11) << cust.phone << " | " 
                              << std::setw(10) << cust.status << " | " 
                              << std::setw(6) << cust.sessions_used << "/" << std::setw(6) << cust.sessions_purchased << " |\n";
                }
                std::cout << "+----+------------------+-------------+------------+--------------------+\n";
                break;
            }
            case 4:
                handle_delete_cust();
                break;
            case 5: {
                // View total sales
                float total_sales = customer_table.get_total_paid();
                std::cout << "\n+==================================================+\n";
                std::cout << "|                 TOTAL SALES                     |\n";
                std::cout << "+==================================================+\n\n";
                std::cout << "Total sales: $" << std::fixed << std::setprecision(2) << total_sales << "\n";
                break;
            }
            case 6:
                search_customer();
                break;
            case 7:
                std::cout << "\nLogging out of admin interface...\n";
                return true; // Return true to indicate logout
            default:
                std::cout << "\nInvalid choice. Please try again.\n";
        }
    }
}

// Search member by name
void interface::search_customer() {
    std::string search_name;
    std::cout << "Enter member name to search: ";
    std::cin.ignore();
    std::getline(std::cin, search_name);

    auto results = customer_table.search_customers_by_name(search_name);
    if (!results.empty()) {
        for (const auto& cust : results) {
            std::cout << "[+] Customer Found!" << std::endl;
            std::cout << "-----------------------------------------------------------------" << std::endl;
            std::cout << "ID: " << cust.id << std::endl;
            std::cout << "Name: " << cust.name << std::endl;
            std::cout << "City: " << cust.city << std::endl;
            std::cout << "Status: " << cust.status << std::endl;
            std::cout << "Last Visit: " << cust.format_date() << std::endl;
            std::cout << "Total Paid: $" << cust.total_paid << std::endl;
            std::cout << "-----------------------------------------------------------------" << std::endl;
        }
    } else {
        std::cout << "[-] Customer with the name '" << search_name << "' not found." << std::endl;
    }
}

// Member-only view
void interface::show_member_view(const std::string& username) {
    std::cout << "\n==== Member Dashboard ====\n";
    auto results = customer_table.search_customers_by_name(username);
    if (!results.empty()) {
        const auto& m = results[0];
        std::cout << "Member ID: " << m.id << "\n"
                  << "Name: " << m.name << "\n"
                  << "Phone: " << m.phone << "\n"
                  << "City: " << m.city << "\n"
                  << "Expiry Date: " << m.expiry_date << "\n"
                  << "Sessions Purchased: " << m.sessions_purchased << "\n"
                  << "Sessions Used: " << m.sessions_used << "\n"
                  << "Status: " << m.status << "\n"
                  << "Total Paid: $" << m.total_paid << "\n";
    } else {
        std::cout << "❌ No data found for member: " << username << "\n";
    }
}

// Check-in: use session
void interface::use_session() {
    int id;
    std::cout << "Enter member ID to check in: ";
    std::cin >> id;
    auto cust_opt = customer_table.get_customer_by_id(id);
    if (cust_opt) {
        customer c = *cust_opt;
        if (c.sessions_used < c.sessions_purchased) {
            c.sessions_used++;
            if (customer_table.update_customer(c)) {
                std::cout << "✅ Check-in complete. "
                          << "Used " << c.sessions_used
                          << " / Purchased " << c.sessions_purchased << "\n";
            } else {
                std::cout << "❌ Failed to update sessions in the database.\n";
            }
        } else {
            std::cout << "❌ No sessions remaining. Member must purchase more sessions.\n";
        }
    } else {
        std::cout << "❌ Member ID not found.\n";
    }
}

#endif