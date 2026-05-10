#pragma once
#ifndef PRESCRIPTION_H
#define PRESCRIPTION_H

// ============================================================
// Prescription.h
// Represents a medical prescription written by a doctor for a
// patient after a completed appointment.
//
// File format (prescriptions.txt):
//   prescription_id,appointment_id,patient_id,doctor_id,
//   date,medicines,notes
//
// medicines is a semicolon-separated list of "Name Dosage" pairs
// e.g.  "Paracetamol 500mg;Amoxicillin 250mg"
// ============================================================

#include <ostream>

class Prescription {
private:
    int  prescriptionId; // unique prescription identifier
    int  appointmentId;  // references the completed Appointment
    int  patientId;      // references the Patient
    int  doctorId;       // references the Doctor who wrote it
    char date[12];       // date written  DD-MM-YYYY
    char medicines[500]; // semicolon-separated medicine list (max 499 chars)
    char notes[300];     // free-text doctor notes (max 299 chars)

public:
    // ----------------------------------------------------------
    // Constructors / Destructor
    // ----------------------------------------------------------

    // Default constructor — zeroes IDs, clears char arrays
    Prescription();

    // Parameterized constructor
    Prescription(int prescriptionId, int appointmentId, int patientId,
        int doctorId, const char* date,
        const char* medicines, const char* notes);

    // Destructor
    ~Prescription();

    // ----------------------------------------------------------
    // Getters
    // ----------------------------------------------------------
    int         getPrescriptionId() const;
    int         getAppointmentId()  const;
    int         getPatientId()      const;
    int         getDoctorId()       const;
    const char* getDate()           const;
    const char* getMedicines()      const;
    const char* getNotes()          const;

    // getId — required by Storage<Prescription> template
    int getId() const;

    // ----------------------------------------------------------
    // File I/O helper
    // Serialises into CSV (single line):
    // prescriptionId,appointmentId,patientId,doctorId,
    // date,medicines,notes
    // ----------------------------------------------------------
    void toCSV(char* buffer, int maxLen) const;

    // << : formatted console output (date, medicines, notes)
    friend std::ostream& operator<<(std::ostream& out, const Prescription& pr);

    // ----------------------------------------------------------
    // Date comparison helper (for descending sort)
    // Returns negative if this < other, 0 equal, positive if >
    // ----------------------------------------------------------
    int compareDate(const Prescription& other) const;
};

#endif // PRESCRIPTION_H