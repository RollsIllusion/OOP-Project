// ============================================================
// Appointment.cpp
// Implements the Appointment class.
// Key feature: the == operator detects scheduling CONFLICTS,
// not equality. Two appointments conflict when same doctor,
// same date, same slot, and neither is "cancelled".
// ============================================================

#include "Appointment.h"
#include <iostream>

// ============================================================
// Internal static helpers (file-scope, not exposed in header)
// ============================================================

// Manual char-by-char equality check (no strcmp)
static bool charsEqual(const char* a, const char* b) {
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        if (a[i] != b[i]) return false;
        i++;
    }
    return (a[i] == '\0' && b[i] == '\0');
}

// Manual copy (no strcpy)
static void manualCopy(char* dest, const char* src, int maxLen) {
    int i = 0;
    while (src[i] != '\0' && i < maxLen - 1) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

// ============================================================
// Constructors / Destructor
// ============================================================

Appointment::Appointment()
    : appointmentId(0), patientId(0), doctorId(0)
{
    date[0] = '\0';
    timeSlot[0] = '\0';
    status[0] = '\0';
}

Appointment::Appointment(int appointmentId, int patientId, int doctorId,
    const char* date, const char* timeSlot,
    const char* status)
    : appointmentId(appointmentId), patientId(patientId), doctorId(doctorId)
{
    manualCopy(this->date, date, 12);
    manualCopy(this->timeSlot, timeSlot, 6);
    manualCopy(this->status, status, 12);
}

Appointment::~Appointment() {}

// ============================================================
// Getters
// ============================================================

int         Appointment::getAppointmentId() const { return appointmentId; }
int         Appointment::getPatientId()     const { return patientId; }
int         Appointment::getDoctorId()      const { return doctorId; }
const char* Appointment::getDate()          const { return date; }
const char* Appointment::getTimeSlot()      const { return timeSlot; }
const char* Appointment::getStatus()        const { return status; }

// ============================================================
// Setters
// ============================================================

void Appointment::setStatus(const char* newStatus) {
    manualCopy(status, newStatus, 12);
}

// ============================================================
// Operator Overloads
// ============================================================

// ------------------------------------------------------------
// == : CONFLICT detection (NOT equality of appointment records)
// Two appointments CONFLICT when:
//   1. Same doctor ID
//   2. Same date string
//   3. Same time slot string
//   4. NEITHER has status "cancelled"
// ------------------------------------------------------------
bool Appointment::operator==(const Appointment& other) const {
    // Check same doctor
    if (doctorId != other.doctorId) return false;

    // Check same date (character-by-character)
    if (!charsEqual(date, other.date)) return false;

    // Check same time slot (character-by-character)
    if (!charsEqual(timeSlot, other.timeSlot)) return false;

    // If either is cancelled, there is no conflict
    if (charsEqual(status, "cancelled"))       return false;
    if (charsEqual(other.status, "cancelled")) return false;

    // All conditions met — these appointments conflict
    return true;
}

// ------------------------------------------------------------
// << : formatted output
// Format: ID | Patient ID | Doctor ID | Date | Slot | Status
// ------------------------------------------------------------
std::ostream& operator<<(std::ostream& out, const Appointment& a) {
    out << "ApptID: " << a.appointmentId
        << " | PatID: " << a.patientId
        << " | DocID: " << a.doctorId
        << " | Date: " << a.date
        << " | Slot: " << a.timeSlot
        << " | Status: " << a.status;
    return out;
}

// ============================================================
// toCSV
// appointmentId,patientId,doctorId,date,timeSlot,status
// ============================================================
void Appointment::toCSV(char* buffer, int maxLen) const {
    int pos = 0;

    // Write integer helper (local lambda)
    auto writeInt = [&](int v) {
        char tmp[20]; int len = 0;
        if (v == 0) { buffer[pos++] = '0'; return; }
        while (v > 0 && len < 19) { tmp[len++] = (char)('0' + v % 10); v /= 10; }
        for (int k = len - 1; k >= 0 && pos < maxLen - 1; k--)
            buffer[pos++] = tmp[k];
        };

    auto writeStr = [&](const char* s) {
        for (int i = 0; s[i] != '\0' && pos < maxLen - 1; i++)
            buffer[pos++] = s[i];
        };

    writeInt(appointmentId); buffer[pos++] = ',';
    writeInt(patientId);     buffer[pos++] = ',';
    writeInt(doctorId);      buffer[pos++] = ',';
    writeStr(date);          buffer[pos++] = ',';
    writeStr(timeSlot);      buffer[pos++] = ',';
    writeStr(status);

    buffer[pos] = '\0';
}

// ============================================================
// compareDate
// Compares dates in DD-MM-YYYY format by extracting numeric
// parts without any library functions.
// Date layout: [0][1]-[3][4]-[6][7][8][9]
//               DD    MM    YYYY
// We compare: year first, then month, then day.
// Returns negative if this < other, 0 if equal, positive if >
// ============================================================
int Appointment::compareDate(const Appointment& other) const {
    // Extract YYYY from positions 6-9
    int y1 = (date[6] - '0') * 1000 + (date[7] - '0') * 100
        + (date[8] - '0') * 10 + (date[9] - '0');
    int y2 = (other.date[6] - '0') * 1000 + (other.date[7] - '0') * 100
        + (other.date[8] - '0') * 10 + (other.date[9] - '0');
    if (y1 != y2) return y1 - y2;

    // Extract MM from positions 3-4
    int m1 = (date[3] - '0') * 10 + (date[4] - '0');
    int m2 = (other.date[3] - '0') * 10 + (other.date[4] - '0');
    if (m1 != m2) return m1 - m2;

    // Extract DD from positions 0-1
    int d1 = (date[0] - '0') * 10 + (date[1] - '0');
    int d2 = (other.date[0] - '0') * 10 + (other.date[1] - '0');
    return d1 - d2;
}

// ============================================================
// getId — required by Storage<Appointment> template
// Returns appointmentId so Storage<Appointment>::findById
// and removeById can locate records by their unique ID.
// ============================================================
int Appointment::getId() const {
    return appointmentId;
}