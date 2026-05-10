#pragma once
#ifndef INVALIDINPUTEXCEPTION_H
#define INVALIDINPUTEXCEPTION_H

// ============================================================
// InvalidInputException.h
// Thrown when user-supplied input fails validation checks
// performed by the Validator class (e.g. bad date format,
// non-numeric contact, negative top-up amount, etc.).
// Inherits from HospitalException.
// ============================================================

#include "HospitalException.h"

class InvalidInputException : public HospitalException {
public:
    // Constructor accepts a descriptive reason string
    // and stores it in the inherited message[] buffer
    InvalidInputException(const char* reason);

    // Overrides what() to return the validation error message
    const char* what() const override;
};

#endif // INVALIDINPUTEXCEPTION_H