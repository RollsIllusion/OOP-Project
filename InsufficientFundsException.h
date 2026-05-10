#pragma once
#ifndef INSUFFICIENTFUNDSEXCEPTION_H
#define INSUFFICIENTFUNDSEXCEPTION_H

// ============================================================
// InsufficientFundsException.h
// Thrown when a patient's current balance is less than the
// required amount (e.g. doctor fee or bill amount).
// Inherits from HospitalException.
// ============================================================

#include "HospitalException.h"

class InsufficientFundsException : public HospitalException {
public:
    // Constructor accepts the required amount and current balance
    // so a detailed message can be built and stored in message[]
    InsufficientFundsException(float required, float available);

    // Overrides what() to return the insufficient-funds message
    const char* what() const override;
};

#endif // INSUFFICIENTFUNDSEXCEPTION_H