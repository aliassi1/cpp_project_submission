#ifndef __table_HPP_INCLUDED__
#define __table_HPP_INCLUDED__

#include "table.h"

// Define the insert_row function here
template <typename K, typename V>
void table<K, V>::insert_row(K key, V value) {
    hashtable[key] = value;
}

#endif
