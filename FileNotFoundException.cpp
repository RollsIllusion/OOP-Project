// ============================================================
// FileNotFoundException.cpp
// Thrown when a required .txt data file cannot be opened.
// Builds a descriptive message and stores it in the inherited
// message[] buffer via the base-class constructor.
// ============================================================

#include "FileNotFoundException.h"

// ------------------------------------------------------------
// Constructor
// Accepts the name of the file that could not be opened and
// builds a human-readable message manually without sprintf.
// ------------------------------------------------------------
FileNotFoundException::FileNotFoundException(const char* filename)
    : HospitalException() // call base default first (clears buffer)
{
    // Manually build message: "File not found: <filename>"
    // We write character-by-character into the inherited message[].

    const char* prefix = "File not found: ";
    int i = 0;

    // Copy prefix
    while (prefix[i] != '\0' && i < 198) {
        message[i] = prefix[i];
        i++;
    }

    // Append filename
    int j = 0;
    while (filename[j] != '\0' && i < 198) {
        message[i] = filename[j];
        i++;
        j++;
    }

    message[i] = '\0'; // null-terminate the completed message
}

// ------------------------------------------------------------
// what() — returns the stored message (inherited buffer)
// ------------------------------------------------------------
const char* FileNotFoundException::what() const {
    return message;
}