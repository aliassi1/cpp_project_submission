#include <iostream>
#include "sqlite3.h"

// Callback function to display results of SELECT queries
static int callback(void* NotUsed, int argc, char** argv, char** azColName) {
    for (int i = 0; i < argc; i++) {
        std::cout << azColName[i] << " = " << (argv[i] ? argv[i] : "NULL") << "\n";
    }
    return 0;
}

// Function to handle SQL errors consistently
void handle_sql_error(int exit_code, char* errorMessage, sqlite3* DB) {
    if (exit_code != SQLITE_OK) {
        std::cerr << "❌ SQL error: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
        sqlite3_close(DB);
        exit(-1);  // Ensure the program exits on error
    }
}

int main() {
    sqlite3* DB;
    char* errorMessage;

    // Open database connection
    int exit_code = sqlite3_open("test.db", &DB);
    if (exit_code) {
        std::cerr << "❌ Error opening DB: " << sqlite3_errmsg(DB) << std::endl;
        return -1;
    } else {
        std::cout << "✅ Database opened successfully!\n";
    }

    // Create the 'users' table if it doesn't exist
    const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                      "id INTEGER PRIMARY KEY, "
                      "name TEXT, "
                      "phone TEXT UNIQUE, "
                      "city TEXT, "
                      "expiry_date DATE, "
                      "sessions_purchased INTEGER, "
                      "sessions_used INTEGER, "
                      "status TEXT, "
                      "total_paid DECIMAL);";

    exit_code = sqlite3_exec(DB, sql, callback, nullptr, &errorMessage);
    handle_sql_error(exit_code, errorMessage, DB);
    std::cout << "✅ Users table created successfully\n";

    // Create 'user_credentials' table if it doesn't exist
    const char* sql_create_password_table = 
        "CREATE TABLE IF NOT EXISTS user_credentials ("
        "phone TEXT PRIMARY KEY, "
        "hashed_password TEXT, "
        "FOREIGN KEY (phone) REFERENCES users(phone) ON DELETE CASCADE);";

    exit_code = sqlite3_exec(DB, sql_create_password_table, callback, nullptr, &errorMessage);
    handle_sql_error(exit_code, errorMessage, DB);
    std::cout << "✅ User_credentials table created successfully\n";

    // Close database connection
    sqlite3_close(DB);
    return 0;
}
