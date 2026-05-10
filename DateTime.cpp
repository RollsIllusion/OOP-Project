// ============================================================
// DateTime.cpp
// The ONLY file in MediCore that includes <ctime> or uses
// struct tm.  All date/time logic is isolated here so the
// rest of the codebase stays free of struct usage.
// ============================================================

// Suppress MSVC C4996 deprecation warnings for localtime / mktime.
// These are standard C functions; the _s variants are MSVC-only
// and would break portability.  The define must come BEFORE any
// system header is included.
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "DateTime.h"
#include <ctime>   // time_t, time(), localtime(), difftime(), mktime()

// ============================================================
// Private helpers
// ============================================================

// ------------------------------------------------------------
// writeTwoDigits
// Writes a zero-padded two-digit number into buf at position pos.
// Example: value=9  → writes '0','9'
//          value=15 → writes '1','5'
// Advances pos by 2.
// ------------------------------------------------------------
void DateTime::writeTwoDigits(char* buf, int& pos, int value) {
    buf[pos++] = (char)('0' + (value / 10) % 10);
    buf[pos++] = (char)('0' + value % 10);
}

// ============================================================
// Constructors
// ============================================================

// Default constructor — zero all fields
DateTime::DateTime()
    : year(0), month(1), day(1), hour(0), minute(0), second(0) {
}

// Private full constructor
DateTime::DateTime(int y, int mo, int d, int h, int mi, int s)
    : year(y), month(mo), day(d), hour(h), minute(mi), second(s) {
}

// ============================================================
// Factory: fromNow
// Calls time() then localtime() — the ONLY place struct tm appears.
// ============================================================
DateTime DateTime::fromNow() {
    time_t raw = time(nullptr);
    struct tm* t = localtime(&raw);

    // tm_year is years since 1900; tm_mon is 0-based
    return DateTime(
        t->tm_year + 1900,
        t->tm_mon + 1,
        t->tm_mday,
        t->tm_hour,
        t->tm_min,
        t->tm_sec
    );
}

// ============================================================
// Factory: fromDateString
// Parses a date string in "DD-MM-YYYY" format.
// Time fields are set to midnight (00:00:00).
// If the string is malformed the result is a zero DateTime.
// ============================================================
DateTime DateTime::fromDateString(const char* s) {
    // Expect exactly "DD-MM-YYYY" = 10 characters
    // Layout: [0][1]-[3][4]-[6][7][8][9]
    if (s == nullptr) return DateTime();

    // Check minimum length
    int len = 0;
    while (s[len] != '\0') len++;
    if (len < 10) return DateTime();

    // Check separator positions
    if (s[2] != '-' || s[5] != '-') return DateTime();

    int day = (s[0] - '0') * 10 + (s[1] - '0');
    int month = (s[3] - '0') * 10 + (s[4] - '0');
    int year = (s[6] - '0') * 1000 + (s[7] - '0') * 100
        + (s[8] - '0') * 10 + (s[9] - '0');

    return DateTime(year, month, day, 0, 0, 0);
}

// ============================================================
// Formatters
// ============================================================

// ------------------------------------------------------------
// toDateString
// Writes "DD-MM-YYYY" into buf (needs at least 11 bytes).
// ------------------------------------------------------------
void DateTime::toDateString(char* buf) const {
    int pos = 0;
    writeTwoDigits(buf, pos, day);
    buf[pos++] = '-';
    writeTwoDigits(buf, pos, month);
    buf[pos++] = '-';
    // Write 4-digit year manually
    buf[pos++] = (char)('0' + (year / 1000) % 10);
    buf[pos++] = (char)('0' + (year / 100) % 10);
    buf[pos++] = (char)('0' + (year / 10) % 10);
    buf[pos++] = (char)('0' + year % 10);
    buf[pos] = '\0';
}

// ------------------------------------------------------------
// toTimestampString
// Writes "DD-MM-YYYY HH:MM:SS" into buf (needs at least 20 bytes).
// ------------------------------------------------------------
void DateTime::toTimestampString(char* buf) const {
    int pos = 0;

    // Date part
    writeTwoDigits(buf, pos, day);
    buf[pos++] = '-';
    writeTwoDigits(buf, pos, month);
    buf[pos++] = '-';
    buf[pos++] = (char)('0' + (year / 1000) % 10);
    buf[pos++] = (char)('0' + (year / 100) % 10);
    buf[pos++] = (char)('0' + (year / 10) % 10);
    buf[pos++] = (char)('0' + year % 10);

    buf[pos++] = ' ';

    // Time part
    writeTwoDigits(buf, pos, hour);
    buf[pos++] = ':';
    writeTwoDigits(buf, pos, minute);
    buf[pos++] = ':';
    writeTwoDigits(buf, pos, second);

    buf[pos] = '\0';
}

// ============================================================
// Accessors
// ============================================================

int DateTime::getYear()  const { return year; }
int DateTime::getMonth() const { return month; }
int DateTime::getDay()   const { return day; }

// ============================================================
// diffDays
// Returns (b - a) in whole days.
// Converts both DateTime objects to time_t using mktime() then
// uses difftime() — both are <ctime> functions, safely hidden here.
// ============================================================
double DateTime::diffDays(const DateTime& a, const DateTime& b) {
    // Build struct tm for 'a'
    struct tm tmA = {};
    tmA.tm_year = a.year - 1900;
    tmA.tm_mon = a.month - 1;
    tmA.tm_mday = a.day;
    tmA.tm_hour = a.hour;
    tmA.tm_min = a.minute;
    tmA.tm_sec = a.second;
    tmA.tm_isdst = -1;   // let the library determine DST

    // Build struct tm for 'b'
    struct tm tmB = {};
    tmB.tm_year = b.year - 1900;
    tmB.tm_mon = b.month - 1;
    tmB.tm_mday = b.day;
    tmB.tm_hour = b.hour;
    tmB.tm_min = b.minute;
    tmB.tm_sec = b.second;
    tmB.tm_isdst = -1;

    time_t tA = mktime(&tmA);
    time_t tB = mktime(&tmB);

    // difftime returns (tB - tA) in seconds; divide by 86400 for days
    double seconds = difftime(tB, tA);
    return seconds / 86400.0;
}