// ============================================================
// Doctor.cpp
// Implements the Doctor class — represents a hospital doctor.
// Inherits from Person; adds specialization and fee.
// ============================================================

#include "Doctor.h"
#include <iostream>

// ============================================================
// Constructors / Destructor
// ============================================================

Doctor::Doctor() : Person(), fee(0.0f) {
    specialization[0] = '\0';
}

Doctor::Doctor(int id, const char* name, const char* specialization,
    const char* contact, const char* password, float fee)
    : Person(id, name, contact, password), fee(fee)
{
    copyStr(this->specialization, specialization, 50);
}

Doctor::~Doctor() {}

// ============================================================
// Getters
// ============================================================

const char* Doctor::getSpecialization() const { return specialization; }
float       Doctor::getFee()            const { return fee; }

// ============================================================
// Operator Overloads
// ============================================================

// == : compare two doctors by ID
bool Doctor::operator==(const Doctor& other) const {
    return (id == other.id);
}

// << : formatted console output
// Prints: ID | Name | Specialization | Contact | Fee
std::ostream& operator<<(std::ostream& out, const Doctor& d) {
    out << "ID: " << d.id
        << " | Name: " << d.name
        << " | Specialization: " << d.specialization
        << " | Contact: " << d.contact
        << " | Fee: PKR " << d.fee;
    return out;
}

// ============================================================
// Pure virtual implementations
// ============================================================

const char* Doctor::getRole() const {
    return "Doctor";
}

void Doctor::display() const {
    std::cout << *this << std::endl;
}

// ============================================================
// toCSV
// Serialises into: id,name,specialization,contact,password,fee
// All number-to-string conversions are manual.
// ============================================================
void Doctor::toCSV(char* buffer, int maxLen) const {
    int pos = 0;

    auto writeInt = [&](int v) {
        char tmp[20]; int len = 0;
        if (v == 0) { buffer[pos++] = '0'; return; }
        while (v > 0 && len < 19) { tmp[len++] = (char)('0' + v % 10); v /= 10; }
        for (int k = len - 1; k >= 0 && pos < maxLen - 1; k--)
            buffer[pos++] = tmp[k];
        };

    auto writeFloat = [&](float v) {
        if (v < 0.0f) { buffer[pos++] = '-'; v = -v; }
        int intPart = (int)v;
        writeInt(intPart);
        buffer[pos++] = '.';
        int frac = (int)((v - (float)intPart) * 100.0f + 0.5f);
        buffer[pos++] = (char)('0' + frac / 10);
        buffer[pos++] = (char)('0' + frac % 10);
        };

    auto writeStr = [&](const char* s) {
        for (int i = 0; s[i] != '\0' && pos < maxLen - 1; i++)
            buffer[pos++] = s[i];
        };

    // id,name,specialization,contact,password,fee
    writeInt(id);               buffer[pos++] = ',';
    writeStr(name);             buffer[pos++] = ',';
    writeStr(specialization);   buffer[pos++] = ',';
    writeStr(contact);          buffer[pos++] = ',';
    writeStr(password);         buffer[pos++] = ',';
    writeFloat(fee);

    buffer[pos] = '\0';
}