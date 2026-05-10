#pragma once
#ifndef SLOTUNAVAILABLEEXCEPTION_H
#define SLOTUNAVAILABLEEXCEPTION_H

// ============================================================
// SlotUnavailableException.h
// Thrown when a patient tries to book a time slot that is
// already occupied by another non-cancelled appointment for
// the same doctor on the same date.
// Inherits from HospitalException.
// ============================================================

#include "HospitalException.h"

class SlotUnavailableException : public HospitalException {
public:
    // Constructor accepts the requested time slot string (e.g. "09:00")
    // and builds a meaningful message stored in message[]
    SlotUnavailableException(const char* timeSlot);

    // Overrides what() to return the slot-unavailable message
    const char* what() const override;
};

#endif // SLOTUNAVAILABLEEXCEPTION_H