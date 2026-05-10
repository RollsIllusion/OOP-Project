// ============================================================
// main.cpp
// Entry point for the MediCore Hospital Management System.
//
// Responsibilities:
//   1. Declare all in-memory Storage<T> containers.
//   2. Load all data from .txt files via FileHandler (startup).
//   3. Call Menu::run() — the entire program runs from there.
//   4. Return 0 when Menu::run() exits (user chose "Exit").
//
// Rules enforced here:
//   - main() is a sequence of function calls only (per spec).
//   - No global variables (all stores are local to main).
//   - No hardcoded data (everything loaded from .txt files).
//   - Dynamic allocation not needed at this level; Storage<T>
//     manages its own fixed-size internal arrays on the stack.
// ============================================================

#include "Storage.h"
#include "Patient.h"
#include "Doctor.h"
#include "Admin.h"
#include "Appointment.h"
#include "Bill.h"
#include "Prescription.h"
#include "FileHandler.h"
#include "Menu.h"
#include "FileNotFoundException.h"
#include <iostream>

int main() {
    // ----------------------------------------------------------
    // Declare in-memory stores for each entity type.
    // Storage<T> uses a fixed internal array of 100 elements;
    // no dynamic allocation and no std::vector here.
    // ----------------------------------------------------------
    Storage<Patient>      patients;
    Storage<Doctor>       doctors;
    Storage<Appointment>  appointments;
    Storage<Bill>         bills;
    Storage<Prescription> prescriptions;

    // Single admin object — loaded directly (not stored in Storage<>
    // because there is only ever ONE admin record).
    Admin admin;

    // ----------------------------------------------------------
    // STARTUP: load all data from .txt files.
    // FileHandler throws FileNotFoundException only for critical
    // files that truly cannot be missing; others silently start
    // empty on first run (patients, doctors, appointments, etc.)
    // ----------------------------------------------------------
    try {
        FileHandler::loadPatients(patients);
        FileHandler::loadDoctors(doctors);
        FileHandler::loadAdmin(admin);       // returns bool; ignore return value
        FileHandler::loadAppointments(appointments);
        FileHandler::loadPrescriptions(prescriptions);
        FileHandler::loadBills(bills);
    }
    catch (const FileNotFoundException& e) {
        // A critical file (e.g. admin.txt) could not be opened.
        // Print the error and exit — the system cannot function
        // without its core data files.
        std::cout << "\n[STARTUP ERROR] " << e.what() << "\n";
        std::cout << "Please ensure all required .txt files are present.\n";
        return 1;
    }

    // ----------------------------------------------------------
    // Run the application.
    // Menu::run() contains the main event loop; it returns only
    // when the user selects "Exit" from the role-selection screen.
    // All menus, sub-menus, and actions are dispatched from here.
    // ----------------------------------------------------------
    Menu::run(patients, doctors, appointments, bills, prescriptions, admin);

    // ----------------------------------------------------------
    // All data has been written immediately on every change
    // (FileHandler always flushes after each write), so there
    // is nothing extra to save on exit.
    // ----------------------------------------------------------
    return 0;
}