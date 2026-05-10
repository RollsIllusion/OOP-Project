// ============================================================
// Patient.cpp
// Implements the Patient class — represents a registered patient.
// Inherits identity fields from Person; adds age, gender, balance.
// All operator overloads and CSV serialisation implemented here.
// ============================================================

#include "Patient.h"
#include <iostream>  // for operator<< (cout)

// ============================================================
// Constructors / Destructor
// ============================================================

// ------------------------------------------------------------
// Default constructor
// Calls Person default constructor; sets numeric fields to 0.
// ------------------------------------------------------------
Patient::Patient() : Person(), age(0), gender('M'), balance(0.0f) {}

// ------------------------------------------------------------
// Parameterized constructor
// Initialises all Patient fields via Person constructor + member list.
// ------------------------------------------------------------
Patient::Patient(int id, const char* name, int age, char gender,
    const char* contact, const char* password, float balance)
    : Person(id, name, contact, password),
    age(age), gender(gender), balance(balance)
{
}

// ------------------------------------------------------------
// Destructor — no heap allocation in Patient, nothing to free
// ------------------------------------------------------------
Patient::~Patient() {}

// ============================================================
// Getters
// ============================================================

int   Patient::getAge()     const { return age; }
char  Patient::getGender()  const { return gender; }
float Patient::getBalance() const { return balance; }

// ============================================================
// Setters
// ============================================================

void Patient::setBalance(float newBalance) {
    balance = newBalance;
}

// ============================================================
// Operator Overloads
// ============================================================

// ------------------------------------------------------------
// += operator — adds amount to balance (top-up / refund)
// Returns reference to *this for chaining if needed.
// ------------------------------------------------------------
Patient& Patient::operator+=(float amount) {
    balance += amount;
    return *this;
}

// ------------------------------------------------------------
// -= operator — deducts amount from balance (fee / bill payment)
// ------------------------------------------------------------
Patient& Patient::operator-=(float amount) {
    balance -= amount;
    return *this;
}

// ------------------------------------------------------------
// == operator — compares two patients by their unique ID
// Used by Storage<Patient>::findById and conflict checks.
// ------------------------------------------------------------
bool Patient::operator==(const Patient& other) const {
    return (id == other.id);
}

// ------------------------------------------------------------
// << operator — formatted console output for Patient
// Prints: ID | Name | Age | Gender | Contact | Balance
// ------------------------------------------------------------
std::ostream& operator<<(std::ostream& out, const Patient& p) {
    out << "ID: " << p.id
        << " | Name: " << p.name
        << " | Age: " << p.age
        << " | Gender: " << p.gender
        << " | Contact: " << p.contact
        << " | Balance: PKR " << p.balance;
    return out;
}

// ============================================================
// Pure virtual implementations (required by Person)
// ============================================================

// Returns the role label for this user type
const char* Patient::getRole() const {
    return "Patient";
}

// Prints all patient fields to stdout
void Patient::display() const {
    std::cout << *this << std::endl;
}

// ============================================================
// toCSV
// Serialises the Patient into a comma-separated line matching
// the format in patients.txt:
//   id,name,age,gender,contact,password,balance
//
// All conversions are manual — no sprintf / snprintf used.
// The caller provides the destination buffer and its max size.
// ============================================================
void Patient::toCSV(char* buffer, int maxLen) const {
    // -------------------------------------------------------
    // Local helpers (lambdas not allowed pre-C++11 strictly,
    // so we use a simple nested approach with index tracking)
    // -------------------------------------------------------
    int pos = 0;  // current write position in buffer

    // --- Helper: write integer at pos ---
    auto writeInt = [&](int v) {
        char tmp[20];
        int  len = 0;
        if (v == 0) { buffer[pos++] = '0'; return; }
        bool neg = (v < 0);
        if (neg) { v = -v; }
        while (v > 0 && len < 19) { tmp[len++] = (char)('0' + v % 10); v /= 10; }
        if (neg) buffer[pos++] = '-';
        for (int k = len - 1; k >= 0 && pos < maxLen - 1; k--)
            buffer[pos++] = tmp[k];
        };

    // --- Helper: write float with 2 decimal places ---
    auto writeFloat = [&](float v) {
        if (v < 0.0f) { buffer[pos++] = '-'; v = -v; }
        int intPart = (int)v;
        writeInt(intPart);
        buffer[pos++] = '.';
        int frac = (int)((v - (float)intPart) * 100.0f + 0.5f);
        buffer[pos++] = (char)('0' + frac / 10);
        buffer[pos++] = (char)('0' + frac % 10);
        };

    // --- Helper: write a C-string ---
    auto writeStr = [&](const char* s) {
        for (int i = 0; s[i] != '\0' && pos < maxLen - 1; i++)
            buffer[pos++] = s[i];
        };

    // Build: id,name,age,gender,contact,password,balance
    writeInt(id);      buffer[pos++] = ',';
    writeStr(name);    buffer[pos++] = ',';
    writeInt(age);     buffer[pos++] = ',';
    buffer[pos++] = gender; buffer[pos++] = ',';
    writeStr(contact); buffer[pos++] = ',';
    writeStr(password); buffer[pos++] = ',';
    writeFloat(balance);

    buffer[pos] = '\0'; // null-terminate
}