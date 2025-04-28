#ifndef USER_H
#define USER_H

#include <string>

// Base User class
class User {
protected:
    std::string username;
    std::string password_hash; // Storing hash for security

public:
    User(const std::string& username, const std::string& password_hash)
        : username(username), password_hash(password_hash) {}
    
    virtual ~User() {}
    
    // Get username
    std::string getUsername() const { return username; }
    
    // Check password
    bool checkPassword(const std::string& password_hash) const {
        return this->password_hash == password_hash;
    }
    
    // Pure virtual function to determine user type
    virtual std::string getUserType() const = 0;
};

// Admin class
class Admin : public User {
public:
    Admin(const std::string& username, const std::string& password_hash)
        : User(username, password_hash) {}
    
    std::string getUserType() const override {
        return "admin";
    }
};

// Member class
class Member : public User {
private:
    std::string phone;

public:
    Member(const std::string& username, const std::string& password_hash, const std::string& phone)
        : User(username, password_hash), phone(phone) {}
    
    std::string getUserType() const override {
        return "member";
    }
    
    std::string getPhone() const {
        return phone;
    }
};

#endif