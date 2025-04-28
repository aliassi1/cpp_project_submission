#include "cust_table.h"
#include <iostream>
#include <iomanip>

// Constructor
cust_table::cust_table(const std::string& db_file)
    : db_name(db_file), db(nullptr)
{
    init_database();
}

// Destructor
cust_table::~cust_table() {
    if (db) {
        sqlite3_close(db);
    }
}

// Initialize the SQLite database and users table
void cust_table::init_database() {
    int rc = sqlite3_open(db_name.c_str(), &db);
    if (rc) {
        std::cerr << "❌ Error opening database: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
        return;
    }
    const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                      "id INTEGER PRIMARY KEY, "
                      "name TEXT, "
                      "phone TEXT, "
                      "city TEXT, "
                      "expiry_date TEXT, "
                      "sessions_purchased INTEGER, "
                      "sessions_used INTEGER, "
                      "status TEXT, "
                      "total_paid REAL);";
    char* errMsg = nullptr;
    rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

// Add a new customer
bool cust_table::add_customer(const customer& cust) {
    const char* sql = "INSERT INTO users "
                      "(id, name, phone, city, expiry_date, sessions_purchased, sessions_used, status, total_paid) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, cust.id);
    sqlite3_bind_text(stmt, 2, cust.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, cust.phone.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, cust.city.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, cust.expiry_date.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 6, cust.sessions_purchased);
    sqlite3_bind_int(stmt, 7, cust.sessions_used);
    sqlite3_bind_text(stmt, 8, cust.status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 9, cust.total_paid);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

// Update an existing customer
bool cust_table::update_customer(const customer& cust) {
    // Fetch the old phone before updating
    std::string old_phone;
    {
        const char* sql_get = "SELECT phone FROM users WHERE id=?;";
        sqlite3_stmt* stmt_get;
        if (sqlite3_prepare_v2(db, sql_get, -1, &stmt_get, nullptr) == SQLITE_OK) {
            sqlite3_bind_int(stmt_get, 1, cust.id);
            if (sqlite3_step(stmt_get) == SQLITE_ROW && sqlite3_column_text(stmt_get, 0)) {
                old_phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt_get, 0));
            }
            sqlite3_finalize(stmt_get);
        }
    }

    // Now update the user as before
    const char* sql = "UPDATE users SET name=?, phone=?, city=?, expiry_date=?, "
                      "sessions_purchased=?, sessions_used=?, status=?, total_paid=? WHERE id=?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, cust.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, cust.phone.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, cust.city.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, cust.expiry_date.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, cust.sessions_purchased);
    sqlite3_bind_int(stmt, 6, cust.sessions_used);
    sqlite3_bind_text(stmt, 7, cust.status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 8, cust.total_paid);
    sqlite3_bind_int(stmt, 9, cust.id);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);

    // If the phone changed, update it in user_credentials as well
    if (success && !old_phone.empty() && old_phone != cust.phone) {
        sqlite3* DB;
        sqlite3_open(db_name.c_str(), &DB);
        std::string sql_update = "UPDATE user_credentials SET phone = ? WHERE phone = ?;";
        sqlite3_stmt* stmt_update;
        sqlite3_prepare_v2(DB, sql_update.c_str(), -1, &stmt_update, nullptr);
        sqlite3_bind_text(stmt_update, 1, cust.phone.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt_update, 2, old_phone.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt_update);
        sqlite3_finalize(stmt_update);
        sqlite3_close(DB);
    }

    return success;
}

// Delete a customer by id
bool cust_table::delete_customer_by_id(int id) {
    // Fetch phone number for the customer
    std::string phone;
    const char* sql_get = "SELECT phone FROM users WHERE id=?;";
    sqlite3_stmt* stmt_get;
    if (sqlite3_prepare_v2(db, sql_get, -1, &stmt_get, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt_get, 1, id);
        if (sqlite3_step(stmt_get) == SQLITE_ROW && sqlite3_column_text(stmt_get, 0)) {
            phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt_get, 0));
        }
        sqlite3_finalize(stmt_get);
    }
    // Delete from user_credentials if phone is found
    if (!phone.empty()) {
        sqlite3* DB;
        sqlite3_open(db_name.c_str(), &DB);
        std::string sql_delete = "DELETE FROM user_credentials WHERE phone = ?;";
        sqlite3_stmt* stmt_delete;
        sqlite3_prepare_v2(DB, sql_delete.c_str(), -1, &stmt_delete, nullptr);
        sqlite3_bind_text(stmt_delete, 1, phone.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt_delete);
        sqlite3_finalize(stmt_delete);
        sqlite3_close(DB);
    }
    // Delete from users
    const char* sql = "DELETE FROM users WHERE id=?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, id);
    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

// Get a customer by id
std::optional<customer> cust_table::get_customer_by_id(int id) {
    const char* sql = "SELECT * FROM users WHERE id=?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    sqlite3_bind_int(stmt, 1, id);
    customer cust;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        cust.id = sqlite3_column_int(stmt, 0);
        cust.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        cust.phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        cust.city = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        cust.expiry_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        cust.sessions_purchased = sqlite3_column_int(stmt, 5);
        cust.sessions_used = sqlite3_column_int(stmt, 6);
        cust.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        cust.total_paid = static_cast<float>(sqlite3_column_double(stmt, 8));
        sqlite3_finalize(stmt);
        return cust;
    }
    sqlite3_finalize(stmt);
    return std::nullopt;
}

// Search for customers by name (exact match)
std::vector<customer> cust_table::search_customers_by_name(const std::string& search_term) {
    // First, retrieve all customers or a broader subset from the database
    std::vector<customer> all_customers;
    const char* sql = "SELECT * FROM users;";  // Get all users or use a broader filter
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) 
        return all_customers;
    
    // Populate the vector with customers from the database
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        customer cust;
        cust.id = sqlite3_column_int(stmt, 0);
        cust.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        cust.phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        cust.city = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        cust.expiry_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        cust.sessions_purchased = sqlite3_column_int(stmt, 5);
        cust.sessions_used = sqlite3_column_int(stmt, 6);
        cust.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        cust.total_paid = static_cast<float>(sqlite3_column_double(stmt, 8));
        all_customers.push_back(cust);
    }
    sqlite3_finalize(stmt);
    
    // Now use STL algorithms to search through the retrieved data
    std::vector<customer> results;
    
    // Using std::copy_if with a lambda predicate for partial name matching
    std::copy_if(all_customers.begin(), all_customers.end(), 
                 std::back_inserter(results),
                 [&search_term](const customer& cust) {
                     // Case-insensitive partial name matching
                     std::string name_lower = cust.name;
                     std::string search_lower = search_term;
                     
                     // Convert both strings to lowercase for case-insensitive comparison
                     std::transform(name_lower.begin(), name_lower.end(), 
                                   name_lower.begin(), ::tolower);
                     std::transform(search_lower.begin(), search_lower.end(), 
                                   search_lower.begin(), ::tolower);
                     
                     // Check if the search term is contained in the name
                     return name_lower.find(search_lower) != std::string::npos;
                 });
    
    // Sort results by name using STL sort algorithm
    std::sort(results.begin(), results.end(),
              [](const customer& a, const customer& b) {
                  return a.name < b.name;
              });
    
    return results;
}

// Get all customers (optionally limit to n)
std::vector<customer> cust_table::get_all_customers(int n) {
    std::vector<customer> results;
    std::string sql = "SELECT * FROM users";
    if (n > 0) {
        sql += " LIMIT " + std::to_string(n);
    }
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return results;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        customer cust;
        cust.id = sqlite3_column_int(stmt, 0);
        cust.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        cust.phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        cust.city = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        cust.expiry_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        cust.sessions_purchased = sqlite3_column_int(stmt, 5);
        cust.sessions_used = sqlite3_column_int(stmt, 6);
        cust.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        cust.total_paid = static_cast<float>(sqlite3_column_double(stmt, 8));
        results.push_back(cust);
    }
    sqlite3_finalize(stmt);
    return results;
}

// Get the max customer ID in the database
int cust_table::get_max_id() {
    int max_id = 0;
    const char* sql = "SELECT MAX(id) FROM users;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            max_id = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    return max_id;
}

// Sum all total_paid values in the database
float cust_table::get_total_paid() {
    float total = 0.0f;
    const char* sql = "SELECT SUM(total_paid) FROM users;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            total = static_cast<float>(sqlite3_column_double(stmt, 0));
        }
        sqlite3_finalize(stmt);
    }
    return total;
}

// (Optional) Get a customer by phone
std::optional<customer> cust_table::get_customer_by_phone(const std::string& phone) {
    const char* sql = "SELECT * FROM users WHERE phone=?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    sqlite3_bind_text(stmt, 1, phone.c_str(), -1, SQLITE_TRANSIENT);
    customer cust;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        cust.id = sqlite3_column_int(stmt, 0);
        cust.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        cust.phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        cust.city = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        cust.expiry_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        cust.sessions_purchased = sqlite3_column_int(stmt, 5);
        cust.sessions_used = sqlite3_column_int(stmt, 6);
        cust.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        cust.total_paid = static_cast<float>(sqlite3_column_double(stmt, 8));
        sqlite3_finalize(stmt);
        return cust;
    }
    sqlite3_finalize(stmt);
    return std::nullopt;
}