#pragma once
#ifndef DOCTOR_H
#define DOCTOR_H

// ============================================================
// Doctor.h
// Represents a hospital doctor registered in the MediCore system.
// Inherits from Person (id, name, contact, password).
// Adds specialization and consultation fee.
//
// Operator overloads required by the project spec:
//   == : compare two Doctor objects by their ID
//   << : formatted console output  (friend function)
// ============================================================

#include "Person.h"
#include <ostream>  // needed only for operator<<

class Doctor : public Person {
private:
    char  specialization[50]; // e.g. "Cardiology", "Neurology"
    float fee;                // consultation fee in PKR

public:
    // ----------------------------------------------------------
    // Constructors / Destructor
    // ----------------------------------------------------------

    // Default constructor — clears all fields, fee = 0
    Doctor();

    // Parameterized constructor — fully initialises a Doctor object
    Doctor(int id, const char* name, const char* specialization,
        const char* contact, const char* password, float fee);

    // Destructor
    ~Doctor();

    // ----------------------------------------------------------
    // Getters
    // ----------------------------------------------------------
    const char* getSpecialization() const;
    float       getFee()            const;

    // ----------------------------------------------------------
    // Operator Overloads
    // ----------------------------------------------------------

    // == : returns true if both doctors share the same ID
    bool operator==(const Doctor& other) const;

    // << : prints formatted doctor info to an output stream
    friend std::ostream& operator<<(std::ostream& out, const Doctor& d);

    // ----------------------------------------------------------
    // Pure virtual implementations (required by Person)
    // ----------------------------------------------------------

    // Returns the string "Doctor"
    const char* getRole() const override;

    // Prints all doctor fields to the console
    void display() const override;

    // ----------------------------------------------------------
    // File I/O helper
    // Serialises the object into CSV format:
    // id,name,specialization,contact,password,fee
    // Written into the caller-supplied buffer.
    // ----------------------------------------------------------
    void toCSV(char* buffer, int maxLen) const;
};

#endif // DOCTOR_H