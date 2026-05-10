#pragma once
#ifndef BILL_H
#define BILL_H

// ============================================================
// Bill.h
// Represents a financial charge generated when an appointment
// is booked.  One bill is created per appointment.
//
// Status values:
//   "unpaid"    — generated on booking; patient owes money
//   "paid"      — patient settled the bill
//   "cancelled" — appointment was cancelled or marked no-show
//
// File format (bills.txt):
//   bill_id,patient_id,appointment_id,amount,status,date
// ============================================================

#include <ostream>

class Bill {
private:
    int   billId;        // unique bill identifier
    int   patientId;     // references a Patient record
    int   appointmentId; // references an Appointment record
    float amount;        // amount due in PKR
    char  status[12];    // "unpaid" / "paid" / "cancelled"
    char  date[12];      // date bill was generated  DD-MM-YYYY

public:
    // ----------------------------------------------------------
    // Constructors / Destructor
    // ----------------------------------------------------------

    // Default constructor
    Bill();

    // Parameterized constructor
    Bill(int billId, int patientId, int appointmentId,
        float amount, const char* status, const char* date);

    // Destructor
    ~Bill();

    // ----------------------------------------------------------
    // Getters
    // ----------------------------------------------------------
    int         getBillId()        const;
    int         getPatientId()     const;
    int         getAppointmentId() const;
    float       getAmount()        const;
    const char* getStatus()        const;
    const char* getDate()          const;

    // getId — required by Storage<Bill> template
    int getId() const;

    // ----------------------------------------------------------
    // Setters (used when paying or cancelling a bill)
    // ----------------------------------------------------------
    void setStatus(const char* newStatus);

    // ----------------------------------------------------------
    // File I/O helper
    // Serialises into CSV:
    // billId,patientId,appointmentId,amount,status,date
    // ----------------------------------------------------------
    void toCSV(char* buffer, int maxLen) const;

    // << : formatted console output
    friend std::ostream& operator<<(std::ostream& out, const Bill& b);
};

#endif // BILL_H