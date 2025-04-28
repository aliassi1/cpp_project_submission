#ifndef BASE_INTERFACE_H
#define BASE_INTERFACE_H

#include "cust_table.h"
#include <string>

// Base class for all interfaces
class BaseInterface {
protected:
    // Reference to customer table - all input actions will affect this table
    cust_table& customer_table;

    // Constructor - Initializes the interface with a reference to the customer table
    BaseInterface(cust_table& i_cust_table)
        : customer_table(i_cust_table)
    {}

    // Display header - virtual so derived classes can override
    virtual void display_header() {
        std::cout << "\n+==================================================+\n";
        std::cout << "|                   GYM MANAGEMENT                 |\n";
        std::cout << "+==================================================+\n\n";
    }

public:
    // Virtual destructor for proper cleanup
    virtual ~BaseInterface() {}

    // Pure virtual function to be implemented by derived classes
    virtual bool show_interface() = 0;
};

#endif // BASE_INTERFACE_H 