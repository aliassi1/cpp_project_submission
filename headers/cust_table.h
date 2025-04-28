#ifndef __CUST_TABLE_H_INCLUDED__
#define __CUST_TABLE_H_INCLUDED__

#include <string>
#include <vector>
#include <optional>
#include "../sqlite3.h"
#include "customer.hpp"

class cust_table {
public:
    // Constructor and destructor
    cust_table(const std::string& db_file = "test.db");
    ~cust_table();

    // Initialize the SQLite database and table
    void init_database();

    // Add a new customer to the database
    bool add_customer(const customer& cust);

    // Update an existing customer in the database
    bool update_customer(const customer& cust);

    // Delete a customer by id
    bool delete_customer_by_id(int id);

    // Get a customer by id (returns nullopt if not found)
    std::optional<customer> get_customer_by_id(int id);

    // Search for customers by name (returns all matches)
    std::vector<customer> search_customers_by_name(const std::string& name);

    // Get all customers (optionally limit to n)
    std::vector<customer> get_all_customers(int n = -1);

    // Get the max customer ID in the database
    int get_max_id();

    // Sum all total_paid values in the database
    float get_total_paid();

    // (Optional) Get a customer by phone (if still needed elsewhere)
    std::optional<customer> get_customer_by_phone(const std::string& phone);

private:
    std::string db_name;
    sqlite3* db;
};

#endif // __CUST_TABLE_H_INCLUDED__