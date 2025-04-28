#ifndef __table_H_INCLUDED__
#define __table_H_INCLUDED__

#include <iostream>
#include <map>

using namespace std;

// Create as a template so class can be expanded upon in future updates
template <typename K, typename V>
class table {
public:
    map<K, V> hashtable;  // Map to store data

    // Declare the function (without definition)
    void insert_row(K key, V value);
};

// Since we are using a template class, we include the table.hpp at the bottom
#include "table.hpp"

#endif
