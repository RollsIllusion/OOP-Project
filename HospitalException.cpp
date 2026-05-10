// ============================================================
// HospitalException.cpp
// Implements the base exception class for all MediCore errors.
// Stores a fixed-size message; no std::string used.
// ============================================================

#include "HospitalException.h"

// ------------------------------------------------------------
// Default constructor
// Clears the message buffer by writing a null terminator at [0].
// ------------------------------------------------------------
HospitalException::HospitalException() {
    message[0] = '\0'; // empty message
}

// ------------------------------------------------------------
// Parameterized constructor
// Manually copies msg into message[] up to 199 characters so
// the buffer is never overrun (no strcpy / strncpy used).
// ------------------------------------------------------------
HospitalException::HospitalException(const char* msg) {
    int i = 0;
    // Copy characters one by one until null-terminator or limit
    while (msg[i] != '\0' && i < 199) {
        message[i] = msg[i];
        i++;
    }
    message[i] = '\0'; // always null-terminate
}

// ------------------------------------------------------------
// Virtual destructor — nothing to free (no heap allocation here)
// ------------------------------------------------------------
HospitalException::~HospitalException() {}

// ------------------------------------------------------------
// what()
// Returns a pointer to the internal message buffer.
// Marked const because it does not modify the object.
// ------------------------------------------------------------
const char* HospitalException::what() const {
    return message;
}