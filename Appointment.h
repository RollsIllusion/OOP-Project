#pragma once
#ifndef APPOINTMENT_H
#define APPOINTMENT_H

// ============================================================
// Appointment.h
// Represents a single scheduled appointment between a patient
// and a doctor in the MediCore system.
//
// Status values (stored as char arrays):
//   "pending"   — booked, not yet seen
//   "completed" — doctor marked as done
//   "cancelled" — patient or admin cancelled
//   "noshow"    — patient did not attend
//
// Operator overloads required by the project spec:
//   == : conflict detection — two appointments conflict if they
//        share the same doctorId, date, and timeSlot and
//        NEITHER has status "cancelled".
//   << : formatted console output  (friend function)
// ============================================================

#include <ostream>

class Appointment {
private:
    int  appointmentId; // unique appointment identifier
    int  patientId;     // references a Patient record
    int  doctorId;      // references a Doctor record
    char date[12];      // format DD-MM-YYYY  (e.g. "15-04-2025")
    char timeSlot[6];   // one of the 8 fixed slots e.g. "09:00"
    char status[12];    // "pending" / "completed" / "cancelled" / "noshow"

public:
    // ----------------------------------------------------------
    // Constructors / Destructor
    // ----------------------------------------------------------

    // Default constructor — zeroes IDs, clears char fields
    Appointment();

    // Parameterized constructor
    Appointment(int appointmentId, int patientId, int doctorId,
        const char* date, const char* timeSlot, const char* status);

    // Destructor
    ~Appointment();

    // ----------------------------------------------------------
    // Getters
    // ----------------------------------------------------------
    int         getAppointmentId() const;
    int         getPatientId()     const;
    int         getDoctorId()      const;
    const char* getDate()          const;
    const char* getTimeSlot()      const;
    const char* getStatus()        const;

    // ----------------------------------------------------------
    // getId — required by Storage<Appointment> template
    // Returns the unique appointment ID (same as getAppointmentId).
    // ----------------------------------------------------------
    int getId() const;

    // ----------------------------------------------------------
    // Setters (used by FileHandler when updating status)
    // ----------------------------------------------------------
    void setStatus(const char* newStatus);

    // ----------------------------------------------------------
    // Operator Overloads
    // ----------------------------------------------------------

    // == : conflict check
    // Two appointments are "equal" (conflicting) when:
    //   - same doctorId
    //   - same date  (char-by-char comparison, no strcmp)
    //   - same timeSlot  (char-by-char comparison)
    //   - NEITHER appointment's status is "cancelled"
    bool operator==(const Appointment& other) const;

    // << : prints a formatted appointment record to an output stream
    friend std::ostream& operator<<(std::ostream& out, const Appointment& a);

    // ----------------------------------------------------------
    // File I/O helper
    // Serialises into CSV:
    // appointmentId,patientId,doctorId,date,timeSlot,status
    // ----------------------------------------------------------
    void toCSV(char* buffer, int maxLen) const;

    // ----------------------------------------------------------
    // Date comparison helper (used for sorting)
    // Compares this appointment's date with another's.
    // Returns negative if this < other, 0 if equal, positive if >
    // Date format is DD-MM-YYYY so we compare year → month → day.
    // ----------------------------------------------------------
    int compareDate(const Appointment& other) const;
};

#endif // APPOINTMENT_H