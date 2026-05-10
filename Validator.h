#pragma once
#ifndef VALIDATOR_H
#define VALIDATOR_H

// ============================================================
// Validator.h
// The ONLY class permitted to contain input validation logic.
// All methods are static — no need to instantiate this class.
//
// Validation rules (derived from project specification):
//   ID          — positive integer (> 0)
//   Date        — DD-MM-YYYY; day 01-31, month 01-12,
//                 year >= current year; validated manually
//   Time slot   — must be one of the 8 fixed daily slots
//   Contact     — exactly 11 numeric digits
//   Password    — minimum 6 characters
//   Positive float — value > 0.0
//   Menu choice — integer within a given range [min, max]
//
// No std::string, strcmp, strtok, or any banned functions used.
// All comparisons are done manually using char-by-char logic.
// ============================================================

class Validator {
public:
    // ----------------------------------------------------------
    // validateId
    // Returns true if the given integer is > 0.
    // ----------------------------------------------------------
    static bool validateId(int id);

    // ----------------------------------------------------------
    // validateDate
    // Checks that 'date' matches DD-MM-YYYY exactly:
    //   - length must be 10 characters
    //   - positions 2 and 5 must be '-'
    //   - day part (DD): 01 to 31  (numeric digits only)
    //   - month part (MM): 01 to 12
    //   - year part (YYYY): >= currentYear (passed in as int)
    // Returns true if valid.
    // ----------------------------------------------------------
    static bool validateDate(const char* date, int currentYear);

    // ----------------------------------------------------------
    // validateTimeSlot
    // Checks that 'slot' is one of the 8 allowed values:
    //   "09:00" "10:00" "11:00" "12:00"
    //   "13:00" "14:00" "15:00" "16:00"
    // Comparison done character-by-character (no strcmp).
    // Returns true if valid.
    // ----------------------------------------------------------
    static bool validateTimeSlot(const char* slot);

    // ----------------------------------------------------------
    // validateContact
    // Returns true if:
    //   - length == 11
    //   - every character is a digit '0'..'9'
    // ----------------------------------------------------------
    static bool validateContact(const char* contact);

    // ----------------------------------------------------------
    // validatePassword
    // Returns true if the password is at least 6 characters long.
    // ----------------------------------------------------------
    static bool validatePassword(const char* password);

    // ----------------------------------------------------------
    // validatePositiveFloat
    // Returns true if value > 0.0f.
    // ----------------------------------------------------------
    static bool validatePositiveFloat(float value);

    // ----------------------------------------------------------
    // validateMenuChoice
    // Returns true if choice is within [min, max] inclusive.
    // ----------------------------------------------------------
    static bool validateMenuChoice(int choice, int min, int max);

    // ----------------------------------------------------------
    // isDigit
    // Returns true if c is '0' through '9'.
    // Private-equivalent helper exposed as static for reuse.
    // ----------------------------------------------------------
    static bool isDigit(char c);

    // ----------------------------------------------------------
    // charToInt
    // Manually converts a null-terminated numeric char array to
    // an integer (no atoi / strtol).
    // Returns -1 if any non-digit character is found.
    // ----------------------------------------------------------
    static int charToInt(const char* s);

    // ----------------------------------------------------------
    // strLen
    // Manually counts characters until the null terminator.
    // (Replaces strlen — no library calls.)
    // ----------------------------------------------------------
    static int strLen(const char* s);

    // ----------------------------------------------------------
    // charsEqual
    // Manually compares two char arrays character-by-character.
    // Returns true only if both have the same length and every
    // character matches.  (Replaces strcmp == 0.)
    // ----------------------------------------------------------
    static bool charsEqual(const char* a, const char* b);

    // ----------------------------------------------------------
    // toLowerChar
    // Returns the lowercase version of c if c is 'A'..'Z',
    // otherwise returns c unchanged.
    // (Replaces tolower — but uses tolower per spec which says
    //  we CAN use tolower on each character manually.)
    // ----------------------------------------------------------
    static char toLowerChar(char c);

    // ----------------------------------------------------------
    // specializationMatch
    // Case-insensitive comparison of two specialization strings.
    // Converts each character with toLowerChar() before comparing.
    // Returns true if they match case-insensitively.
    // ----------------------------------------------------------
    static bool specializationMatch(const char* a, const char* b);
};

#endif // VALIDATOR_H