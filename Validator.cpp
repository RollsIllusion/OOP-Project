// ============================================================
// Validator.cpp
// The ONLY class permitted to contain input-validation logic.
// All methods are static; no std::string or library functions.
// ============================================================

#include "Validator.h"

// ============================================================
// isDigit — returns true if c is '0'..'9'
// ============================================================
bool Validator::isDigit(char c) {
    return (c >= '0' && c <= '9');
}

// ============================================================
// strLen — counts characters up to (but not including) '\0'
// ============================================================
int Validator::strLen(const char* s) {
    int len = 0;
    while (s[len] != '\0') len++;
    return len;
}

// ============================================================
// charsEqual — true only if a and b have identical content
// ============================================================
bool Validator::charsEqual(const char* a, const char* b) {
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        if (a[i] != b[i]) return false;
        i++;
    }
    return (a[i] == '\0' && b[i] == '\0');
}

// ============================================================
// charToInt
// Converts a digit string to int manually. No atoi / strtol.
// Returns -1 if the string contains any non-digit character.
// ============================================================
int Validator::charToInt(const char* s) {
    if (s == nullptr || s[0] == '\0') return -1;
    int result = 0;
    for (int i = 0; s[i] != '\0'; i++) {
        if (!isDigit(s[i])) return -1;
        result = result * 10 + (s[i] - '0');
    }
    return result;
}

// ============================================================
// toLowerChar
// Converts an uppercase letter to lowercase manually.
// Allowed by the spec: "use tolower on each character".
// We implement the logic ourselves rather than calling tolower().
// ============================================================
char Validator::toLowerChar(char c) {
    if (c >= 'A' && c <= 'Z') {
        return (char)(c + ('a' - 'A'));
    }
    return c;
}

// ============================================================
// specializationMatch
// Case-insensitive comparison: converts each character with
// toLowerChar() before comparing.  Returns true if equal.
// ============================================================
bool Validator::specializationMatch(const char* a, const char* b) {
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        if (toLowerChar(a[i]) != toLowerChar(b[i])) return false;
        i++;
    }
    return (a[i] == '\0' && b[i] == '\0');
}

// ============================================================
// validateId — a valid ID is any integer greater than 0
// ============================================================
bool Validator::validateId(int id) {
    return (id > 0);
}

// ============================================================
// validateDate
// Validates a date string in the format DD-MM-YYYY.
//
// Rules:
//   - Exactly 10 characters total
//   - Positions [2] and [5] must be '-'
//   - All other positions must be numeric digits
//   - Day (positions [0][1]):  01..31
//   - Month (positions [3][4]): 01..12
//   - Year (positions [6..9]):  >= currentYear
// ============================================================
bool Validator::validateDate(const char* date, int currentYear) {
    // Check exact length of 10
    if (strLen(date) != 10) return false;

    // Check separator positions
    if (date[2] != '-' || date[5] != '-') return false;

    // Check that every non-separator position is a digit
    int digitPos[] = { 0, 1, 3, 4, 6, 7, 8, 9 };
    for (int i = 0; i < 8; i++) {
        if (!isDigit(date[digitPos[i]])) return false;
    }

    // Extract day, month, year using character arithmetic
    int day = (date[0] - '0') * 10 + (date[1] - '0');
    int month = (date[3] - '0') * 10 + (date[4] - '0');
    int year = (date[6] - '0') * 1000 + (date[7] - '0') * 100
        + (date[8] - '0') * 10 + (date[9] - '0');

    // Validate ranges
    if (day < 1 || day   > 31) return false;
    if (month < 1 || month > 12) return false;
    if (year < currentYear)     return false;

    return true;
}

// ============================================================
// validateTimeSlot
// Must be one of the 8 fixed daily slots.
// Comparison is done character-by-character (no strcmp).
// ============================================================
bool Validator::validateTimeSlot(const char* slot) {
    // The 8 valid slots stored as string literals
    const char* validSlots[8] = {
        "09:00", "10:00", "11:00", "12:00",
        "13:00", "14:00", "15:00", "16:00"
    };

    for (int i = 0; i < 8; i++) {
        if (charsEqual(slot, validSlots[i])) {
            return true; // found a match
        }
    }
    return false; // no match found
}

// ============================================================
// validateContact
// Must be exactly 11 characters, all numeric digits.
// ============================================================
bool Validator::validateContact(const char* contact) {
    if (strLen(contact) != 11) return false;
    for (int i = 0; i < 11; i++) {
        if (!isDigit(contact[i])) return false;
    }
    return true;
}

// ============================================================
// validatePassword
// Must be at least 6 characters long.
// ============================================================
bool Validator::validatePassword(const char* password) {
    return (strLen(password) >= 6);
}

// ============================================================
// validatePositiveFloat
// Value must be strictly greater than zero.
// ============================================================
bool Validator::validatePositiveFloat(float value) {
    return (value > 0.0f);
}

// ============================================================
// validateMenuChoice
// Choice must be within the inclusive range [min, max].
// ============================================================
bool Validator::validateMenuChoice(int choice, int min, int max) {
    return (choice >= min && choice <= max);
}