#pragma once
#ifndef MENU_H
#define MENU_H

// ============================================================
// Menu.h
// Manages all interactive menus in the MediCore system.
// All methods are static — called directly from main().
//
// The Menu class is the "controller" layer:
//   - It calls Validator for input checking
//   - It calls FileHandler for persistence
//   - It calls Storage<T> to access/modify in-memory data
//   - It throws and catches exceptions defined in the project
//
// Data is passed in by pointer/reference so Menu can modify
// the live in-memory stores (no global variables).
//
// All user input is read through Utility::readLine / readInt /
// readFloat — never directly with cin >> or scanf.
// ============================================================

#include "Storage.h"
#include "Patient.h"
#include "Doctor.h"
#include "Admin.h"
#include "Appointment.h"
#include "Bill.h"
#include "Prescription.h"

class Menu {
public:
    // ==========================================================
    // TOP-LEVEL ENTRY POINT
    // Called once from main() — shows the role-selection screen
    // and dispatches to the correct login/menu flow.
    // ==========================================================
    static void run(Storage<Patient>& patients,
        Storage<Doctor>& doctors,
        Storage<Appointment>& appointments,
        Storage<Bill>& bills,
        Storage<Prescription>& prescriptions,
        Admin& admin);

    // ==========================================================
    // LOGIN FLOWS
    // Each returns a pointer to the logged-in entity (from the
    // corresponding store) or nullptr on failure / lockout.
    // After 3 consecutive failures, the session is locked and
    // the event is written to security_log.txt.
    // ==========================================================

    static Patient* loginPatient(Storage<Patient>& patients);

    static Doctor* loginDoctor(Storage<Doctor>& doctors);

    // Admin login returns bool — there is only one admin record.
    static bool     loginAdmin(const Admin& admin);

    // ==========================================================
    // PATIENT MENU & SUB-ACTIONS
    // ==========================================================

    // Displays the patient main menu and routes to sub-actions.
    static void patientMenu(Patient& patient,
        Storage<Doctor>& doctors,
        Storage<Appointment>& appointments,
        Storage<Bill>& bills,
        Storage<Prescription>& prescriptions,
        Storage<Patient>& patients);

    // 1. Book an appointment
    static void bookAppointment(Patient& patient,
        Storage<Doctor>& doctors,
        Storage<Appointment>& appointments,
        Storage<Bill>& bills,
        Storage<Patient>& patients);

    // 2. Cancel an appointment
    static void cancelAppointment(Patient& patient,
        Storage<Doctor>& doctors,
        Storage<Appointment>& appointments,
        Storage<Bill>& bills,
        Storage<Patient>& patients);

    // 3. View all appointments for this patient (sorted ascending)
    static void viewMyAppointments(const Patient& patient,
        const Storage<Doctor>& doctors,
        const Storage<Appointment>& appointments);

    // 4. View medical records (prescriptions) sorted date descending
    static void viewMyMedicalRecords(const Patient& patient,
        const Storage<Doctor>& doctors,
        const Storage<Appointment>& appointments,
        const Storage<Prescription>& prescriptions);

    // 5. View all bills for this patient
    static void viewMyBills(const Patient& patient,
        const Storage<Bill>& bills);

    // 6. Pay a bill
    static void payBill(Patient& patient,
        Storage<Bill>& bills,
        Storage<Patient>& patients);

    // 7. Top up wallet balance
    static void topUpBalance(Patient& patient,
        Storage<Patient>& patients);

    // ==========================================================
    // DOCTOR MENU & SUB-ACTIONS
    // ==========================================================

    // Displays the doctor main menu and routes to sub-actions.
    static void doctorMenu(Doctor& doctor,
        Storage<Patient>& patients,
        Storage<Appointment>& appointments,
        Storage<Bill>& bills,
        Storage<Prescription>& prescriptions);

    // 1. View today's appointments
    static void viewTodaysAppointments(const Doctor& doctor,
        const Storage<Patient>& patients,
        const Storage<Appointment>& appointments);

    // 2. Mark an appointment as completed
    static void markCompleted(const Doctor& doctor,
        Storage<Appointment>& appointments);

    // 3. Mark an appointment as no-show
    static void markNoShow(const Doctor& doctor,
        Storage<Appointment>& appointments,
        Storage<Bill>& bills);

    // 4. Write a prescription for a completed appointment
    static void writePrescription(const Doctor& doctor,
        Storage<Appointment>& appointments,
        Storage<Prescription>& prescriptions);

    // 5. View a specific patient's medical history (access-controlled)
    static void viewPatientHistory(const Doctor& doctor,
        const Storage<Patient>& patients,
        const Storage<Appointment>& appointments,
        const Storage<Prescription>& prescriptions);

    // ==========================================================
    // ADMIN MENU & SUB-ACTIONS
    // ==========================================================

    // Displays the admin main menu and routes to sub-actions.
    static void adminMenu(Storage<Patient>& patients,
        Storage<Doctor>& doctors,
        Storage<Appointment>& appointments,
        Storage<Bill>& bills,
        Storage<Prescription>& prescriptions);

    // 1. Add a new doctor
    static void addDoctor(Storage<Doctor>& doctors);

    // 2. Remove a doctor (blocked if they have pending appointments)
    static void removeDoctor(Storage<Doctor>& doctors,
        const Storage<Appointment>& appointments);

    // 3. View all patients with unpaid bill count
    static void viewAllPatients(const Storage<Patient>& patients,
        const Storage<Bill>& bills);

    // 4. View all doctors
    static void viewAllDoctors(const Storage<Doctor>& doctors);

    // 5. View all appointments sorted date descending
    static void viewAllAppointments(const Storage<Appointment>& appointments,
        const Storage<Patient>& patients,
        const Storage<Doctor>& doctors);

    // 6. View all unpaid bills (with OVERDUE flag for >7 days old)
    static void viewUnpaidBills(const Storage<Bill>& bills,
        const Storage<Patient>& patients);

    // 7. Discharge a patient (archives and removes all records)
    static void dischargePatient(Storage<Patient>& patients,
        Storage<Appointment>& appointments,
        Storage<Prescription>& prescriptions,
        Storage<Bill>& bills);

    // 8. View security log
    static void viewSecurityLog();

    // 9. Generate daily report (no separate file — derived from existing files)
    static void generateDailyReport(const Storage<Appointment>& appointments,
        const Storage<Bill>& bills,
        const Storage<Patient>& patients,
        const Storage<Doctor>& doctors);

private:
    // ----------------------------------------------------------
    // Private helpers shared across menu actions
    // ----------------------------------------------------------

    // Displays available time slots for doctorId on date,
    // excluding those already booked (non-cancelled appointments).
    static void displayAvailableSlots(int                         doctorId,
        const char* date,
        const Storage<Appointment>& appointments);

    // Checks if the given slot is already taken for the doctor/date.
    // Returns true if unavailable.
    static bool isSlotTaken(int                         doctorId,
        const char* date,
        const char* slot,
        const Storage<Appointment>& appointments);

    // Prints a formatted date comparison to detect OVERDUE bills
    // (bill date is more than 7 days before today).
    static bool isBillOverdue(const char* billDate, const char* todayDate);
};

#endif // MENU_H