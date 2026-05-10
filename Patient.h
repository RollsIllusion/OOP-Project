#pragma once
#ifndef PATIENT_H
#define PATIENT_H

// ============================================================
// Patient.h
// Represents a registered patient in the MediCore system.
// Inherits from Person (id, name, contact, password).
// Adds age, gender, and balance fields.
//
// Operator overloads required by the project spec:
//   += : add an amount to balance  (e.g. refund / top-up)
//   -= : deduct an amount from balance  (e.g. booking fee)
//   == : compare two Patient objects by their ID
//   << : formatted console output  (friend function)
// ============================================================

#include "Person.h"
#include <ostream>  // needed only for operator<<

class Patient : public Person {
private:
    int   age;        // patient's age in years
    char  gender;     // 'M' or 'F'
    float balance;    // current wallet balance in PKR

public:
    // ----------------------------------------------------------
    // Constructors / Destructor
    // ----------------------------------------------------------

    // Default constructor — sets numeric fields to 0, gender to 'M'
    Patient();

    // Parameterized constructor — fully initialises a Patient object
    Patient(int id, const char* name, int age, char gender,
        const char* contact, const char* password, float balance);

    // Destructor (Person destructor is virtual, so this is called correctly)
    ~Patient();

    // ----------------------------------------------------------
    // Getters
    // ----------------------------------------------------------
    int   getAge()     const;
    char  getGender()  const;
    float getBalance() const;

    // ----------------------------------------------------------
    // Setters (used by FileHandler when updating patients.txt)
    // ----------------------------------------------------------
    void setBalance(float newBalance);

    // ----------------------------------------------------------
    // Operator Overloads
    // ----------------------------------------------------------

    // += : adds 'amount' to balance (refund / top-up)
    Patient& operator+=(float amount);

    // -= : deducts 'amount' from balance (fee payment)
    Patient& operator-=(float amount);

    // == : returns true if both patients share the same ID
    bool operator==(const Patient& other) const;

    // << : prints a formatted patient summary to an output stream
    friend std::ostream& operator<<(std::ostream& out, const Patient& p);

    // ----------------------------------------------------------
    // Pure virtual implementations (required by Person)
    // ----------------------------------------------------------

    // Returns the string "Patient"
    const char* getRole() const override;

    // Prints all patient fields to the console
    void display() const override;

    // ----------------------------------------------------------
    // File I/O helper
    // Serialises the object into CSV format: id,name,age,gender,
    // contact,password,balance  — written into the supplied buffer.
    // The caller provides the buffer and its max size.
    // ----------------------------------------------------------
    void toCSV(char* buffer, int maxLen) const;
};

#endif // PATIENT_H