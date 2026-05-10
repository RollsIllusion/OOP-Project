#pragma once
#ifndef UTILITY_H
#define UTILITY_H

// ============================================================
// Utility.h
// A collection of free-standing helper functions used across
// the MediCore system.
//
// Responsibilities:
//   - Get today's date string (DD-MM-YYYY) using <ctime>
//   - Get the current year (integer)
//   - Manual implementations of common string operations that
//     replace banned library functions (no std::string, no
//     strcpy, no strcmp, no strtok, etc.)
//   - Sorting routines (insertion sort) for Appointment and
//     Prescription arrays — no std::sort or any library sort.
//   - Float-to-char and int-to-char conversion helpers
//     (these are also used inside FileHandler / toCSV methods)
//   - Trim trailing newline / carriage return from a char buffer
//     (needed after fgets reads lines from files)
// ============================================================

#include "Appointment.h"
#include "Prescription.h"
#include "DateTime.h"   // DateTime class replaces all struct tm usage

class Utility {
public:
    // ----------------------------------------------------------
    // Date / Time helpers
    // ----------------------------------------------------------

    // Fills 'buf' (must be at least 11 chars) with today's date
    // in the format DD-MM-YYYY using time() and localtime().
    static void getTodayDate(char* buf);

    // Returns the current year as an integer (e.g. 2025)
    static int getCurrentYear();

    // ----------------------------------------------------------
    // String helpers  (replace banned library functions)
    // ----------------------------------------------------------

    // Copies at most maxLen-1 characters from src into dest and
    // null-terminates.  (Replaces strncpy / strcpy)
    static void copyStr(char* dest, const char* src, int maxLen);

    // Returns the number of characters before the null terminator.
    // (Replaces strlen)
    static int strLen(const char* s);

    // Compares a and b character-by-character.
    // Returns 0 if identical, negative if a < b, positive if a > b.
    // (Replaces strcmp)
    static int strCmp(const char* a, const char* b);

    // Case-insensitive version of strCmp using toLower on each char.
    // Returns 0 if identical ignoring case.
    static int strCmpI(const char* a, const char* b);

    // Concatenates src onto the end of dest (up to maxLen total).
    // (Replaces strncat / strcat)
    static void appendStr(char* dest, const char* src, int maxLen);

    // ----------------------------------------------------------
    // Numeric conversion helpers
    // ----------------------------------------------------------

    // Converts an integer to its decimal char representation.
    // Writes into buf (caller must provide sufficient space, ~12 chars).
    static void intToStr(int value, char* buf);

    // Converts a float to a char representation with 'decimals'
    // decimal places.  e.g. floatToStr(1500.5f, buf, 2) → "1500.50"
    static void floatToStr(float value, char* buf, int decimals);

    // Converts a numeric char array to int without atoi/strtol.
    // Returns -1 if a non-digit character is found.
    static int  strToInt(const char* s);

    // Converts a numeric char array to float without atof/strtof.
    static float strToFloat(const char* s);

    // ----------------------------------------------------------
    // String manipulation helpers
    // ----------------------------------------------------------

    // Removes trailing '\n', '\r', or spaces from buf in-place.
    static void trimNewline(char* buf);

    // Converts every character of buf to lowercase in-place
    // using the project-legal toLower approach (char-by-char).
    static void toLowerInPlace(char* buf);

    // ----------------------------------------------------------
    // Sorting helpers (implement insertion sort — O(n^2) is fine)
    // No library sort functions used.
    // ----------------------------------------------------------

    // Sorts an array of Appointment pointers by date ASCENDING.
    // Used for "View My Appointments" and doctor's daily view.
    static void sortAppointmentsAsc(Appointment** arr, int n);

    // Sorts an array of Appointment pointers by date DESCENDING.
    // Used for admin's "View All Appointments".
    static void sortAppointmentsDesc(Appointment** arr, int n);

    // Sorts an array of Prescription pointers by date DESCENDING.
    // Used for "View My Medical Records" (most recent first).
    static void sortPrescriptionsDesc(Prescription** arr, int n);

    // ----------------------------------------------------------
    // Menu / display helpers
    // ----------------------------------------------------------

    // Prints a horizontal rule of 'width' dashes to console.
    static void printLine(int width);

    // Reads an integer from stdin safely (no cin >> int directly).
    // Returns the parsed integer, or -1 on non-numeric input.
    static int readInt();

    // Reads a float from stdin safely.
    // Returns the parsed float, or -1.0f on invalid input.
    static float readFloat();

    // Reads a line of text from stdin into buf (max maxLen chars).
    // Strips the trailing newline.
    static void readLine(char* buf, int maxLen);
};

#endif // UTILITY_H