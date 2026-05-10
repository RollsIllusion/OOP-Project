#pragma once
#ifndef HOSPITALEXCEPTION_H
#define HOSPITALEXCEPTION_H

// ============================================================
// HospitalException.h
// Base class for all custom exceptions in the MediCore system.
// Inherits from std::exception and stores a message in a
// fixed-size char array (no std::string allowed).
// ============================================================

class HospitalException {
protected:
    char message[200]; // stores the error/exception message

public:
    // Default constructor — clears the message buffer
    HospitalException();

    // Parameterized constructor — copies msg into message[]
    HospitalException(const char* msg);

    // Virtual destructor so derived exception classes clean up properly
    virtual ~HospitalException();

    // Returns the stored message; virtual so derived classes can override
    virtual const char* what() const;
};

#endif // HOSPITALEXCEPTION_H