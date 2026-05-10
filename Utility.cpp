// ============================================================
// Utility.cpp
// General-purpose helpers: date/time, string ops, numeric
// conversions, sorting routines, and console I/O.
// No banned library functions (std::string, strcmp, atoi etc.)
// No 'struct' keyword — all date/time work goes through DateTime.
// ============================================================

#include "Utility.h"
#include "DateTime.h" // replaces direct <ctime> / struct tm usage
#include <cstdio>     // fgets — for safe input reading
#include <iostream>   // cout for printLine

// ============================================================
// DATE / TIME HELPERS
// ============================================================

// ------------------------------------------------------------
// getTodayDate
// Delegates to DateTime::fromNow() so no 'struct tm' appears
// anywhere in Utility.cpp.  'buf' must be at least 11 chars.
// ------------------------------------------------------------
void Utility::getTodayDate(char* buf) {
    // DateTime::fromNow() calls time() + localtime() internally;
    // struct tm is hidden inside DateTime.cpp only.
    DateTime now = DateTime::fromNow();

    // Use the DateTime method to format DD-MM-YYYY into buf
    now.toDateString(buf);
}

// ------------------------------------------------------------
// getCurrentYear
// Returns the current 4-digit year without using struct tm here.
// ------------------------------------------------------------
int Utility::getCurrentYear() {
    DateTime now = DateTime::fromNow();
    return now.getYear();
}

// ============================================================
// STRING HELPERS
// ============================================================

// ------------------------------------------------------------
// copyStr — safe copy up to (maxLen-1) chars, always terminates
// ------------------------------------------------------------
void Utility::copyStr(char* dest, const char* src, int maxLen) {
    int i = 0;
    while (src[i] != '\0' && i < maxLen - 1) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

// ------------------------------------------------------------
// strLen — manual strlen replacement
// ------------------------------------------------------------
int Utility::strLen(const char* s) {
    int len = 0;
    while (s[len] != '\0') len++;
    return len;
}

// ------------------------------------------------------------
// strCmp — manual strcmp replacement
// Returns 0 if equal, negative if a < b, positive if a > b
// ------------------------------------------------------------
int Utility::strCmp(const char* a, const char* b) {
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        if (a[i] != b[i])
            return (unsigned char)a[i] - (unsigned char)b[i];
        i++;
    }
    return (unsigned char)a[i] - (unsigned char)b[i];
}

// ------------------------------------------------------------
// strCmpI — case-insensitive comparison
// Converts each character with toLower logic before comparing.
// Returns 0 if equal ignoring case.
// ------------------------------------------------------------
int Utility::strCmpI(const char* a, const char* b) {
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        char ca = a[i];
        char cb = b[i];
        // manual toLower
        if (ca >= 'A' && ca <= 'Z') ca = (char)(ca + ('a' - 'A'));
        if (cb >= 'A' && cb <= 'Z') cb = (char)(cb + ('a' - 'A'));
        if (ca != cb) return (unsigned char)ca - (unsigned char)cb;
        i++;
    }
    char ca = a[i]; char cb = b[i];
    if (ca >= 'A' && ca <= 'Z') ca = (char)(ca + ('a' - 'A'));
    if (cb >= 'A' && cb <= 'Z') cb = (char)(cb + ('a' - 'A'));
    return (unsigned char)ca - (unsigned char)cb;
}

// ------------------------------------------------------------
// appendStr — concatenates src onto dest, respecting maxLen total
// ------------------------------------------------------------
void Utility::appendStr(char* dest, const char* src, int maxLen) {
    int i = 0;
    // Find end of dest
    while (dest[i] != '\0' && i < maxLen - 1) i++;
    // Append src
    int j = 0;
    while (src[j] != '\0' && i < maxLen - 1) {
        dest[i++] = src[j++];
    }
    dest[i] = '\0';
}

// ============================================================
// NUMERIC CONVERSION HELPERS
// ============================================================

// ------------------------------------------------------------
// intToStr — converts integer to decimal char representation
// buf must have room for at least 12 characters (sign + digits + \0)
// ------------------------------------------------------------
void Utility::intToStr(int value, char* buf) {
    if (value == 0) { buf[0] = '0'; buf[1] = '\0'; return; }

    char tmp[20];
    int  len = 0;
    bool neg = (value < 0);
    if (neg) value = -value;

    while (value > 0 && len < 19) {
        tmp[len++] = (char)('0' + value % 10);
        value /= 10;
    }

    int pos = 0;
    if (neg) buf[pos++] = '-';
    for (int k = len - 1; k >= 0; k--) buf[pos++] = tmp[k];
    buf[pos] = '\0';
}

// ------------------------------------------------------------
// floatToStr — converts float to string with 'decimals' decimal places
// e.g. floatToStr(1500.5f, buf, 2) → "1500.50"
// ------------------------------------------------------------
void Utility::floatToStr(float value, char* buf, int decimals) {
    int pos = 0;
    if (value < 0.0f) { buf[pos++] = '-'; value = -value; }

    int intPart = (int)value;
    float fracPart = value - (float)intPart;

    // Write integer part using intToStr
    char tmp[20];
    intToStr(intPart, tmp);
    for (int i = 0; tmp[i] != '\0'; i++) buf[pos++] = tmp[i];

    if (decimals > 0) {
        buf[pos++] = '.';
        // Multiply fractional part to get required decimal digits
        for (int d = 0; d < decimals; d++) {
            fracPart *= 10.0f;
            int digit = (int)(fracPart + (d == decimals - 1 ? 0.5f : 0.0f));
            if (digit >= 10) digit = 9; // clamp rounding overflow
            buf[pos++] = (char)('0' + digit);
            fracPart -= (float)(int)fracPart;
        }
    }
    buf[pos] = '\0';
}

// ------------------------------------------------------------
// strToInt — manual atoi replacement
// Returns -1 on invalid (non-digit) input.
// ------------------------------------------------------------
int Utility::strToInt(const char* s) {
    if (s == nullptr || s[0] == '\0') return -1;
    int result = 0;
    int i = 0;
    bool neg = false;
    if (s[0] == '-') { neg = true; i = 1; }
    for (; s[i] != '\0'; i++) {
        if (s[i] < '0' || s[i] > '9') return -1;
        result = result * 10 + (s[i] - '0');
    }
    return neg ? -result : result;
}

// ------------------------------------------------------------
// strToFloat — manual atof replacement
// Handles optional leading '-', integer part, and '.XX' fraction.
// ------------------------------------------------------------
float Utility::strToFloat(const char* s) {
    if (s == nullptr || s[0] == '\0') return -1.0f;

    float result = 0.0f;
    int   i = 0;
    bool  neg = false;

    if (s[0] == '-') { neg = true; i = 1; }

    // Integer part
    while (s[i] != '\0' && s[i] != '.') {
        if (s[i] < '0' || s[i] > '9') return -1.0f;
        result = result * 10.0f + (float)(s[i] - '0');
        i++;
    }

    // Fractional part
    if (s[i] == '.') {
        i++;
        float factor = 0.1f;
        while (s[i] != '\0') {
            if (s[i] < '0' || s[i] > '9') return -1.0f;
            result += (float)(s[i] - '0') * factor;
            factor *= 0.1f;
            i++;
        }
    }

    return neg ? -result : result;
}

// ============================================================
// STRING MANIPULATION HELPERS
// ============================================================

// ------------------------------------------------------------
// trimNewline
// Removes trailing '\n', '\r', or space characters in-place.
// Needed after fgets() which includes the newline character.
// ------------------------------------------------------------
void Utility::trimNewline(char* buf) {
    int len = strLen(buf);
    while (len > 0 &&
        (buf[len - 1] == '\n' || buf[len - 1] == '\r' || buf[len - 1] == ' ')) {
        buf[--len] = '\0';
    }
}

// ------------------------------------------------------------
// toLowerInPlace — converts every character to lowercase in buf
// ------------------------------------------------------------
void Utility::toLowerInPlace(char* buf) {
    for (int i = 0; buf[i] != '\0'; i++) {
        if (buf[i] >= 'A' && buf[i] <= 'Z') {
            buf[i] = (char)(buf[i] + ('a' - 'A'));
        }
    }
}

// ============================================================
// SORTING HELPERS
// All use insertion sort — O(n²) but acceptable for ≤100 items.
// No std::sort or any library sort used.
// ============================================================

// ------------------------------------------------------------
// sortAppointmentsAsc
// Sorts an array of Appointment pointers by date ASCENDING.
// Uses Appointment::compareDate() for comparison.
// ------------------------------------------------------------
void Utility::sortAppointmentsAsc(Appointment** arr, int n) {
    // Insertion sort
    for (int i = 1; i < n; i++) {
        Appointment* key = arr[i];
        int j = i - 1;
        // Shift elements that are GREATER than key to the right
        while (j >= 0 && arr[j]->compareDate(*key) > 0) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

// ------------------------------------------------------------
// sortAppointmentsDesc
// Sorts an array of Appointment pointers by date DESCENDING.
// ------------------------------------------------------------
void Utility::sortAppointmentsDesc(Appointment** arr, int n) {
    // Insertion sort — shift elements LESS than key to the right
    for (int i = 1; i < n; i++) {
        Appointment* key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j]->compareDate(*key) < 0) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

// ------------------------------------------------------------
// sortPrescriptionsDesc
// Sorts an array of Prescription pointers by date DESCENDING.
// ------------------------------------------------------------
void Utility::sortPrescriptionsDesc(Prescription** arr, int n) {
    for (int i = 1; i < n; i++) {
        Prescription* key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j]->compareDate(*key) < 0) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

// ============================================================
// MENU / DISPLAY HELPERS
// ============================================================

// ------------------------------------------------------------
// printLine — prints 'width' dash characters then a newline
// ------------------------------------------------------------
void Utility::printLine(int width) {
    for (int i = 0; i < width; i++) std::cout << '-';
    std::cout << '\n';
}

// ------------------------------------------------------------
// readInt
// Reads a line from stdin, then manually converts to int.
// Returns -1 if the input is not a valid integer.
// Avoids cin >> int which leaves newlines in the buffer.
// ------------------------------------------------------------
int Utility::readInt() {
    char buf[32];
    if (fgets(buf, 32, stdin) == nullptr) return -1;
    trimNewline(buf);
    return strToInt(buf);
}

// ------------------------------------------------------------
// readFloat
// Reads a line from stdin and converts to float manually.
// Returns -1.0f on invalid input.
// ------------------------------------------------------------
float Utility::readFloat() {
    char buf[32];
    if (fgets(buf, 32, stdin) == nullptr) return -1.0f;
    trimNewline(buf);
    return strToFloat(buf);
}

// ------------------------------------------------------------
// readLine
// Reads up to (maxLen-1) characters from stdin into buf.
// Strips the trailing newline character left by fgets.
// ------------------------------------------------------------
void Utility::readLine(char* buf, int maxLen) {
    if (fgets(buf, maxLen, stdin) == nullptr) {
        buf[0] = '\0';
        return;
    }
    trimNewline(buf);
}