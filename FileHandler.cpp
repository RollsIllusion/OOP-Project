// Suppress MSVC C4996 'fopen'/'localtime' deprecation warnings.
// Must appear before any system header is included.
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

// ============================================================
// FileHandler.cpp
// The ONLY class that performs file I/O in MediCore.
// Uses C-style FILE* (fopen/fclose/fgets/fputs/fprintf) because
// std::fstream is not banned but FILE* is simpler for line-by-
// line CSV work without std::string.
//
// All parsing is done manually:
//   - splitCSV() breaks lines on commas without strtok
//   - charToInt / charToFloat convert text to numbers
//   - intToChar / floatToChar convert numbers to text
//
// Every write function immediately flushes the file so no data
// is lost if the program exits unexpectedly (satisfies the
// "every change is immediately written" requirement).
// ============================================================

#include "FileHandler.h"
#include "FileNotFoundException.h"
#include "Utility.h"
#include "DateTime.h"  // replaces direct struct tm / ctime usage
#include <cstdio>      // fopen, fclose, fgets, fputs, fflush

// ============================================================
// Static file path constants
// ============================================================
const char* FileHandler::PATIENTS_FILE = "patients.txt";
const char* FileHandler::DOCTORS_FILE = "doctors.txt";
const char* FileHandler::ADMIN_FILE = "admin.txt";
const char* FileHandler::APPOINTMENTS_FILE = "appointments.txt";
const char* FileHandler::PRESCRIPTIONS_FILE = "prescriptions.txt";
const char* FileHandler::BILLS_FILE = "bills.txt";
const char* FileHandler::SECURITY_LOG_FILE = "security_log.txt";
const char* FileHandler::DISCHARGED_FILE = "discharged.txt";

// ============================================================
// Private helpers
// ============================================================

// ------------------------------------------------------------
// charToInt
// Converts a null-terminated digit string to int.
// Returns -1 if any character is not a digit.
// ------------------------------------------------------------
int FileHandler::charToInt(const char* s) {
    if (s == nullptr || s[0] == '\0') return 0;
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
// charToFloat
// Manual atof replacement — handles integer and decimal parts.
// ------------------------------------------------------------
float FileHandler::charToFloat(const char* s) {
    if (s == nullptr || s[0] == '\0') return 0.0f;
    float result = 0.0f;
    int   i = 0;
    bool  neg = false;
    if (s[0] == '-') { neg = true; i = 1; }

    while (s[i] != '\0' && s[i] != '.') {
        if (s[i] < '0' || s[i] > '9') return 0.0f;
        result = result * 10.0f + (float)(s[i] - '0');
        i++;
    }
    if (s[i] == '.') {
        i++;
        float factor = 0.1f;
        while (s[i] != '\0') {
            if (s[i] < '0' || s[i] > '9') break;
            result += (float)(s[i] - '0') * factor;
            factor *= 0.1f;
            i++;
        }
    }
    return neg ? -result : result;
}

// ------------------------------------------------------------
// intToChar
// Writes the decimal representation of value into buf.
// buf must be large enough (~12 chars).
// ------------------------------------------------------------
void FileHandler::intToChar(int value, char* buf) {
    Utility::intToStr(value, buf);
}

// ------------------------------------------------------------
// floatToChar
// Writes float to buf with 'decimals' decimal places.
// ------------------------------------------------------------
void FileHandler::floatToChar(float value, char* buf, int decimals) {
    Utility::floatToStr(value, buf, decimals);
}

// ------------------------------------------------------------
// splitCSV
// Splits a comma-delimited line into fields[].
// Does NOT use strtok — scans character-by-character.
// Returns the number of fields found.
// Each field is stored in fields[i] (char array of size 300).
// Trailing newline in the last field is trimmed automatically.
// ------------------------------------------------------------
int FileHandler::splitCSV(const char* line, char fields[][300], int maxFields) {
    int fieldIndex = 0;  // which field we are currently writing into
    int charIndex = 0;  // position within current field

    for (int i = 0; line[i] != '\0' && fieldIndex < maxFields; i++) {
        if (line[i] == ',') {
            // Null-terminate current field and move to next
            fields[fieldIndex][charIndex] = '\0';
            fieldIndex++;
            charIndex = 0;
        }
        else if (line[i] == '\n' || line[i] == '\r') {
            // End of line — stop
            break;
        }
        else {
            // Regular character — add to current field
            if (charIndex < 299) {
                fields[fieldIndex][charIndex++] = line[i];
            }
        }
    }
    // Null-terminate the last field
    if (fieldIndex < maxFields) {
        fields[fieldIndex][charIndex] = '\0';
        fieldIndex++; // count the last field
    }
    return fieldIndex;
}

// ============================================================
// LOAD FUNCTIONS
// ============================================================

// ------------------------------------------------------------
// loadPatients
// File format: patient_id,name,age,gender,contact,password,balance
// ------------------------------------------------------------
void FileHandler::loadPatients(Storage<Patient>& store) {
    FILE* f = fopen(PATIENTS_FILE, "r");
    if (f == nullptr) {
        // File not existing on first run is acceptable — create it empty
        // Throw only for critical files; patients can start empty.
        return;
    }

    char line[512];
    store.clear(); // wipe any existing records before reloading

    while (fgets(line, 512, f) != nullptr) {
        // Skip empty lines
        if (line[0] == '\n' || line[0] == '\r' || line[0] == '\0') continue;

        char fields[7][300]; // 7 columns in patients.txt
        int  count = splitCSV(line, fields, 7);
        if (count < 7) continue; // malformed line — skip

        int   pid = charToInt(fields[0]);
        // fields[1] = name
        int   age = charToInt(fields[2]);
        char  gender = fields[3][0]; // single char 'M' or 'F'
        // fields[4] = contact
        // fields[5] = password
        float balance = charToFloat(fields[6]);

        Patient p(pid, fields[1], age, gender,
            fields[4], fields[5], balance);
        store.add(p);
    }

    fclose(f);
}

// ------------------------------------------------------------
// loadDoctors
// File format: doctor_id,name,specialization,contact,password,fee
// ------------------------------------------------------------
void FileHandler::loadDoctors(Storage<Doctor>& store) {
    FILE* f = fopen(DOCTORS_FILE, "r");
    if (f == nullptr) return; // empty on first run is fine

    char line[512];
    store.clear();

    while (fgets(line, 512, f) != nullptr) {
        if (line[0] == '\n' || line[0] == '\r' || line[0] == '\0') continue;

        char fields[6][300]; // 6 columns
        int  count = splitCSV(line, fields, 6);
        if (count < 6) continue;

        int   did = charToInt(fields[0]);
        float fee = charToFloat(fields[5]);

        Doctor d(did, fields[1], fields[2], fields[3], fields[4], fee);
        store.add(d);
    }

    fclose(f);
}

// ------------------------------------------------------------
// loadAdmin
// File format: admin_id,name,password
// There is exactly one admin record.
// Returns false if file is missing or empty.
// ------------------------------------------------------------
bool FileHandler::loadAdmin(Admin& admin) {
    FILE* f = fopen(ADMIN_FILE, "r");
    if (f == nullptr) return false;

    char line[256];
    bool loaded = false;

    if (fgets(line, 256, f) != nullptr) {
        if (line[0] != '\n' && line[0] != '\0') {
            char fields[3][300]; // admin_id,name,password
            int  count = splitCSV(line, fields, 3);
            if (count >= 3) {
                int aid = charToInt(fields[0]);
                // Admin constructor: id, name, contact (empty), password
                admin = Admin(aid, fields[1], "", fields[2]);
                loaded = true;
            }
        }
    }

    fclose(f);
    return loaded;
}

// ------------------------------------------------------------
// loadAppointments
// Format: appointment_id,patient_id,doctor_id,date,time_slot,status
// ------------------------------------------------------------
void FileHandler::loadAppointments(Storage<Appointment>& store) {
    FILE* f = fopen(APPOINTMENTS_FILE, "r");
    if (f == nullptr) return;

    char line[256];
    store.clear();

    while (fgets(line, 256, f) != nullptr) {
        if (line[0] == '\n' || line[0] == '\r' || line[0] == '\0') continue;

        char fields[6][300];
        int  count = splitCSV(line, fields, 6);
        if (count < 6) continue;

        int aid = charToInt(fields[0]);
        int pid = charToInt(fields[1]);
        int did = charToInt(fields[2]);
        // fields[3] = date, fields[4] = time_slot, fields[5] = status

        Appointment a(aid, pid, did, fields[3], fields[4], fields[5]);
        store.add(a);
    }

    fclose(f);
}

// ------------------------------------------------------------
// loadPrescriptions
// Format: prescription_id,appointment_id,patient_id,doctor_id,
//         date,medicines,notes
// ------------------------------------------------------------
void FileHandler::loadPrescriptions(Storage<Prescription>& store) {
    FILE* f = fopen(PRESCRIPTIONS_FILE, "r");
    if (f == nullptr) return;

    // Prescriptions can have long medicine/note strings — use larger buffer
    char line[1024];
    store.clear();

    while (fgets(line, 1024, f) != nullptr) {
        if (line[0] == '\n' || line[0] == '\r' || line[0] == '\0') continue;

        char fields[7][300];
        int  count = splitCSV(line, fields, 7);
        if (count < 7) continue;

        int prid = charToInt(fields[0]);
        int apid = charToInt(fields[1]);
        int ptid = charToInt(fields[2]);
        int drid = charToInt(fields[3]);
        // fields[4] = date, fields[5] = medicines, fields[6] = notes

        Prescription pr(prid, apid, ptid, drid,
            fields[4], fields[5], fields[6]);
        store.add(pr);
    }

    fclose(f);
}

// ------------------------------------------------------------
// loadBills
// Format: bill_id,patient_id,appointment_id,amount,status,date
// ------------------------------------------------------------
void FileHandler::loadBills(Storage<Bill>& store) {
    FILE* f = fopen(BILLS_FILE, "r");
    if (f == nullptr) return;

    char line[256];
    store.clear();

    while (fgets(line, 256, f) != nullptr) {
        if (line[0] == '\n' || line[0] == '\r' || line[0] == '\0') continue;

        char fields[6][300];
        int  count = splitCSV(line, fields, 6);
        if (count < 6) continue;

        int   bid = charToInt(fields[0]);
        int   pid = charToInt(fields[1]);
        int   apid = charToInt(fields[2]);
        float amount = charToFloat(fields[3]);
        // fields[4] = status, fields[5] = date

        Bill b(bid, pid, apid, amount, fields[4], fields[5]);
        store.add(b);
    }

    fclose(f);
}

// ============================================================
// APPEND FUNCTIONS
// Open file in append mode ("a"), write one CSV line, close.
// The OS flushes the write buffer when fclose() is called.
// ============================================================

void FileHandler::appendPatient(const Patient& p) {
    FILE* f = fopen(PATIENTS_FILE, "a");
    if (f == nullptr) return;

    char buf[512];
    p.toCSV(buf, 512);  // serialise to CSV
    fputs(buf, f);
    fputs("\n", f);     // line terminator
    fclose(f);
}

void FileHandler::appendDoctor(const Doctor& d) {
    FILE* f = fopen(DOCTORS_FILE, "a");
    if (f == nullptr) return;

    char buf[256];
    d.toCSV(buf, 256);
    fputs(buf, f);
    fputs("\n", f);
    fclose(f);
}

void FileHandler::appendAppointment(const Appointment& a) {
    FILE* f = fopen(APPOINTMENTS_FILE, "a");
    if (f == nullptr) return;

    char buf[256];
    a.toCSV(buf, 256);
    fputs(buf, f);
    fputs("\n", f);
    fclose(f);
}

void FileHandler::appendPrescription(const Prescription& pr) {
    FILE* f = fopen(PRESCRIPTIONS_FILE, "a");
    if (f == nullptr) return;

    // Prescriptions have long text fields — use a big buffer
    char buf[1200];
    pr.toCSV(buf, 1200);
    fputs(buf, f);
    fputs("\n", f);
    fclose(f);
}

void FileHandler::appendBill(const Bill& b) {
    FILE* f = fopen(BILLS_FILE, "a");
    if (f == nullptr) return;

    char buf[256];
    b.toCSV(buf, 256);
    fputs(buf, f);
    fputs("\n", f);
    fclose(f);
}

// ============================================================
// UPDATE FUNCTIONS
// Re-write the entire file from the in-memory Storage<T>.
// This is the simplest correct approach for small datasets.
// ============================================================

// ------------------------------------------------------------
// updatePatient — rewrites patients.txt from in-memory store
// ------------------------------------------------------------
void FileHandler::updatePatient(const Storage<Patient>& store) {
    FILE* f = fopen(PATIENTS_FILE, "w"); // "w" truncates the file
    if (f == nullptr) return;

    const Patient* arr = store.getAll();
    int n = store.size();
    for (int i = 0; i < n; i++) {
        char buf[512];
        arr[i].toCSV(buf, 512);
        fputs(buf, f);
        fputs("\n", f);
    }
    fclose(f);
}

// ------------------------------------------------------------
// updateAppointment — rewrites appointments.txt
// ------------------------------------------------------------
void FileHandler::updateAppointment(const Storage<Appointment>& store) {
    FILE* f = fopen(APPOINTMENTS_FILE, "w");
    if (f == nullptr) return;

    const Appointment* arr = store.getAll();
    int n = store.size();
    for (int i = 0; i < n; i++) {
        char buf[256];
        arr[i].toCSV(buf, 256);
        fputs(buf, f);
        fputs("\n", f);
    }
    fclose(f);
}

// ------------------------------------------------------------
// updateBill — rewrites bills.txt
// ------------------------------------------------------------
void FileHandler::updateBill(const Storage<Bill>& store) {
    FILE* f = fopen(BILLS_FILE, "w");
    if (f == nullptr) return;

    const Bill* arr = store.getAll();
    int n = store.size();
    for (int i = 0; i < n; i++) {
        char buf[256];
        arr[i].toCSV(buf, 256);
        fputs(buf, f);
        fputs("\n", f);
    }
    fclose(f);
}

// ============================================================
// DELETE FUNCTIONS
// Remove the record from in-memory store then rewrite file.
// ============================================================

void FileHandler::deletePatient(int patientId, Storage<Patient>& store) {
    store.removeById(patientId);
    updatePatient(store);
}

void FileHandler::deleteDoctor(int doctorId, Storage<Doctor>& store) {
    store.removeById(doctorId);
    // Rewrite doctors.txt
    FILE* f = fopen(DOCTORS_FILE, "w");
    if (f == nullptr) return;
    const Doctor* arr = store.getAll();
    int n = store.size();
    for (int i = 0; i < n; i++) {
        char buf[256];
        arr[i].toCSV(buf, 256);
        fputs(buf, f);
        fputs("\n", f);
    }
    fclose(f);
}

void FileHandler::deleteAppointmentsByPatient(int patientId,
    Storage<Appointment>& store) {
    // Remove all appointments belonging to this patient
    // We iterate backwards to avoid index skipping after removal
    for (int i = store.size() - 1; i >= 0; i--) {
        if (store.getAll()[i].getPatientId() == patientId) {
            store.removeById(store.getAll()[i].getId());
        }
    }
    updateAppointment(store);
}

void FileHandler::deletePrescriptionsByPatient(int patientId,
    Storage<Prescription>& store) {
    for (int i = store.size() - 1; i >= 0; i--) {
        if (store.getAll()[i].getPatientId() == patientId) {
            store.removeById(store.getAll()[i].getId());
        }
    }
    // Rewrite prescriptions.txt
    FILE* f = fopen(PRESCRIPTIONS_FILE, "w");
    if (f == nullptr) return;
    const Prescription* arr = store.getAll();
    int n = store.size();
    for (int i = 0; i < n; i++) {
        char buf[1200];
        arr[i].toCSV(buf, 1200);
        fputs(buf, f);
        fputs("\n", f);
    }
    fclose(f);
}

void FileHandler::deleteBillsByPatient(int patientId,
    Storage<Bill>& store) {
    for (int i = store.size() - 1; i >= 0; i--) {
        if (store.getAll()[i].getPatientId() == patientId) {
            store.removeById(store.getAll()[i].getId());
        }
    }
    updateBill(store);
}

// ============================================================
// SECURITY LOG
// ============================================================

// ------------------------------------------------------------
// logFailedLogin
// Appends one line to security_log.txt:
//   timestamp,role,entered_id,FAILED
// Timestamp is generated using time() + localtime() from <ctime>.
// Format: DD-MM-YYYY HH:MM:SS
// ------------------------------------------------------------
void FileHandler::logFailedLogin(const char* role, const char* enteredId) {
    FILE* f = fopen(SECURITY_LOG_FILE, "a");
    if (f == nullptr) return;

    // Build timestamp using DateTime::fromNow() — no struct tm here.
    // DateTime encapsulates all struct tm usage inside DateTime.cpp.
    DateTime now = DateTime::fromNow();

    // Get the full "DD-MM-YYYY HH:MM:SS" timestamp string from DateTime
    char buf[20];
    now.toTimestampString(buf); // writes 19 chars + null terminator

    // Write the log entry: timestamp,role,enteredId,FAILED
    fputs(buf, f);
    fputs(",", f);
    fputs(role, f);
    fputs(",", f);
    fputs(enteredId, f);
    fputs(",FAILED\n", f);
    fclose(f);
}

// ------------------------------------------------------------
// printSecurityLog
// Reads and prints security_log.txt line by line.
// ------------------------------------------------------------
void FileHandler::printSecurityLog() {
    FILE* f = fopen(SECURITY_LOG_FILE, "r");
    if (f == nullptr) {
        // File doesn't exist yet — no events logged
        return;
    }

    char line[256];
    bool hasContent = false;

    while (fgets(line, 256, f) != nullptr) {
        if (line[0] != '\n' && line[0] != '\0') {
            // Print each log line directly
            fputs(line, stdout);
            hasContent = true;
        }
    }

    if (!hasContent) {
        fputs("No security events logged.\n", stdout);
    }

    fclose(f);
}

// ============================================================
// DISCHARGE ARCHIVE
// Copies all records for a patient into discharged.txt in
// the same CSV format as the original files, then sections
// are separated by a blank line for readability.
// ============================================================
void FileHandler::archivePatient(const Patient& p,
    const Storage<Appointment>& appts,
    const Storage<Prescription>& prescriptions,
    const Storage<Bill>& bills)
{
    FILE* f = fopen(DISCHARGED_FILE, "a"); // append to archive
    if (f == nullptr) return;

    // --- Patient record ---
    char buf[512];
    p.toCSV(buf, 512);
    fputs(buf, f);
    fputs("\n", f);

    // --- Appointments for this patient ---
    const Appointment* aArr = appts.getAll();
    int an = appts.size();
    for (int i = 0; i < an; i++) {
        if (aArr[i].getPatientId() == p.getId()) {
            char abuf[256];
            aArr[i].toCSV(abuf, 256);
            fputs(abuf, f);
            fputs("\n", f);
        }
    }

    // --- Prescriptions for this patient ---
    const Prescription* prArr = prescriptions.getAll();
    int prn = prescriptions.size();
    for (int i = 0; i < prn; i++) {
        if (prArr[i].getPatientId() == p.getId()) {
            char pbuf[1200];
            prArr[i].toCSV(pbuf, 1200);
            fputs(pbuf, f);
            fputs("\n", f);
        }
    }

    // --- Bills for this patient ---
    const Bill* bArr = bills.getAll();
    int bn = bills.size();
    for (int i = 0; i < bn; i++) {
        if (bArr[i].getPatientId() == p.getId()) {
            char bbuf[256];
            bArr[i].toCSV(bbuf, 256);
            fputs(bbuf, f);
            fputs("\n", f);
        }
    }

    // Separator between discharged patient blocks
    fputs("---\n", f);
    fclose(f);
}