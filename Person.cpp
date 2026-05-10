// ============================================================
// Person.cpp
// Implements the abstract base class Person.
// All string operations are manual — no strcpy/strcmp/strlen.
// ============================================================

#include "Person.h"

// ============================================================
// Static helper implementations
// These are shared by Patient, Doctor, and Admin through Person.
// ============================================================

// ------------------------------------------------------------
// copyStr
// Copies at most (maxLen - 1) characters from src into dest,
// then null-terminates.  Replacement for strcpy / strncpy.
// ------------------------------------------------------------
void Person::copyStr(char* dest, const char* src, int maxLen) {
    int i = 0;
    while (src[i] != '\0' && i < maxLen - 1) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

// ------------------------------------------------------------
// strLen
// Counts characters until the null terminator.
// Replacement for strlen.
// ------------------------------------------------------------
int Person::strLen(const char* s) {
    int len = 0;
    while (s[len] != '\0') {
        len++;
    }
    return len;
}

// ------------------------------------------------------------
// strCmp
// Compares a and b character-by-character.
// Returns  0 if equal, negative if a < b, positive if a > b.
// Replacement for strcmp.
// ------------------------------------------------------------
int Person::strCmp(const char* a, const char* b) {
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        if (a[i] != b[i]) {
            return (unsigned char)a[i] - (unsigned char)b[i];
        }
        i++;
    }
    // If one string ended before the other
    return (unsigned char)a[i] - (unsigned char)b[i];
}

// ============================================================
// Constructors / Destructor
// ============================================================

// ------------------------------------------------------------
// Default constructor — zeroes all fields
// ------------------------------------------------------------
Person::Person() : id(0) {
    name[0] = '\0';
    contact[0] = '\0';
    password[0] = '\0';
}

// ------------------------------------------------------------
// Parameterized constructor — copies all supplied fields
// ------------------------------------------------------------
Person::Person(int id, const char* name, const char* contact,
    const char* password)
    : id(id)
{
    copyStr(this->name, name, 100);
    copyStr(this->contact, contact, 15);
    copyStr(this->password, password, 50);
}

// ------------------------------------------------------------
// Virtual destructor — nothing dynamic to release in Person
// ------------------------------------------------------------
Person::~Person() {}

// ============================================================
// Getters
// ============================================================

int Person::getId() const {
    return id;
}

const char* Person::getName() const {
    return name;
}

const char* Person::getContact() const {
    return contact;
}

// ============================================================
// checkPassword
// Compares supplied pwd against stored password char-by-char.
// Returns true only if every character matches and both end
// at the same position (same length and same content).
// ============================================================
bool Person::checkPassword(const char* pwd) const {
    return (strCmp(password, pwd) == 0);
}