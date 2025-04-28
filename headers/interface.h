#ifndef INTERFACE_H
#define INTERFACE_H

#include <iostream>
#include "cust_table.h"
#include "base_interface.h"

class interface : public BaseInterface {
private:
    // Override display header for admin interface
    void display_header() override {
        std::cout << "\n+==================================================+\n";
        std::cout << "|                   ADMIN INTERFACE                |\n";
        std::cout << "+==================================================+\n\n";
    }

public:
    // Constructor - now calls base class constructor
    interface(cust_table& i_cust_table)
        : BaseInterface(i_cust_table)
    {}

    // Implementation of the pure virtual function
    bool show_interface() override;

    // Shows available options for the user (admin)
    void show_options();

    // Handles adding a new customer to the table
    void handle_add_cust();

    // Handles updating an existing customer's information
    void handle_update_cust();

    // Handles deleting a customer from the table
    void handle_delete_cust();

    // Searches for a customer based on user input
    void search_customer();

    // Displays the member-specific view (e.g., profile or session details)
    void show_member_view(const std::string& username);  

    // Handles member session usage (e.g., reduce sessions_used)
    void use_session();
};

#endif
