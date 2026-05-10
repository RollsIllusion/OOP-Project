// ============================================================
// InsufficientFundsException.cpp
// Thrown when a patient's balance is less than the required
// amount for booking or paying a bill.
// ============================================================

#include "InsufficientFundsException.h"

// ------------------------------------------------------------
// Helper: write an integer into buf starting at position *pos.
// Advances *pos past the written digits.
// No sprintf / itoa used — manual digit extraction.
// ------------------------------------------------------------
static void writeInt(char* buf, int& pos, int value) {
    if (value < 0) {
        buf[pos++] = '-';
        value = -value;
    }
    // Temporary buffer to reverse digits
    char tmp[20];
    int len = 0;
    if (value == 0) {
        buf[pos++] = '0';
        return;
    }
    while (value > 0 && len < 19) {
        tmp[len++] = (char)('0' + (value % 10));
        value /= 10;
    }
    // Reverse into buf
    for (int k = len - 1; k >= 0; k--) {
        buf[pos++] = tmp[k];
    }
}

// ------------------------------------------------------------
// Helper: write a float with 2 decimal places into buf.
// ------------------------------------------------------------
static void writeFloat(char* buf, int& pos, float value) {
    // Handle negative
    if (value < 0.0f) {
        buf[pos++] = '-';
        value = -value;
    }
    // Integer part
    int intPart = (int)value;
    writeInt(buf, pos, intPart);
    buf[pos++] = '.';
    // Two decimal digits
    int frac = (int)((value - (float)intPart) * 100.0f + 0.5f);
    buf[pos++] = (char)('0' + (frac / 10));
    buf[pos++] = (char)('0' + (frac % 10));
}

// ------------------------------------------------------------
// Helper: copy a string literal into buf from position *pos.
// ------------------------------------------------------------
static void writeStr(char* buf, int& pos, const char* s) {
    int i = 0;
    while (s[i] != '\0' && pos < 198) {
        buf[pos++] = s[i++];
    }
}

// ------------------------------------------------------------
// Constructor
// Builds: "Insufficient funds. Required: PKR X.XX, Available: PKR Y.YY"
// ------------------------------------------------------------
InsufficientFundsException::InsufficientFundsException(float required,
    float available)
    : HospitalException()
{
    int pos = 0;
    writeStr(message, pos, "Insufficient funds. Required: PKR ");
    writeFloat(message, pos, required);
    writeStr(message, pos, ", Available: PKR ");
    writeFloat(message, pos, available);
    message[pos] = '\0';
}

// ------------------------------------------------------------
// what() — returns inherited message buffer
// ------------------------------------------------------------
const char* InsufficientFundsException::what() const {
    return message;
}