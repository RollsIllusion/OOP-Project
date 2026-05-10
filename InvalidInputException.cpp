// ============================================================
// InvalidInputException.cpp
// Thrown when user-supplied input fails a Validator check.
// Stores the reason string in the inherited message[] buffer.
// ============================================================

#include "InvalidInputException.h"

// ------------------------------------------------------------
// Constructor
// Accepts a descriptive reason and stores it in message[]
// by delegating to the base-class parameterized constructor.
// ------------------------------------------------------------
InvalidInputException::InvalidInputException(const char* reason)
    : HospitalException(reason)  // base constructor copies reason → message[]
{
    // Nothing additional needed; base constructor handles the copy.
}

// ------------------------------------------------------------
// what() — returns the stored validation error message
// ------------------------------------------------------------
const char* InvalidInputException::what() const {
    return message;
}