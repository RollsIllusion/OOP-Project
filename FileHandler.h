#pragma once
#ifndef FILEHANDLER_H
#define FILEHANDLER_H

// ============================================================
// FileHandler.h
// The ONLY class permitted to perform file I/O in MediCore.
// All other classes call FileHandler methods to read from and
// write to .txt files; they never use fopen/fclose themselves.
//
// All data files use CSV format (comma-separated, one record
// per line, no spaces around commas):
//
//   patients.txt       — patient records
//   doctors.txt        — doctor records
//   admin.txt          — single admin record
//   appointments.txt   — appointment records
//   prescriptions.txt  — prescription records
//   bills.txt          — billing records
//   security_log.txt   — failed login events
//   discharged.txt     — archived discharged patient data
//
// Design notes:
//   - All parsing is done manually with char arrays.
//   - No std::string, strtok, strcmp, or any banned functions.
//   - char* splitting uses a private helper splitCSV().
// ============================================================

#include "Storage.h"
#include "Patient.h"
#include "Doctor.h"
#include "Admin.h"
#include "Appointment.h"
#include "Bill.h"
#include "Prescription.h"

class FileHandler {
public:
    // ----------------------------------------------------------
    // File path constants
    // These are the only places where filenames are stored.
    // ----------------------------------------------------------
    static const char* PATIENTS_FILE;
    static const char* DOCTORS_FILE;
    static const char* ADMIN_FILE;
    static const char* APPOINTMENTS_FILE;
    static const char* PRESCRIPTIONS_FILE;
    static const char* BILLS_FILE;
    static const char* SECURITY_LOG_FILE;
    static const char* DISCHARGED_FILE;

    // ----------------------------------------------------------
    // LOAD functions
    // Read all records from the corresponding file into the
    // supplied Storage<T> object.
    // Throws FileNotFoundException if the file cannot be opened.
    // ----------------------------------------------------------

    // Loads all patient records from patients.txt
    static void loadPatients(Storage<Patient>& store);

    // Loads all doctor records from doctors.txt
    static void loadDoctors(Storage<Doctor>& store);

    // Loads the single admin record from admin.txt
    // Returns false if the file is empty or unreadable.
    static bool loadAdmin(Admin& admin);

    // Loads all appointment records from appointments.txt
    static void loadAppointments(Storage<Appointment>& store);

    // Loads all prescription records from prescriptions.txt
    static void loadPrescriptions(Storage<Prescription>& store);

    // Loads all bill records from bills.txt
    static void loadBills(Storage<Bill>& store);

    // ----------------------------------------------------------
    // APPEND functions
    // Write a single new record as a CSV line at the end of
    // the specified file.  Creates the file if it doesn't exist.
    // ----------------------------------------------------------
    static void appendPatient(const Patient& p);
    static void appendDoctor(const Doctor& d);
    static void appendAppointment(const Appointment& a);
    static void appendPrescription(const Prescription& pr);
    static void appendBill(const Bill& b);

    // ----------------------------------------------------------
    // UPDATE functions
    // Rewrites the entire file replacing the record whose id
    // matches with an updated CSV line (all other lines kept).
    // ----------------------------------------------------------

    // Updates a patient record (e.g. after balance change)
    static void updatePatient(const Storage<Patient>& store);

    // Updates an appointment record (e.g. status change)
    static void updateAppointment(const Storage<Appointment>& store);

    // Updates a bill record (e.g. status change)
    static void updateBill(const Storage<Bill>& store);

    // ----------------------------------------------------------
    // DELETE functions
    // Removes the record with the given ID by rewriting the
    // file without that line.
    // ----------------------------------------------------------
    static void deletePatient(int patientId, Storage<Patient>& store);
    static void deleteDoctor(int doctorId, Storage<Doctor>& store);
    static void deleteAppointmentsByPatient(int patientId,
        Storage<Appointment>& store);
    static void deletePrescriptionsByPatient(int patientId,
        Storage<Prescription>& store);
    static void deleteBillsByPatient(int patientId, Storage<Bill>& store);

    // ----------------------------------------------------------
    // SECURITY LOG
    // Appends one failed-login entry to security_log.txt.
    // Format:  timestamp,role,entered_id,FAILED
    // timestamp is obtained from <ctime> inside the implementation.
    // ----------------------------------------------------------
    static void logFailedLogin(const char* role, const char* enteredId);

    // Reads and prints security_log.txt line by line to console.
    static void printSecurityLog();

    // ----------------------------------------------------------
    // DISCHARGE ARCHIVE
    // Copies patient row + all related rows into discharged.txt
    // in the same CSV format used by each original file.
    // ----------------------------------------------------------
    static void archivePatient(const Patient& p,
        const Storage<Appointment>& appts,
        const Storage<Prescription>& prescriptions,
        const Storage<Bill>& bills);

private:
    // ----------------------------------------------------------
    // Private helper — CSV line parser
    // Splits 'line' on commas, writing each field into fields[].
    // 'maxFields' is the size of the fields array.
    // Returns the number of fields actually found.
    // No strtok used — implemented manually with pointer arithmetic.
    // ----------------------------------------------------------
    static int splitCSV(const char* line, char fields[][300], int maxFields);

    // Private helper — converts a char[] to float manually
    // (no atof / strtof)
    static float charToFloat(const char* s);

    // Private helper — converts a char[] to int manually
    // (no atoi / strtol)
    static int charToInt(const char* s);

    // Private helper — writes a float into a char buffer manually
    // (no sprintf / snprintf)
    static void floatToChar(float value, char* buf, int decimals);

    // Private helper — writes an int into a char buffer manually
    static void intToChar(int value, char* buf);
};

#endif // FILEHANDLER_H