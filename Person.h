#pragma once
#ifndef PERSON_H
#define PERSON_H

// ============================================================
// Person.h
// Abstract base class for all user types in the MediCore system.
// Holds identity attributes common to Patient, Doctor, and Admin.
// Cannot be instantiated directly — contains pure virtual methods.
// No std::string used; all strings are plain char arrays.
// ============================================================

class Person {
protected:
    int    id;           // unique numeric identifier
    char   name[100];    // full name of the person
    char   contact[15];  // 11-digit contact number stored as char array
    char   password[50]; // login password

public:
    // ----------------------------------------------------------
    // Constructors / Destructor
    // ----------------------------------------------------------

    // Default constructor — zeroes out all fields
    Person();

    // Parameterized constructor — initialises all common fields
    Person(int id, const char* name, const char* contact, const char* password);

    // Virtual destructor — ensures proper cleanup in derived classes
    virtual ~Person();

    // ----------------------------------------------------------
    // Getters (const — they do not modify the object)
    // ----------------------------------------------------------
    int         getId()      const;
    const char* getName()    const;
    const char* getContact() const;

    // ----------------------------------------------------------
    // Password check — manually compares char-by-char
    // Returns true if the supplied password matches stored one
    // ----------------------------------------------------------
    bool checkPassword(const char* pwd) const;

    // ----------------------------------------------------------
    // Pure virtual methods — every derived class MUST implement
    // these, making Person truly abstract.
    // ----------------------------------------------------------

    // Returns a short role label, e.g. "Patient", "Doctor", "Admin"
    virtual const char* getRole() const = 0;

    // Displays the person's information to the console
    virtual void display() const = 0;

    // ----------------------------------------------------------
    // Utility helpers (non-virtual, shared logic)
    // ----------------------------------------------------------

    // Copies src into dest manually (replaces strcpy)
    static void copyStr(char* dest, const char* src, int maxLen);

    // Returns length of a char array manually (replaces strlen)
    static int  strLen(const char* s);

    // Compares two char arrays manually (replaces strcmp)
    // Returns 0 if equal, <0 or >0 otherwise
    static int  strCmp(const char* a, const char* b);
};

#endif // PERSON_H