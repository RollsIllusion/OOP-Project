// ============================================================
// Admin.cpp
// Implements the Admin class — the single system administrator.
// Inherits all identity fields from Person.
// ============================================================

#include "Admin.h"
#include <iostream>

// ============================================================
// Constructors / Destructor
// ============================================================

Admin::Admin() : Person() {}

Admin::Admin(int id, const char* name, const char* contact, const char* password)
    : Person(id, name, contact, password)
{
}

Admin::~Admin() {}

// ============================================================
// Pure virtual implementations
// ============================================================

const char* Admin::getRole() const {
    return "Admin";
}

void Admin::display() const {
    std::cout << *this << std::endl;
}

// ============================================================
// << operator — formatted output for Admin
// ============================================================
std::ostream& operator<<(std::ostream& out, const Admin& a) {
    out << "Admin ID: " << a.id << " | Name: " << a.name;
    return out;
}

// ============================================================
// toCSV
// Serialises into: id,name,password
// admin.txt format: admin_id,name,password
// ============================================================
void Admin::toCSV(char* buffer, int maxLen) const {
    int pos = 0;

    // Write id manually
    char tmp[20];
    int  len = 0;
    int  v = id;
    if (v == 0) {
        buffer[pos++] = '0';
    }
    else {
        while (v > 0 && len < 19) { tmp[len++] = (char)('0' + v % 10); v /= 10; }
        for (int k = len - 1; k >= 0 && pos < maxLen - 1; k--)
            buffer[pos++] = tmp[k];
    }
    buffer[pos++] = ',';

    // Write name
    for (int i = 0; name[i] != '\0' && pos < maxLen - 1; i++)
        buffer[pos++] = name[i];
    buffer[pos++] = ',';

    // Write password
    for (int i = 0; password[i] != '\0' && pos < maxLen - 1; i++)
        buffer[pos++] = password[i];

    buffer[pos] = '\0';
}