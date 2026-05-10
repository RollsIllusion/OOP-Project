// ============================================================
// SlotUnavailableException.cpp
// Thrown when a patient tries to book an already-occupied slot.
// Builds the message manually — no sprintf / std::string used.
// ============================================================

#include "SlotUnavailableException.h"

// ------------------------------------------------------------
// Constructor
// Builds: "Time slot XX:XX is already taken. Please choose another."
// ------------------------------------------------------------
SlotUnavailableException::SlotUnavailableException(const char* timeSlot)
    : HospitalException()
{
    // Build message character-by-character
    const char* prefix = "Time slot ";
    const char* suffix = " is already taken. Please choose another.";

    int i = 0;

    // Write prefix
    int p = 0;
    while (prefix[p] != '\0' && i < 198) {
        message[i++] = prefix[p++];
    }

    // Write the time slot (e.g. "09:00")
    int s = 0;
    while (timeSlot[s] != '\0' && i < 198) {
        message[i++] = timeSlot[s++];
    }

    // Write suffix
    int q = 0;
    while (suffix[q] != '\0' && i < 198) {
        message[i++] = suffix[q++];
    }

    message[i] = '\0'; // null-terminate
}

// ------------------------------------------------------------
// what() — returns the stored message
// ------------------------------------------------------------
const char* SlotUnavailableException::what() const {
    return message;
}