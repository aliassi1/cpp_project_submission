#ifndef CUSTOMER_H
#define CUSTOMER_H

#include <string>
#include <ctime>

class customer {
public:
    int id;
    std::string name;
    std::string phone;
    std::string city;
    std::string expiry_date;
    int sessions_purchased;
    int sessions_used;
    std::string status;
    double total_paid;

    customer() = default;

    // Constructor that sets sessions_used to 0 by default
    customer(int i_id, std::string i_name, std::string i_phone, std::string i_city,
             std::string i_expiry, int i_sessions_purchased, std::string i_status, double i_total_paid, int i_sessions_used = 0)
        : id(i_id), name(i_name), phone(i_phone), city(i_city),
          expiry_date(i_expiry), sessions_purchased(i_sessions_purchased),
          sessions_used(i_sessions_used), status(i_status), total_paid(i_total_paid) {}

    std::string format_date() const {
        return expiry_date;
    }
};

#endif
