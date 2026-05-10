#pragma once
#ifndef ADMIN_H
#define ADMIN_H

// ============================================================
// Admin.h
// Represents the system administrator of MediCore.
// Inherits from Person (id, name, contact, password).
// Admin has no extra data fields beyond what Person provides,
// but implements Person's pure virtuals and exposes all
// admin-specific menu actions through dedicated methods.
// There is only ONE admin record (loaded from admin.txt).
// ============================================================

#include "Person.h"
#include <ostream>  // for operator<<

class Admin : public Person {
public:
    // ----------------------------------------------------------
    // Constructors / Destructor
    // ----------------------------------------------------------

    // Default constructor
    Admin();

    // Parameterized constructor
    // 'contact' is optional for admin but kept for consistency
    Admin(int id, const char* name, const char* contact, const char* password);

    // Destructor
    ~Admin();

    // ----------------------------------------------------------
    // Pure virtual implementations (required by Person)
    // ----------------------------------------------------------

    // Returns the string "Admin"
    const char* getRole() const override;

    // Prints admin info to the console
    void display() const override;

    // ----------------------------------------------------------
    // File I/O helper
    // Serialises into CSV: id,name,password
    // (admin.txt does not store contact — field is left blank)
    // ----------------------------------------------------------
    void toCSV(char* buffer, int maxLen) const;

    // << : formatted output
    friend std::ostream& operator<<(std::ostream& out, const Admin& a);
};

#endif // ADMIN_H