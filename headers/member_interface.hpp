#ifndef MEMBER_INTERFACE_HPP
#define MEMBER_INTERFACE_HPP

#include "base_interface.h"
#include "cust_table.h"
#include "user_table.h"
#include <iostream>
#include <iomanip>
#include <string>

class member_interface : public BaseInterface {
private:
    UserTable user_table;
    std::string phone;

    // Override display header for member interface
    void display_header() override {
        std::cout << "\n+==================================================+\n";
        std::cout << "|                   GYM MEMBER PORTAL               |\n";
        std::cout << "+==================================================+\n\n";
    }

    void display_member_info() {
        auto cust_opt = customer_table.get_customer_by_phone(phone);
        if (cust_opt) {
            const auto& cust = *cust_opt;
            std::cout << "\n+==================================================+\n";
            std::cout << "|                 MEMBER INFORMATION                 |\n";
            std::cout << "+==================================================+\n\n";

            std::cout << "  Name: " << cust.name << "\n";
            std::cout << "  Membership Status: " << cust.status << "\n";
            std::cout << "  Expiry Date: " << cust.expiry_date << "\n";
            std::cout << "  Sessions Purchased: " << cust.sessions_purchased << "\n";
            std::cout << "  Sessions Used: " << cust.sessions_used << "\n";
            std::cout << "  Sessions Remaining: " << (cust.sessions_purchased - cust.sessions_used) << "\n\n";

            std::cout << "+==================================================+\n";
            std::cout << "|                  SESSION STATUS                   |\n";
            std::cout << "+==================================================+\n\n";

            int total_width = 50;
            int used_width = (cust.sessions_purchased > 0) ? (cust.sessions_used * total_width) / cust.sessions_purchased : 0;
            int remaining_width = total_width - used_width;

            std::cout << "  Progress: [" << std::string(used_width, '#') << std::string(remaining_width, '-')
                      << "] " << (cust.sessions_purchased > 0 ? (cust.sessions_used * 100.0 / cust.sessions_purchased) : 0.0) << "% ("
                      << cust.sessions_used << "/" << cust.sessions_purchased << ")\n\n";

            if (cust.sessions_purchased - cust.sessions_used <= 5) {
                std::cout << "  ! Warning: You have " << (cust.sessions_purchased - cust.sessions_used)
                          << " sessions remaining. Consider purchasing more sessions.\n\n";
            }

            if (cust.status != "Active") {
                std::cout << "  ! Your membership is currently " << cust.status << ". Please contact the gym to reactivate.\n\n";
            }
        } else {
            std::cout << "\n* No membership information found for your account.\n";
            std::cout << "Please contact the gym administration to link your account.\n\n";
        }
    }

    void add_session() {
        auto cust_opt = customer_table.get_customer_by_phone(phone);
        if (cust_opt) {
            customer cust = *cust_opt;
            if (cust.status != "Active") {
                std::cout << "\n* Cannot add session. Your membership is currently " << cust.status << ".\n";
                std::cout << "  Please contact the gym to reactivate your membership.\n\n";
                return;
            }

            if (cust.sessions_used >= cust.sessions_purchased) {
                std::cout << "\n* Cannot add session. You have used all your purchased sessions.\n";
                std::cout << "  Please purchase more sessions to continue.\n\n";
                return;
            }

            cust.sessions_used++;
            if (customer_table.update_customer(cust)) {
                std::cout << "\n* Session added successfully!\n";
                std::cout << "  Sessions remaining: " << (cust.sessions_purchased - cust.sessions_used) << "\n\n";
            } else {
                std::cout << "\n* Failed to update session usage in the database.\n";
            }
        } else {
            std::cout << "\n* Error: Could not find your membership information.\n";
            std::cout << "  Please contact the gym administration.\n\n";
        }
    }

public:
    member_interface(cust_table& table, const std::string& phone)
        : BaseInterface(table), phone(phone) {}

    // Implementation of the pure virtual function
    bool show_interface() override {
        while (true) {
            display_header();
            display_member_info();

            std::cout << "\nOptions:\n";
            std::cout << "1. Refresh Information\n";
            std::cout << "2. Add Session\n";
            std::cout << "3. Logout\n";
            std::cout << "Enter your choice (1-3): ";

            int choice;
            std::cin >> choice;

            switch (choice) {
                case 1:
                    // Refresh is automatic on next loop
                    break;
                case 2:
                    add_session();
                    break;
                case 3:
                    std::cout << "\nLogging out...\n";
                    return true;  // Return true to indicate logout
                default:
                    std::cout << "\nInvalid choice. Please try again.\n";
            }
        }
    }
};

#endif