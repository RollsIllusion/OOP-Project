// ============================================================
// DateTime.h
// Wraps all struct tm / ctime usage so no other file needs to
// include <ctime> or use 'struct tm' directly.
//
// Usage summary:
//   DateTime now  = DateTime::fromNow();          // current time
//   DateTime d    = DateTime::fromDateString(s);  // parse DD-MM-YYYY
//   now.toDateString(buf);                         // writes DD-MM-YYYY
//   now.toTimestampString(buf);                    // writes DD-MM-YYYY HH:MM:SS
//   int y = now.getYear();
//   double days = DateTime::diffDays(a, b);        // (b - a) in days
// ============================================================

#ifndef DATETIME_H
#define DATETIME_H

class DateTime {
public:
    // --------------------------------------------------------
    // Construction
    // --------------------------------------------------------
    DateTime();   // default — zero-initialised fields

    // --------------------------------------------------------
    // Factory methods
    // --------------------------------------------------------
    static DateTime fromNow();                       // current local time
    static DateTime fromDateString(const char* s);  // parse "DD-MM-YYYY"

    // --------------------------------------------------------
    // Formatters
    // --------------------------------------------------------
    void toDateString(char* buf) const;       // writes "DD-MM-YYYY\0"  (needs 11 bytes)
    void toTimestampString(char* buf) const;  // writes "DD-MM-YYYY HH:MM:SS\0" (needs 20 bytes)

    // --------------------------------------------------------
    // Accessors
    // --------------------------------------------------------
    int getYear()  const;
    int getMonth() const;
    int getDay()   const;

    // --------------------------------------------------------
    // Utility
    // --------------------------------------------------------
    // Returns (b - a) in whole days (positive if b is later).
    static double diffDays(const DateTime& a, const DateTime& b);

private:
    // Store fields as plain ints — no struct tm visible in header.
    int year;   // full 4-digit year, e.g. 2026
    int month;  // 1-12
    int day;    // 1-31
    int hour;   // 0-23
    int minute; // 0-59
    int second; // 0-59

    // Private constructor used by factory methods
    DateTime(int year, int month, int day,
        int hour, int minute, int second);

    // Helper: write a two-digit int (zero-padded) into buf at pos
    static void writeTwoDigits(char* buf, int& pos, int value);
};

#endif // DATETIME_H