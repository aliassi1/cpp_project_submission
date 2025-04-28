#ifndef USER_TABLE_H
#define USER_TABLE_H

#include <iostream>
#include <string>
#include "sha256.h"
#include "../sqlite3.h"

class UserTable {
public:
    UserTable() {
        // Optionally, ensure tables exist here
    }

    // Add a new user to the database (no password at this stage)
    bool add_user(const std::string& username, const std::string& phone) {
        if (username == "admin") return false; // prevent admin registration

        sqlite3* DB;
        sqlite3_open("test.db", &DB);

        // Check if user already exists
        std::string check_sql = "SELECT COUNT(*) FROM users WHERE name = ?;";
        sqlite3_stmt* check_stmt;
        sqlite3_prepare_v2(DB, check_sql.c_str(), -1, &check_stmt, nullptr);
        sqlite3_bind_text(check_stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        int rc = sqlite3_step(check_stmt);
        int count = sqlite3_column_int(check_stmt, 0);
        sqlite3_finalize(check_stmt);

        if (rc != SQLITE_ROW || count > 0) {
            std::cout << "❌ User already exists with username: " << username << std::endl;
            sqlite3_close(DB);
            return false;
        }

        // Insert into users table
        std::string sql_insert = "INSERT INTO users (name, phone) VALUES (?, ?);";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(DB, sql_insert.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, phone.c_str(), -1, SQLITE_TRANSIENT);
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        // Insert phone into user_credentials if not already present
        std::string sql_insert_cred = "INSERT OR IGNORE INTO user_credentials (phone) VALUES (?);";
        sqlite3_stmt* cred_stmt;
        sqlite3_prepare_v2(DB, sql_insert_cred.c_str(), -1, &cred_stmt, nullptr);
        sqlite3_bind_text(cred_stmt, 1, phone.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(cred_stmt);
        sqlite3_finalize(cred_stmt);

        sqlite3_close(DB);
        return rc == SQLITE_DONE;
    }

    // Helper: check if a phone exists in users table
    bool user_exists(const std::string& phone) {
        sqlite3* DB;
        sqlite3_open("test.db", &DB);
        std::string sql = "SELECT COUNT(*) FROM users WHERE phone = ?;";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(DB, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, phone.c_str(), -1, SQLITE_TRANSIENT);
        int rc = sqlite3_step(stmt);
        int count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        sqlite3_close(DB);
        return rc == SQLITE_ROW && count > 0;
    }

    // Helper: check if a phone exists in user_credentials
    bool phone_exists_in_credentials(const std::string& phone) {
        sqlite3* DB;
        sqlite3_open("test.db", &DB);
        std::string sql = "SELECT COUNT(*) FROM user_credentials WHERE phone = ?;";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(DB, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, phone.c_str(), -1, SQLITE_TRANSIENT);
        int rc = sqlite3_step(stmt);
        int count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        sqlite3_close(DB);
        return rc == SQLITE_ROW && count > 0;
    }

    // Login and first-time password setup
    bool login(const std::string& phone, const std::string& password) {
        if (!user_exists(phone)) {
            std::cout << "❌ No matching user found.\n";
            return false;
        }

        sqlite3* DB;
        sqlite3_open("test.db", &DB);

        // Check if the phone exists in user_credentials and fetch hashed_password
        std::string sql_check = "SELECT hashed_password FROM user_credentials WHERE phone = ?;";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(DB, sql_check.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, phone.c_str(), -1, SQLITE_TRANSIENT);
        int rc = sqlite3_step(stmt);
        std::string stored_hashed_password;
        if (rc == SQLITE_ROW && sqlite3_column_text(stmt, 0)) {
            stored_hashed_password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        }
        sqlite3_finalize(stmt);

        // If phone does not exist in user_credentials, INSERT it and prompt for password
        if (!phone_exists_in_credentials(phone)) {
            std::cout << "No password set. Please set a new password: ";
            std::string new_password, confirm_password;
            std::cin >> new_password;
            std::cout << "Confirm your password: ";
            std::cin >> confirm_password;

            if (new_password != confirm_password) {
                std::cout << "❌ Passwords do not match. Please try again.\n";
                sqlite3_close(DB);
                return false;
            }

            std::string hashed_password = sha256(new_password);
            std::string sql_insert = "INSERT INTO user_credentials (phone, hashed_password) VALUES (?, ?);";
            sqlite3_stmt* insert_stmt;
            sqlite3_prepare_v2(DB, sql_insert.c_str(), -1, &insert_stmt, nullptr);
            sqlite3_bind_text(insert_stmt, 1, phone.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(insert_stmt, 2, hashed_password.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(insert_stmt);
            sqlite3_finalize(insert_stmt);

            std::cout << "✅ Password set successfully. You can now log in with your phone number and password.\n";
            sqlite3_close(DB);
            return true;
        }

        // If phone exists but no password is set (NULL or empty), prompt for password and UPDATE
        if (stored_hashed_password.empty()) {
            std::cout << "No password set. Please set a new password: ";
            std::string new_password, confirm_password;
            std::cin >> new_password;
            std::cout << "Confirm your password: ";
            std::cin >> confirm_password;

            if (new_password != confirm_password) {
                std::cout << "❌ Passwords do not match. Please try again.\n";
                sqlite3_close(DB);
                return false;
            }

            std::string hashed_password = sha256(new_password);
            std::string sql_update = "UPDATE user_credentials SET hashed_password = ? WHERE phone = ?;";
            sqlite3_stmt* update_stmt;
            sqlite3_prepare_v2(DB, sql_update.c_str(), -1, &update_stmt, nullptr);
            sqlite3_bind_text(update_stmt, 1, hashed_password.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(update_stmt, 2, phone.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(update_stmt);
            sqlite3_finalize(update_stmt);

            std::cout << "✅ Password set successfully. You can now log in with your phone number and password.\n";
            sqlite3_close(DB);
            return true;
        }

        // Otherwise, verify the entered password
        std::string hashed_input_password = sha256(password);
        if (hashed_input_password == stored_hashed_password) {
            std::cout << "✅ Login successful!\n";
            sqlite3_close(DB);
            return true;
        } else {
            std::cout << "❌ Incorrect password.\n";
            sqlite3_close(DB);
            return false;
        }
    }

    // Example: get phone by username
    std::string get_phone(const std::string& username) {
        sqlite3* DB;
        sqlite3_open("test.db", &DB);
        std::string sql = "SELECT phone FROM users WHERE name = ?;";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(DB, sql.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        int rc = sqlite3_step(stmt);
        std::string phone;
        if (rc == SQLITE_ROW && sqlite3_column_text(stmt, 0)) {
            phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        }
        sqlite3_finalize(stmt);
        sqlite3_close(DB);
        return phone;
    }
};

#endif