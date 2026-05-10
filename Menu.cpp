
// Implements the entire interactive UI of the MediCore system.
//
// Architecture:
//
// 
// All file writes go through FileHandler.
// All validation goes through Validator.
// Exceptions are thrown and caught at the action level so the
// menu loop always remains in control.

#include "Menu.h"
#include "Validator.h"
#include "Utility.h"
#include "FileHandler.h"
#include "InsufficientFundsException.h"
#include "InvalidInputException.h"
#include "SlotUnavailableException.h"
#include <iostream>  // std::cout, std::cin

// 
// Helper macro — print a separator line of fixed width
// 
#define SEP Utility::printLine(50)

// 
// run
// Top-level loop. Shows role-selection screen and dispatches.
// Loops until the user selects "Exit".
// 
void Menu::run(Storage<Patient>& patients,
    Storage<Doctor>& doctors,
    Storage<Appointment>& appointments,
    Storage<Bill>& bills,
    Storage<Prescription>& prescriptions,
    Admin& admin)
{
    int choice = 0;

    while (true) {
        // 
        // Display the welcome banner and role-selection menu
        // 
        std::cout << "\n";
        std::cout << " Welcome to MediCore Hospital Management System\n";
        std::cout << " Login as:\n";
        std::cout << "  1. Patient\n";
        std::cout << "  2. Doctor\n";
        std::cout << "  3. Admin\n";
        std::cout << "  4. Exit\n";
        std::cout << " Enter choice: ";

        choice = Utility::readInt();

        if (!Validator::validateMenuChoice(choice, 1, 4)) {
            std::cout << " Invalid choice. Please enter 1-4.\n";
            continue;
        }

        if (choice == 4) {
            // User wants to exit — say goodbye and break the loop
            std::cout << "\n Thank you for using MediCore. Goodbye!\n";
            break;
        }

        if (choice == 1) {
            // 
            // Patient login flow
            // 
            Patient* p = loginPatient(patients);
            if (p != nullptr) {
                patientMenu(*p, doctors, appointments, bills,
                    prescriptions, patients);
            }

        }
        else if (choice == 2) {
            // 
            // Doctor login flow
            // 
            Doctor* d = loginDoctor(doctors);
            if (d != nullptr) {
                doctorMenu(*d, patients, appointments, bills, prescriptions);
            }

        }
        else if (choice == 3) {
            // 
            // Admin login flow
            // 
            if (loginAdmin(admin)) {
                adminMenu(patients, doctors, appointments, bills, prescriptions);
            }
        }
    }
}

// 
// loginPatient
// Prompts for patient ID and password.
// Allows up to 3 consecutive failed attempts.
// On lockout: logs to security_log.txt and prints a message.
// Returns pointer to the Patient in the store, or nullptr.
// 
Patient* Menu::loginPatient(Storage<Patient>& patients) {
    int  attempts = 0;   // failed attempt counter
    char idBuf[20];      // raw input for patient ID
    char pwBuf[50];      // raw input for password

    while (attempts < 3) {
        std::cout << "\n Patient Login.....) \n";
        std::cout << " Patient ID: ";
        Utility::readLine(idBuf, 20);

        std::cout << " Password  : ";
        Utility::readLine(pwBuf, 50);

        // Convert the entered ID string to integer
        int enteredId = Utility::strToInt(idBuf);

        // Look for a patient with this ID in the in-memory store
        Patient* found = patients.findById(enteredId);

        if (found != nullptr && found->checkPassword(pwBuf)) {
            // Credentials are valid — login successful
            std::cout << " Login successful. Welcome, "
                << found->getName() << "!\n";
            return found;
        }

        // Credentials did not match
        attempts++;
        std::cout << " Invalid ID or password. Attempts remaining: "
            << (3 - attempts) << "\n";

        // Log this failed attempt to security_log.txt
        FileHandler::logFailedLogin("Patient", idBuf);
    }

    // Three consecutive failures — lock the session
    std::cout << " Account locked. Contact admin.\n";
    return nullptr;
}

// 
// loginDoctor
// Same logic as loginPatient but searches doctors store.
// 
Doctor* Menu::loginDoctor(Storage<Doctor>& doctors) {
    int  attempts = 0;
    char idBuf[20];
    char pwBuf[50];

    while (attempts < 3) {
        std::cout << "\nDoctor Login........)\n";
        std::cout << " Doctor ID: ";
        Utility::readLine(idBuf, 20);

        std::cout << " Password : ";
        Utility::readLine(pwBuf, 50);

        int enteredId = Utility::strToInt(idBuf);
        Doctor* found = doctors.findById(enteredId);

        if (found != nullptr && found->checkPassword(pwBuf)) {
            std::cout << " Login successful. Welcome, Dr. "
                << found->getName() << "!\n";
            return found;
        }

        attempts++;
        std::cout << " Invalid ID or password. Attempts remaining: "
            << (3 - attempts) << "\n";
        FileHandler::logFailedLogin("Doctor", idBuf);
    }

    std::cout << " Account locked. Contact admin.\n";
    return nullptr;
}

// 
// loginAdmin
// Validates against the single Admin record loaded from admin.txt.
// Returns true on success, false after 3 failures.
// 
bool Menu::loginAdmin(const Admin& admin) {
    int  attempts = 0;
    char pwBuf[50];
    char idBuf[20];

    while (attempts < 3) {
        std::cout << "\nAdmin Login ...........)\n";
        std::cout << " Admin ID : ";
        Utility::readLine(idBuf, 20);

        std::cout << " Password : ";
        Utility::readLine(pwBuf, 50);

        int enteredId = Utility::strToInt(idBuf);

        if (enteredId == admin.getId() && admin.checkPassword(pwBuf)) {
            std::cout << " Login successful. Welcome, Admin!\n";
            return true;
        }

        attempts++;
        std::cout << " Invalid credentials. Attempts remaining: "
            << (3 - attempts) << "\n";
        FileHandler::logFailedLogin("Admin", idBuf);
    }

    std::cout << " Account locked. Contact admin.\n";
    return false;
}

// 
// 
//  PATIENT MENU
// 
// 

// 
// patientMenu
// Displays the patient dashboard and dispatches to sub-actions.
// Loops until the patient selects Logout (8).
// 
void Menu::patientMenu(Patient& patient,
    Storage<Doctor>& doctors,
    Storage<Appointment>& appointments,
    Storage<Bill>& bills,
    Storage<Prescription>& prescriptions,
    Storage<Patient>& patients)
{
    int choice = 0;

    while (true) {
        // Display header with current balance
        std::cout << "\n";
        SEP;
        std::cout << " Welcome, " << patient.getName() << "\n";

        // Show balance — convert float to string manually
        char balBuf[20];
        Utility::floatToStr(patient.getBalance(), balBuf, 2);
        std::cout << " Balance: PKR " << balBuf << "\n";
        SEP;
        std::cout << "  1. Book Appointment\n";
        std::cout << "  2. Cancel Appointment\n";
        std::cout << "  3. View My Appointments\n";
        std::cout << "  4. View My Medical Records\n";
        std::cout << "  5. View My Bills\n";
        std::cout << "  6. Pay Bill\n";
        std::cout << "  7. Top Up Balance\n";
        std::cout << "  8. Logout\n";
        SEP;
        std::cout << " Enter choice: ";

        choice = Utility::readInt();

        if (!Validator::validateMenuChoice(choice, 1, 8)) {
            std::cout << " Invalid choice. Enter 1-8.\n";
            continue;
        }

        if (choice == 8) {
            std::cout << " Logged out successfully.\n";
            return;
        }

        // Dispatch to the correct sub-action
        switch (choice) {
        case 1: bookAppointment(patient, doctors, appointments,
            bills, patients);             break;
        case 2: cancelAppointment(patient, doctors, appointments,
            bills, patients);           break;
        case 3: viewMyAppointments(patient, doctors,
            appointments);             break;
        case 4: viewMyMedicalRecords(patient, doctors,
            appointments,
            prescriptions);          break;
        case 5: viewMyBills(patient, bills);                  break;
        case 6: payBill(patient, bills, patients);            break;
        case 7: topUpBalance(patient, patients);              break;
        }
    }
}

// 
// bookAppointment
// Patient menu option 1.
//
// 
void Menu::bookAppointment(Patient& patient,
    Storage<Doctor>& doctors,
    Storage<Appointment>& appointments,
    Storage<Bill>& bills,
    Storage<Patient>& patients)
{
    SEP;
    std::cout << " == Book Appointment ==\n";

    // 
    // Step 1: Ask for specialization and display matching doctors
    // 
    char spec[50];
    std::cout << " Enter specialization to search (e.g. Cardiology): ";
    Utility::readLine(spec, 50);

    // Find all doctors whose specialization matches case-insensitively
    bool foundAny = false;
    const Doctor* docArr = doctors.getAll();
    int  docCount = doctors.size();

    for (int i = 0; i < docCount; i++) {
        if (Validator::specializationMatch(docArr[i].getSpecialization(), spec)) {
            // Print: ID | Name | Fee
            char feeBuf[20];
            Utility::floatToStr(docArr[i].getFee(), feeBuf, 2);
            std::cout << "  ID: " << docArr[i].getId()
                << " | " << docArr[i].getName()
                << " | Fee: PKR " << feeBuf << "\n";
            foundAny = true;
        }
    }

    if (!foundAny) {
        std::cout << " No doctors available for that specialization.\n";
        return;
    }

    // 
    // Step 2: Ask for Doctor ID and validate
    // 
    std::cout << " Enter Doctor ID: ";
    int docId = Utility::readInt();

    Doctor* selectedDoc = doctors.findById(docId);
    if (selectedDoc == nullptr) {
        std::cout << " Doctor not found.\n";
        return;
    }

    // Make sure the doctor's specialization matches what was searched
    if (!Validator::specializationMatch(selectedDoc->getSpecialization(), spec)) {
        std::cout << " Doctor not found.\n";
        return;
    }

    // 
    // Step 3: Ask for date with up to 3 re-tries
    // 
    char date[12];
    int  dateAttempts = 0;
    bool validDate = false;
    int  currentYear = Utility::getCurrentYear();

    while (dateAttempts < 3) {
        std::cout << " Enter date (DD-MM-YYYY): ";
        Utility::readLine(date, 12);

        if (Validator::validateDate(date, currentYear)) {
            validDate = true;
            break;
        }

        dateAttempts++;
        std::cout << " Invalid date. Use format DD-MM-YYYY.\n";
    }

    if (!validDate) {
        std::cout << " Too many invalid date attempts. Returning to menu.\n";
        return;
    }

    // 
    // Step 4: Display available slots and ask patient to choose
    // 
    displayAvailableSlots(docId, date, appointments);

    char slot[6];
    bool slotOk = false;

    // Allow repeated attempts until a valid, available slot is chosen
    while (!slotOk) {
        std::cout << " Enter time slot (e.g. 09:00): ";
        Utility::readLine(slot, 6);

        // Validate format first
        if (!Validator::validateTimeSlot(slot)) {
            std::cout << " Invalid time slot. Must be one of the 8 daily slots.\n";
            continue;
        }

        // Check availability — throw SlotUnavailableException if taken
        try {
            if (isSlotTaken(docId, date, slot, appointments)) {
                throw SlotUnavailableException(slot);
            }
            slotOk = true;  // slot is valid and available
        }
        catch (const SlotUnavailableException& e) {
            std::cout << " " << e.what() << "\n";
            // Re-display available slots so the patient can choose again
            displayAvailableSlots(docId, date, appointments);
        }
    }

    // 
    // Step 5: Check patient balance >= doctor fee
    // 
    float fee = selectedDoc->getFee();

    try {
        if (patient.getBalance() < fee) {
            throw InsufficientFundsException(fee, patient.getBalance());
        }
    }
    catch (const InsufficientFundsException& e) {
        std::cout << " " << e.what() << "\n";
        return;
    }

    // 
    // Step 6: Deduct fee, generate IDs, create appointment + bill
    // 

    // Deduct fee from patient balance using the overloaded -= operator
    patient -= fee;

    // Generate new appointment ID: max existing ID + 1
    int newApptId = appointments.getMaxId() + 1;

    // Create the Appointment object and add to in-memory store
    Appointment newAppt(newApptId, patient.getId(), docId,
        date, slot, "pending");
    appointments.add(newAppt);

    // Persist the new appointment to appointments.txt
    FileHandler::appendAppointment(newAppt);

    // Generate new bill ID
    int newBillId = bills.getMaxId() + 1;

    // Create a Bill for this appointment — status "unpaid"
    // (The project says a bill is created when booking; note:
    //  the patient's balance is ALREADY deducted at booking time
    //  per the spec, but the bill still starts as "unpaid" so
    //  the patient can later "Pay Bill" to finalise it.)
    Bill newBill(newBillId, patient.getId(), newApptId,
        fee, "unpaid", date);
    bills.add(newBill);

    // Persist the bill to bills.txt
    FileHandler::appendBill(newBill);

    // Persist the updated patient balance to patients.txt
    FileHandler::updatePatient(patients);

    // Success message
    char apptIdBuf[12];
    Utility::intToStr(newApptId, apptIdBuf);
    std::cout << " Appointment booked successfully. Appointment ID: "
        << apptIdBuf << "\n";
}

// 
// cancelAppointment
// Patient menu option 2.
// Lists the patient's pending appointments, then cancels one.
// Refunds the doctor's fee using the += operator.
// 
void Menu::cancelAppointment(Patient& patient,
    Storage<Doctor>& doctors,
    Storage<Appointment>& appointments,
    Storage<Bill>& bills,
    Storage<Patient>& patients)
{
    SEP;
    std::cout << " Cancel Appointment\n";

    // Build a filtered list of this patient's pending appointments
    Appointment* pending[100];
    int          pendingCount = 0;

    Appointment* apptArr = appointments.getAll();
    int          apptN = appointments.size();

    for (int i = 0; i < apptN; i++) {
        if (apptArr[i].getPatientId() == patient.getId()) {
            // Check status == "pending" manually
            if (Validator::charsEqual(apptArr[i].getStatus(), "pending")) {
                pending[pendingCount++] = &apptArr[i];
            }
        }
    }

    if (pendingCount == 0) {
        std::cout << " You have no pending appointments.\n";
        return;
    }

    // Display the pending appointments to the patient
    std::cout << " Your pending appointments:\n";
    std::cout << " ID  | Doctor Name              | Date       | Slot\n";
    SEP;
    for (int i = 0; i < pendingCount; i++) {
        // Look up doctor name
        Doctor* doc = doctors.findById(pending[i]->getDoctorId());
        const char* docName = (doc != nullptr) ? doc->getName() : "Unknown";

        std::cout << "  " << pending[i]->getAppointmentId()
            << " | " << docName
            << " | " << pending[i]->getDate()
            << " | " << pending[i]->getTimeSlot() << "\n";
    }

    // Ask the patient which appointment to cancel
    std::cout << " Enter Appointment ID to cancel: ";
    int cancelId = Utility::readInt();

    // Validate: must belong to this patient and be pending
    Appointment* toCancel = nullptr;
    for (int i = 0; i < pendingCount; i++) {
        if (pending[i]->getAppointmentId() == cancelId) {
            toCancel = pending[i];
            break;
        }
    }

    if (toCancel == nullptr) {
        std::cout << " Invalid appointment ID.\n";
        return;
    }

    // Find the corresponding bill (same appointment ID)
    Bill* relatedBill = nullptr;
    Bill* billArr = bills.getAll();
    int   billN = bills.size();
    for (int i = 0; i < billN; i++) {
        if (billArr[i].getAppointmentId() == cancelId &&
            billArr[i].getPatientId() == patient.getId()) {
            relatedBill = &billArr[i];
            break;
        }
    }

    // Get doctor fee for refund
    float refundAmount = 0.0f;
    Doctor* doc = doctors.findById(toCancel->getDoctorId());
    if (doc != nullptr) {
        refundAmount = doc->getFee();
    }

    // Update appointment status to "cancelled" in memory
    toCancel->setStatus("cancelled");

    // Persist updated appointments to appointments.txt
    FileHandler::updateAppointment(appointments);

    // Refund the fee to patient using the overloaded += operator
    patient += refundAmount;

    // Update the bill status to "cancelled" in memory
    if (relatedBill != nullptr) {
        relatedBill->setStatus("cancelled");
        FileHandler::updateBill(bills);
    }

    // Persist updated patient balance to patients.txt
    FileHandler::updatePatient(patients);

    // Print confirmation
    char feeBuf[20];
    Utility::floatToStr(refundAmount, feeBuf, 2);
    std::cout << " Appointment cancelled. PKR " << feeBuf
        << " refunded to your balance.\n";
}

// 
// viewMyAppointments
// Patient menu option 3.
// Displays all appointments sorted by date ascending.
// 
void Menu::viewMyAppointments(const Patient& patient,
    const Storage<Doctor>& doctors,
    const Storage<Appointment>& appointments)
{
    std::cout << " == My Appointments ==\n";

    // Collect all appointments for this patient into a pointer array
    // so we can sort without modifying the store.
    Appointment* filtered[100];
    int          count = 0;

    // We need a non-const pointer to sort — cast is safe here because
    // we only reorder the pointer array, never modify the objects.
    Appointment* apptArr = const_cast<Appointment*>(appointments.getAll());
    int          apptN = appointments.size();

    for (int i = 0; i < apptN; i++) {
        if (apptArr[i].getPatientId() == patient.getId()) {
            filtered[count++] = &apptArr[i];
        }
    }

    if (count == 0) {
        std::cout << " No appointments found.\n";
        return;
    }

    // Sort by date ascending (insertion sort inside Utility)
    Utility::sortAppointmentsAsc(filtered, count);

    // Print header
    std::cout << " ID  | Doctor                   | Spec       "
        << "| Date       | Slot  | Status\n";

    for (int i = 0; i < count; i++) {
        // Look up doctor details
        const Doctor* doc = doctors.findById(filtered[i]->getDoctorId());
        const char* docName = (doc != nullptr) ? doc->getName() : "Unknown";
        const char* docSpec = (doc != nullptr) ? doc->getSpecialization() : "-";

        std::cout << "  " << filtered[i]->getAppointmentId()
            << " | " << docName
            << " | " << docSpec
            << " | " << filtered[i]->getDate()
            << " | " << filtered[i]->getTimeSlot()
            << " | " << filtered[i]->getStatus() << "\n";
    }
}

// 
// viewMyMedicalRecords
// Patient menu option 4.
// Displays prescriptions grouped by appointment, date descending.
// 
void Menu::viewMyMedicalRecords(const Patient& patient,
    const Storage<Doctor>& doctors,
    const Storage<Appointment>& appointments,
    const Storage<Prescription>& prescriptions)
{
    SEP;
    std::cout << " My Medical Records......)\n";

    // Collect prescriptions for this patient
    Prescription* filtered[100];
    int           count = 0;

    Prescription* prArr = const_cast<Prescription*>(prescriptions.getAll());
    int           prN = prescriptions.size();

    for (int i = 0; i < prN; i++) {
        if (prArr[i].getPatientId() == patient.getId()) {
            filtered[count++] = &prArr[i];
        }
    }

    if (count == 0) {
        std::cout << " No medical records found.\n";
        return;
    }

    // Sort by date descending (most recent first)
    Utility::sortPrescriptionsDesc(filtered, count);

    for (int i = 0; i < count; i++) {
        // Look up the appointment to get the date
        const Appointment* appt =
            appointments.findById(filtered[i]->getAppointmentId());
        const char* apptDate = (appt != nullptr) ? appt->getDate() : filtered[i]->getDate();

        // Look up doctor name
        const Doctor* doc = doctors.findById(filtered[i]->getDoctorId());
        const char* docName = (doc != nullptr) ? doc->getName() : "Unknown";

        std::cout << " Date      : " << apptDate << "\n";
        std::cout << " Doctor    : " << docName << "\n";
        std::cout << " Medicines : " << filtered[i]->getMedicines() << "\n";
        std::cout << " Notes     : " << filtered[i]->getNotes() << "\n";
    }
}

// 
// viewMyBills
// Patient menu option 5.
// Displays all bills and total outstanding unpaid amount.
// 
void Menu::viewMyBills(const Patient& patient,
    const Storage<Bill>& bills)
{

    std::cout << " == My Bills ==\n";

    const Bill* billArr = bills.getAll();
    int         billN = bills.size();
    bool        found = false;
    float       totalUnpaid = 0.0f;

    std::cout << " BillID | ApptID | Amount (PKR) | Status    | Date\n";


    for (int i = 0; i < billN; i++) {
        if (billArr[i].getPatientId() == patient.getId()) {
            found = true;

            char amtBuf[20];
            Utility::floatToStr(billArr[i].getAmount(), amtBuf, 2);

            std::cout << "  " << billArr[i].getBillId()
                << "       | " << billArr[i].getAppointmentId()
                << "      | PKR " << amtBuf
                << " | " << billArr[i].getStatus()
                << " | " << billArr[i].getDate() << "\n";

            // Sum up unpaid bills
            if (Validator::charsEqual(billArr[i].getStatus(), "unpaid")) {
                totalUnpaid += billArr[i].getAmount();
            }
        }
    }

    if (!found) {
        std::cout << " No bills found.\n";
        return;
    }

    // Show total outstanding amount
    char totalBuf[20];
    Utility::floatToStr(totalUnpaid, totalBuf, 2);
    SEP;
    std::cout << " Total outstanding (unpaid): PKR " << totalBuf << "\n";
}

// 
// payBill
// Patient menu option 6.
// Lets the patient settle an unpaid bill using their balance.
// 
void Menu::payBill(Patient& patient,
    Storage<Bill>& bills,
    Storage<Patient>& patients)
{

    std::cout << "  Pay Bill ........)\n";

    // Collect unpaid bills for this patient
    Bill* unpaid[100];
    int   unpaidCount = 0;

    Bill* billArr = bills.getAll();
    int   billN = bills.size();

    for (int i = 0; i < billN; i++) {
        if (billArr[i].getPatientId() == patient.getId() &&
            Validator::charsEqual(billArr[i].getStatus(), "unpaid")) {
            unpaid[unpaidCount++] = &billArr[i];
        }
    }

    if (unpaidCount == 0) {
        std::cout << " No unpaid bills.\n";
        return;
    }

    // Display unpaid bills
    std::cout << " Your unpaid bills:\n";
    std::cout << " BillID | ApptID | Amount (PKR) | Date\n";

    for (int i = 0; i < unpaidCount; i++) {
        char amtBuf[20];
        Utility::floatToStr(unpaid[i]->getAmount(), amtBuf, 2);
        std::cout << "  " << unpaid[i]->getBillId()
            << "       | " << unpaid[i]->getAppointmentId()
            << "      | PKR " << amtBuf
            << " | " << unpaid[i]->getDate() << "\n";
    }

    // Ask which bill to pay
    std::cout << " Enter Bill ID to pay: ";
    int billId = Utility::readInt();

    // Validate: must belong to this patient and be unpaid
    Bill* toPay = nullptr;
    for (int i = 0; i < unpaidCount; i++) {
        if (unpaid[i]->getBillId() == billId) {
            toPay = unpaid[i];
            break;
        }
    }

    if (toPay == nullptr) {
        std::cout << " Invalid bill ID.\n";
        return;
    }

    // Check sufficient balance — throw InsufficientFundsException if not
    try {
        if (patient.getBalance() < toPay->getAmount()) {
            throw InsufficientFundsException(toPay->getAmount(),
                patient.getBalance());
        }
    }
    catch (const InsufficientFundsException& e) {
        std::cout << " " << e.what() << "\n";
        return;
    }

    // Deduct bill amount from patient balance using -= operator
    patient -= toPay->getAmount();

    // Update bill status to "paid"
    toPay->setStatus("paid");
    FileHandler::updateBill(bills);

    // Persist updated patient balance
    FileHandler::updatePatient(patients);

    // Print confirmation with remaining balance
    char balBuf[20];
    Utility::floatToStr(patient.getBalance(), balBuf, 2);
    std::cout << " Bill paid successfully. Remaining balance: PKR "
        << balBuf << "\n";
}

// 
// topUpBalance
// Patient menu option 7.
// Adds a positive amount to the patient's wallet balance.
// Up to 3 attempts for valid input before returning to menu.
// 
void Menu::topUpBalance(Patient& patient,
    Storage<Patient>& patients)
{
    SEP;
    std::cout << "  Top Up Balance ..)\n";

    int   attempt = 0;
    float amount = 0.0f;

    while (attempt < 3) {
        std::cout << " Enter amount to add (PKR): ";
        amount = Utility::readFloat();

        try {
            if (!Validator::validatePositiveFloat(amount)) {
                throw InvalidInputException(
                    "Amount must be a positive number greater than 0.");
            }
            break; // valid input — exit loop
        }
        catch (const InvalidInputException& e) {
            std::cout << " " << e.what() << "\n";
            attempt++;
        }
    }

    if (attempt >= 3) {
        std::cout << " Too many invalid attempts. Returning to menu.\n";
        return;
    }

    // Add amount to patient balance using the += operator
    patient += amount;

    // Persist the updated balance to patients.txt
    FileHandler::updatePatient(patients);

    // Print confirmation
    char balBuf[20];
    Utility::floatToStr(patient.getBalance(), balBuf, 2);
    std::cout << " Balance updated. New balance: PKR " << balBuf << "\n";
}

// 
// 
//  DOCTOR MENU
// 
// 

// 
// doctorMenu
// Displays the doctor dashboard and dispatches sub-actions.
// 
void Menu::doctorMenu(Doctor& doctor,
    Storage<Patient>& patients,
    Storage<Appointment>& appointments,
    Storage<Bill>& bills,
    Storage<Prescription>& prescriptions)
{
    int choice = 0;

    while (true) {
        std::cout << "\n";
        SEP;
        std::cout << " Welcome, Dr. " << doctor.getName()
            << " | Specialization: " << doctor.getSpecialization() << "\n";
        SEP;
        std::cout << "  1. View Today's Appointments\n";
        std::cout << "  2. Mark Appointment Complete\n";
        std::cout << "  3. Mark Appointment No-Show\n";
        std::cout << "  4. Write Prescription\n";
        std::cout << "  5. View Patient Medical History\n";
        std::cout << "  6. Logout\n";
        SEP;
        std::cout << " Enter choice: ";

        choice = Utility::readInt();

        if (!Validator::validateMenuChoice(choice, 1, 6)) {
            std::cout << " Invalid choice. Enter 1-6.\n";
            continue;
        }

        if (choice == 6) {
            std::cout << " Logged out.\n";
            return;
        }

        switch (choice) {
        case 1: viewTodaysAppointments(doctor, patients, appointments); break;
        case 2: markCompleted(doctor, appointments);                    break;
        case 3: markNoShow(doctor, appointments, bills);               break;
        case 4: writePrescription(doctor, appointments, prescriptions); break;
        case 5: viewPatientHistory(doctor, patients, appointments,
            prescriptions);                      break;
        }
    }
}

// 
// viewTodaysAppointments
// Doctor menu option 1.
// Shows appointments where doctorId matches and date == today.
// Sorted by time slot ascending.
// 
void Menu::viewTodaysAppointments(const Doctor& doctor,
    const Storage<Patient>& patients,
    const Storage<Appointment>& appointments)
{
    SEP;
    std::cout << " == Today's Appointments ==\n";

    // Get today's date string
    char today[12];
    Utility::getTodayDate(today);
    std::cout << " Date: " << today << "\n";
    SEP;

    // Collect matching appointments into pointer array
    Appointment* todays[100];
    int          count = 0;

    Appointment* apptArr = const_cast<Appointment*>(appointments.getAll());
    int          apptN = appointments.size();

    for (int i = 0; i < apptN; i++) {
        if (apptArr[i].getDoctorId() == doctor.getId() &&
            Validator::charsEqual(apptArr[i].getDate(), today)) {
            todays[count++] = &apptArr[i];
        }
    }

    if (count == 0) {
        std::cout << " No appointments scheduled for today.\n";
        return;
    }

    // Sort by time slot ascending (insertion sort on slot string)
    // Slots are fixed format HH:00 so lexicographic order == time order
    for (int i = 1; i < count; i++) {
        Appointment* key = todays[i];
        int j = i - 1;
        // Compare slots character-by-character
        while (j >= 0) {
            // Manual string compare of time slots
            bool greater = false;
            const char* a = todays[j]->getTimeSlot();
            const char* b = key->getTimeSlot();
            for (int k = 0; a[k] != '\0' && b[k] != '\0'; k++) {
                if (a[k] > b[k]) { greater = true;  break; }
                if (a[k] < b[k]) { greater = false; break; }
            }
            if (!greater) break;
            todays[j + 1] = todays[j];
            j--;
        }
        todays[j + 1] = key;
    }

    // Print header
    std::cout << " ApptID | Patient Name          | Slot  | Status\n";
    SEP;

    for (int i = 0; i < count; i++) {
        // Look up patient name
        const Patient* pat = patients.findById(todays[i]->getPatientId());
        const char* patName = (pat != nullptr) ? pat->getName() : "Unknown";

        std::cout << "  " << todays[i]->getAppointmentId()
            << "      | " << patName
            << " | " << todays[i]->getTimeSlot()
            << " | " << todays[i]->getStatus() << "\n";
    }
}

// 
// markCompleted
// Doctor menu option 2.
// Validates: appointment belongs to this doctor, pending, today.
// 
void Menu::markCompleted(const Doctor& doctor,
    Storage<Appointment>& appointments)
{

    std::cout << " == Mark Appointment Complete ==\n";

    char today[12];
    Utility::getTodayDate(today);

    // Display today's pending appointments for this doctor
    Appointment* apptArr = appointments.getAll();
    int          apptN = appointments.size();
    bool         anyShown = false;

    std::cout << " Today's pending appointments:\n";
    for (int i = 0; i < apptN; i++) {
        if (apptArr[i].getDoctorId() == doctor.getId() &&
            Validator::charsEqual(apptArr[i].getDate(), today) &&
            Validator::charsEqual(apptArr[i].getStatus(), "pending")) {

            std::cout << "  ID: " << apptArr[i].getAppointmentId()
                << " | Slot: " << apptArr[i].getTimeSlot() << "\n";
            anyShown = true;
        }
    }

    if (!anyShown) {
        std::cout << " No pending appointments for today.\n";
        return;
    }

    std::cout << " Enter Appointment ID: ";
    int apptId = Utility::readInt();

    // Validate: belongs to this doctor, pending, today
    Appointment* found = appointments.findById(apptId);
    if (found == nullptr ||
        found->getDoctorId() != doctor.getId() ||
        !Validator::charsEqual(found->getStatus(), "pending") ||
        !Validator::charsEqual(found->getDate(), today)) {
        std::cout << " Invalid appointment ID.\n";
        return;
    }

    // Update status to completed
    found->setStatus("completed");
    FileHandler::updateAppointment(appointments);

    std::cout << " Appointment marked as completed.\n";
}

// 
// markNoShow
// Doctor menu option 3.
// Same validation as markCompleted but sets status "noshow"
// and cancels the associated bill (no refund issued).
// 
void Menu::markNoShow(const Doctor& doctor,
    Storage<Appointment>& appointments,
    Storage<Bill>& bills)
{
    SEP;
    std::cout << " Mark Appointment No-Show .......)\n";

    char today[12];
    Utility::getTodayDate(today);

    Appointment* apptArr = appointments.getAll();
    int          apptN = appointments.size();
    bool         anyShown = false;

    std::cout << " Today's pending appointments:\n";
    for (int i = 0; i < apptN; i++) {
        if (apptArr[i].getDoctorId() == doctor.getId() &&
            Validator::charsEqual(apptArr[i].getDate(), today) &&
            Validator::charsEqual(apptArr[i].getStatus(), "pending")) {

            std::cout << "  ID: " << apptArr[i].getAppointmentId()
                << " | Slot: " << apptArr[i].getTimeSlot() << "\n";
            anyShown = true;
        }
    }

    if (!anyShown) {
        std::cout << " No pending appointments for today.\n";
        return;
    }

    std::cout << " Enter Appointment ID: ";
    int apptId = Utility::readInt();

    Appointment* found = appointments.findById(apptId);
    if (found == nullptr ||
        found->getDoctorId() != doctor.getId() ||
        !Validator::charsEqual(found->getStatus(), "pending") ||
        !Validator::charsEqual(found->getDate(), today)) {
        std::cout << " Invalid appointment ID.\n";
        return;
    }

    // Set appointment status to "noshow"
    found->setStatus("noshow");
    FileHandler::updateAppointment(appointments);

    // Find the corresponding bill and set it to "cancelled"
    // (No refund is issued for a no-show)
    Bill* billArr = bills.getAll();
    int   billN = bills.size();
    for (int i = 0; i < billN; i++) {
        if (billArr[i].getAppointmentId() == apptId) {
            billArr[i].setStatus("cancelled");
            FileHandler::updateBill(bills);
            break;
        }
    }

    std::cout << " Appointment marked as no-show.\n";
}

// 
// writePrescription
// Doctor menu option 4.
// Can only write a prescription for a completed appointment.
// Prevents duplicate prescriptions for the same appointment.
// 
void Menu::writePrescription(const Doctor& doctor,
    Storage<Appointment>& appointments,
    Storage<Prescription>& prescriptions)
{
    SEP;
    std::cout << " == Write Prescription ==\n";

    std::cout << " Enter Appointment ID: ";
    int apptId = Utility::readInt();

    // Validate: appointment belongs to this doctor and is completed
    Appointment* appt = appointments.findById(apptId);
    if (appt == nullptr ||
        appt->getDoctorId() != doctor.getId() ||
        !Validator::charsEqual(appt->getStatus(), "completed")) {
        std::cout << " Invalid appointment ID or appointment not completed.\n";
        return;
    }

    // Check if a prescription already exists for this appointment
    const Prescription* prArr = prescriptions.getAll();
    int                 prN = prescriptions.size();
    for (int i = 0; i < prN; i++) {
        if (prArr[i].getAppointmentId() == apptId) {
            std::cout << " Prescription already written for this appointment.\n";
            return;
        }
    }

    // 
    // Collect medicines (up to 499 chars, truncate silently)
    // 
    char medicines[500];
    std::cout << " Enter medicines (format: MedicineName Dosage; e.g. "
        << "Paracetamol 500mg;Amoxicillin 250mg): ";
    Utility::readLine(medicines, 500);

    // 
    // Collect notes (up to 299 chars, truncate silently)
    // 
    char notes[300];
    std::cout << " Enter notes (max 300 chars): ";
    Utility::readLine(notes, 300);

    // Generate a new prescription ID
    int newPrId = prescriptions.getMaxId() + 1;

    // Use the appointment's date as the prescription date
    Prescription newPr(newPrId, apptId,
        appt->getPatientId(),
        doctor.getId(),
        appt->getDate(),
        medicines, notes);

    prescriptions.add(newPr);
    FileHandler::appendPrescription(newPr);

    std::cout << " Prescription saved.\n";
}

// 
// viewPatientHistory
// Doctor menu option 5.
// Access-controlled: doctor can only view patients who have had
// at least one completed appointment with this doctor.
// 
void Menu::viewPatientHistory(const Doctor& doctor,
    const Storage<Patient>& patients,
    const Storage<Appointment>& appointments,
    const Storage<Prescription>& prescriptions)
{
    SEP;
    std::cout << " == View Patient Medical History ==\n";

    std::cout << " Enter Patient ID: ";
    int patId = Utility::readInt();

    // Validate patient exists
    const Patient* pat = patients.findById(patId);
    if (pat == nullptr) {
        std::cout << " Access denied. You can only view records of your own patients.\n";
        return;
    }

    // Check: patient must have at least one COMPLETED appointment
    // with this specific doctor
    const Appointment* apptArr = appointments.getAll();
    int                apptN = appointments.size();
    bool               isMyPatient = false;

    for (int i = 0; i < apptN; i++) {
        if (apptArr[i].getDoctorId() == doctor.getId() &&
            apptArr[i].getPatientId() == patId &&
            Validator::charsEqual(apptArr[i].getStatus(), "completed")) {
            isMyPatient = true;
            break;
        }
    }

    if (!isMyPatient) {
        std::cout << " Access denied. You can only view records of your own patients.\n";
        return;
    }

    // Collect prescriptions written by this doctor for this patient
    Prescription* myPr[100];
    int           myPrCount = 0;

    Prescription* prArr = const_cast<Prescription*>(prescriptions.getAll());
    int           prN = prescriptions.size();

    for (int i = 0; i < prN; i++) {
        if (prArr[i].getDoctorId() == doctor.getId() &&
            prArr[i].getPatientId() == patId) {
            myPr[myPrCount++] = &prArr[i];
        }
    }

    if (myPrCount == 0) {
        std::cout << " No prescriptions written for this patient.\n";
        return;
    }

    // Sort by date descending (most recent first)
    Utility::sortPrescriptionsDesc(myPr, myPrCount);

    std::cout << " Patient: " << pat->getName() << "\n";
    SEP;
    for (int i = 0; i < myPrCount; i++) {
        std::cout << " Date     : " << myPr[i]->getDate() << "\n";
        std::cout << " Medicines: " << myPr[i]->getMedicines() << "\n";
        std::cout << " Notes    : " << myPr[i]->getNotes() << "\n";
        SEP;
    }
}

//  ADMIN MENU
// 
// 

// 
// adminMenu
// Displays the admin panel and dispatches to sub-actions.
// 
void Menu::adminMenu(Storage<Patient>& patients,
    Storage<Doctor>& doctors,
    Storage<Appointment>& appointments,
    Storage<Bill>& bills,
    Storage<Prescription>& prescriptions)
{
    int choice = 0;

    while (true) {
        std::cout << "\n";
        SEP;
        std::cout << " Admin Panel -- MediCore\n";
        SEP;
        std::cout << "  1.  Add Doctor\n";
        std::cout << "  2.  Remove Doctor\n";
        std::cout << "  3.  View All Patients\n";
        std::cout << "  4.  View All Doctors\n";
        std::cout << "  5.  View All Appointments\n";
        std::cout << "  6.  View Unpaid Bills\n";
        std::cout << "  7.  Discharge Patient\n";
        std::cout << "  8.  View Security Log\n";
        std::cout << "  9.  Generate Daily Report\n";
        std::cout << "  10. Logout\n";
        SEP;
        std::cout << " Enter choice: ";

        choice = Utility::readInt();

        if (!Validator::validateMenuChoice(choice, 1, 10)) {
            std::cout << " Invalid choice. Enter 1-10.\n";
            continue;
        }

        if (choice == 10) {
            std::cout << " Logged out.\n";
            return;
        }

        switch (choice) {
        case 1:  addDoctor(doctors);                                    break;
        case 2:  removeDoctor(doctors, appointments);                   break;
        case 3:  viewAllPatients(patients, bills);                      break;
        case 4:  viewAllDoctors(doctors);                               break;
        case 5:  viewAllAppointments(appointments, patients, doctors);  break;
        case 6:  viewUnpaidBills(bills, patients);                      break;
        case 7:  dischargePatient(patients, appointments,
            prescriptions, bills);               break;
        case 8:  viewSecurityLog();                                     break;
        case 9:  generateDailyReport(appointments, bills,
            patients, doctors);              break;
        }
    }
}

// 
// addDoctor
// Admin menu option 1.
// Collects and validates all doctor fields, then persists.
// 
void Menu::addDoctor(Storage<Doctor>& doctors) {
    SEP;
    std::cout << " == Add Doctor ==\n";

    char name[51];
    char spec[51];
    char contact[12];
    char password[51];
    float fee = 0.0f;

    // --- Name ---
    std::cout << " Name (max 50 chars): ";
    Utility::readLine(name, 51);

    // --- Specialization ---
    std::cout << " Specialization (max 50 chars): ";
    Utility::readLine(spec, 51);

    // --- Contact: must be exactly 11 digits ---
    bool contactOk = false;
    while (!contactOk) {
        std::cout << " Contact (11 digits): ";
        Utility::readLine(contact, 12);
        try {
            if (!Validator::validateContact(contact)) {
                throw InvalidInputException(
                    "Contact must be exactly 11 numeric digits.");
            }
            contactOk = true;
        }
        catch (const InvalidInputException& e) {
            std::cout << " " << e.what() << "\n";
        }
    }

    // --- Password: minimum 6 characters ---
    bool passOk = false;
    while (!passOk) {
        std::cout << " Password (min 6 chars): ";
        Utility::readLine(password, 51);
        try {
            if (!Validator::validatePassword(password)) {
                throw InvalidInputException(
                    "Password must be at least 6 characters.");
            }
            passOk = true;
        }
        catch (const InvalidInputException& e) {
            std::cout << " " << e.what() << "\n";
        }
    }

    // --- Fee: must be a positive float ---
    bool feeOk = false;
    while (!feeOk) {
        std::cout << " Consultation fee (PKR): ";
        fee = Utility::readFloat();
        try {
            if (!Validator::validatePositiveFloat(fee)) {
                throw InvalidInputException(
                    "Fee must be a positive number.");
            }
            feeOk = true;
        }
        catch (const InvalidInputException& e) {
            std::cout << " " << e.what() << "\n";
        }
    }

    // Generate new doctor ID (max existing + 1)
    int newDocId = doctors.getMaxId() + 1;

    Doctor newDoc(newDocId, name, spec, contact, password, fee);
    doctors.add(newDoc);

    // Persist to doctors.txt
    FileHandler::appendDoctor(newDoc);

    char idBuf[12];
    Utility::intToStr(newDocId, idBuf);
    std::cout << " Doctor added successfully. ID: " << idBuf << "\n";
}

// 
// removeDoctor
// Admin menu option 2.
// Cannot remove a doctor who has pending appointments.
// 
void Menu::removeDoctor(Storage<Doctor>& doctors,
    const Storage<Appointment>& appointments)
{
    SEP;
    std::cout << " == Remove Doctor ==\n";

    // Display all doctors
    const Doctor* docArr = doctors.getAll();
    int           docN = doctors.size();

    if (docN == 0) {
        std::cout << " No doctors in the system.\n";
        return;
    }

    std::cout << " ID | Name                | Specialization    | Fee\n";
    SEP;
    for (int i = 0; i < docN; i++) {
        char feeBuf[20];
        Utility::floatToStr(docArr[i].getFee(), feeBuf, 2);
        std::cout << "  " << docArr[i].getId()
            << " | " << docArr[i].getName()
            << " | " << docArr[i].getSpecialization()
            << " | PKR " << feeBuf << "\n";
    }

    std::cout << " Enter Doctor ID to remove: ";
    int docId = Utility::readInt();

    // Check the doctor exists
    Doctor* doc = doctors.findById(docId);
    if (doc == nullptr) {
        std::cout << " Doctor not found.\n";
        return;
    }

    // Check for pending appointments for this doctor
    const Appointment* apptArr = appointments.getAll();
    int                apptN = appointments.size();

    for (int i = 0; i < apptN; i++) {
        if (apptArr[i].getDoctorId() == docId &&
            Validator::charsEqual(apptArr[i].getStatus(), "pending")) {
            std::cout << " Cannot remove doctor with pending appointments. "
                << "Cancel or reassign them first.\n";
            return;
        }
    }

    // Remove from in-memory store and rewrite doctors.txt
    FileHandler::deleteDoctor(docId, doctors);

    std::cout << " Doctor removed.\n";
}

// 
// viewAllPatients
// Admin menu option 3.
// Shows each patient with a count of their unpaid bills.
// 
void Menu::viewAllPatients(const Storage<Patient>& patients,
    const Storage<Bill>& bills)
{
    SEP;
    std::cout << " == All Patients ==\n";

    const Patient* patArr = patients.getAll();
    int            patN = patients.size();

    if (patN == 0) {
        std::cout << " No patients registered.\n";
        return;
    }

    std::cout << " ID | Name            | Age | G | Contact     "
        << "| Balance      | Unpaid Bills\n";
    SEP;

    const Bill* billArr = bills.getAll();
    int         billN = bills.size();

    for (int i = 0; i < patN; i++) {
        // Count unpaid bills for this patient
        int unpaidCount = 0;
        for (int j = 0; j < billN; j++) {
            if (billArr[j].getPatientId() == patArr[i].getId() &&
                Validator::charsEqual(billArr[j].getStatus(), "unpaid")) {
                unpaidCount++;
            }
        }

        char balBuf[20];
        Utility::floatToStr(patArr[i].getBalance(), balBuf, 2);
        char unpaidBuf[12];
        Utility::intToStr(unpaidCount, unpaidBuf);

        std::cout << "  " << patArr[i].getId()
            << " | " << patArr[i].getName()
            << " | " << patArr[i].getAge()
            << " | " << patArr[i].getGender()
            << " | " << patArr[i].getContact()
            << " | PKR " << balBuf
            << " | " << unpaidBuf << "\n";
    }
}

// 
// viewAllDoctors
// Admin menu option 4.
// 
void Menu::viewAllDoctors(const Storage<Doctor>& doctors) {
    SEP;
    std::cout << " == All Doctors ==\n";

    const Doctor* docArr = doctors.getAll();
    int           docN = doctors.size();

    if (docN == 0) {
        std::cout << " No doctors registered.\n";
        return;
    }

    std::cout << " ID | Name                | Specialization    | Contact     | Fee\n";
    SEP;
    for (int i = 0; i < docN; i++) {
        char feeBuf[20];
        Utility::floatToStr(docArr[i].getFee(), feeBuf, 2);
        std::cout << "  " << docArr[i].getId()
            << " | " << docArr[i].getName()
            << " | " << docArr[i].getSpecialization()
            << " | " << docArr[i].getContact()
            << " | PKR " << feeBuf << "\n";
    }
}

// 
// viewAllAppointments
// Admin menu option 5.
// Displays all appointments sorted by date descending.
// 
void Menu::viewAllAppointments(const Storage<Appointment>& appointments,
    const Storage<Patient>& patients,
    const Storage<Doctor>& doctors)
{
    SEP;
    std::cout << " == All Appointments ==\n";

    int apptN = appointments.size();
    if (apptN == 0) {
        std::cout << " No appointments found.\n";
        return;
    }

    // Build sorted pointer array
    Appointment* arr[100];
    Appointment* apptAll = const_cast<Appointment*>(appointments.getAll());
    for (int i = 0; i < apptN; i++) arr[i] = &apptAll[i];

    Utility::sortAppointmentsDesc(arr, apptN);

    std::cout << " ID | Patient Name        | Doctor Name         "
        << "| Date       | Slot  | Status\n";
    SEP;

    for (int i = 0; i < apptN; i++) {
        const Patient* pat = patients.findById(arr[i]->getPatientId());
        const Doctor* doc = doctors.findById(arr[i]->getDoctorId());
        const char* patName = (pat != nullptr) ? pat->getName() : "Unknown";
        const char* docName = (doc != nullptr) ? doc->getName() : "Unknown";

        std::cout << "  " << arr[i]->getAppointmentId()
            << " | " << patName
            << " | " << docName
            << " | " << arr[i]->getDate()
            << " | " << arr[i]->getTimeSlot()
            << " | " << arr[i]->getStatus() << "\n";
    }
}

// 
// viewUnpaidBills
// Admin menu option 6.
// Appends [OVERDUE] to bills whose date is > 7 days before today.
// Uses difftime() on parsed dates as required by the spec.
// 
void Menu::viewUnpaidBills(const Storage<Bill>& bills,
    const Storage<Patient>& patients)
{
    SEP;
    std::cout << " == Unpaid Bills ==\n";

    // Get today's date string to compare
    char today[12];
    Utility::getTodayDate(today);

    const Bill* billArr = bills.getAll();
    int            billN = bills.size();
    bool           found = false;

    std::cout << " BillID | Patient Name        | Amount (PKR) | Date\n";
    SEP;

    for (int i = 0; i < billN; i++) {
        if (!Validator::charsEqual(billArr[i].getStatus(), "unpaid")) continue;

        found = true;

        // Look up patient name
        const Patient* pat = patients.findById(billArr[i].getPatientId());
        const char* patName = (pat != nullptr) ? pat->getName() : "Unknown";

        char amtBuf[20];
        Utility::floatToStr(billArr[i].getAmount(), amtBuf, 2);

        // Check if OVERDUE (bill date more than 7 days before today)
        bool overdue = isBillOverdue(billArr[i].getDate(), today);

        std::cout << "  " << billArr[i].getBillId()
            << "      | " << patName
            << " | PKR " << amtBuf
            << " | " << billArr[i].getDate();

        if (overdue) std::cout << " [OVERDUE]";
        std::cout << "\n";
    }

    if (!found) {
        std::cout << " No unpaid bills.\n";
    }
}

// 
// dischargePatient
// Admin menu option 7.
// Checks: no unpaid bills, no pending appointments.
// Archives all records then deletes from active files.
// 
void Menu::dischargePatient(Storage<Patient>& patients,
    Storage<Appointment>& appointments,
    Storage<Prescription>& prescriptions,
    Storage<Bill>& bills)
{
    SEP;
    std::cout << " == Discharge Patient ==\n";

    std::cout << " Enter Patient ID: ";
    int patId = Utility::readInt();

    Patient* pat = patients.findById(patId);
    if (pat == nullptr) {
        std::cout << " Patient not found.\n";
        return;
    }

    // Check for unpaid bills
    const Bill* billArr = bills.getAll();
    int         billN = bills.size();
    for (int i = 0; i < billN; i++) {
        if (billArr[i].getPatientId() == patId &&
            Validator::charsEqual(billArr[i].getStatus(), "unpaid")) {
            std::cout << " Cannot discharge patient with unpaid bills.\n";
            return;
        }
    }

    // Check for pending appointments
    const Appointment* apptArr = appointments.getAll();
    int                apptN = appointments.size();
    for (int i = 0; i < apptN; i++) {
        if (apptArr[i].getPatientId() == patId &&
            Validator::charsEqual(apptArr[i].getStatus(), "pending")) {
            std::cout << " Cannot discharge patient with pending appointments.\n";
            return;
        }
    }

    // Archive all records into discharged.txt
    FileHandler::archivePatient(*pat, appointments, prescriptions, bills);

    // Delete from all active files and in-memory stores
    FileHandler::deleteAppointmentsByPatient(patId, appointments);
    FileHandler::deletePrescriptionsByPatient(patId, prescriptions);
    FileHandler::deleteBillsByPatient(patId, bills);
    FileHandler::deletePatient(patId, patients);

    std::cout << " Patient discharged and archived successfully.\n";
}

// 
// viewSecurityLog
// Admin menu option 8.
// Delegates to FileHandler which reads security_log.txt.
// 
void Menu::viewSecurityLog() {
    SEP;
    std::cout << " == Security Log ==\n";
    FileHandler::printSecurityLog();
}

// 
// generateDailyReport
// Admin menu option 9.
// All figures derived from in-memory stores; no separate file.
// 
void Menu::generateDailyReport(const Storage<Appointment>& appointments,
    const Storage<Bill>& bills,
    const Storage<Patient>& patients,
    const Storage<Doctor>& doctors)
{
    SEP;
    std::cout << " == Daily Report ==\n";

    char today[12];
    Utility::getTodayDate(today);
    std::cout << " Date: " << today << "\n";
    SEP;

    // 
    // Count today's appointments by status
    // 
    int totalToday = 0, pendingToday = 0, completedToday = 0;
    int noshowToday = 0, cancelledToday = 0;

    const Appointment* apptArr = appointments.getAll();
    int                apptN = appointments.size();

    for (int i = 0; i < apptN; i++) {
        if (!Validator::charsEqual(apptArr[i].getDate(), today)) continue;
        totalToday++;
        if (Validator::charsEqual(apptArr[i].getStatus(), "pending"))
            pendingToday++;
        else if (Validator::charsEqual(apptArr[i].getStatus(), "completed"))
            completedToday++;
        else if (Validator::charsEqual(apptArr[i].getStatus(), "noshow"))
            noshowToday++;
        else if (Validator::charsEqual(apptArr[i].getStatus(), "cancelled"))
            cancelledToday++;
    }

    std::cout << " Total appointments today: " << totalToday
        << " (Pending: " << pendingToday
        << " Completed: " << completedToday
        << " No-show: " << noshowToday
        << " Cancelled: " << cancelledToday << ")\n";

    // ---------------------------------------------------------
    // Revenue collected today (paid bills with today's date)
    // ---------------------------------------------------------
    float revenueToday = 0.0f;
    const Bill* billArr = bills.getAll();
    int         billN = bills.size();

    for (int i = 0; i < billN; i++) {
        if (Validator::charsEqual(billArr[i].getDate(), today) &&
            Validator::charsEqual(billArr[i].getStatus(), "paid")) {
            revenueToday += billArr[i].getAmount();
        }
    }

    char revBuf[20];
    Utility::floatToStr(revenueToday, revBuf, 2);
    std::cout << " Revenue collected today (paid bills): PKR " << revBuf << "\n";
    SEP;

    // 
    // Patients with outstanding unpaid bills
    // 
    std::cout << " Patients with outstanding unpaid bills:\n";
    std::cout << " Patient Name            | Total Owed\n";
    SEP;

    const Patient* patArr = patients.getAll();
    int            patN = patients.size();
    bool           anyOwed = false;

    for (int i = 0; i < patN; i++) {
        float owed = 0.0f;
        for (int j = 0; j < billN; j++) {
            if (billArr[j].getPatientId() == patArr[i].getId() &&
                Validator::charsEqual(billArr[j].getStatus(), "unpaid")) {
                owed += billArr[j].getAmount();
            }
        }
        if (owed > 0.0f) {
            char owedBuf[20];
            Utility::floatToStr(owed, owedBuf, 2);
            std::cout << "  " << patArr[i].getName()
                << " | PKR " << owedBuf << "\n";
            anyOwed = true;
        }
    }
    if (!anyOwed) std::cout << "  None\n";
    SEP;

    // 
    // Doctor-wise summary for today
    // 
    std::cout << " Doctor-wise summary for today:\n";
    std::cout << " Doctor Name             | Completed | Pending | No-show\n";
    SEP;

    const Doctor* docArr = doctors.getAll();
    int           docN = doctors.size();

    for (int i = 0; i < docN; i++) {
        int completed = 0, pending = 0, noshow = 0;
        int dId = docArr[i].getId();

        for (int j = 0; j < apptN; j++) {
            if (apptArr[j].getDoctorId() != dId) continue;
            if (!Validator::charsEqual(apptArr[j].getDate(), today)) continue;

            if (Validator::charsEqual(apptArr[j].getStatus(), "completed"))
                completed++;
            else if (Validator::charsEqual(apptArr[j].getStatus(), "pending"))
                pending++;
            else if (Validator::charsEqual(apptArr[j].getStatus(), "noshow"))
                noshow++;
        }

        // Only print doctors who had activity today
        if (completed + pending + noshow > 0) {
            std::cout << "  " << docArr[i].getName()
                << " | " << completed
                << " | " << pending
                << " | " << noshow << "\n";
        }
    }
}

// 
// 
//  PRIVATE HELPERS
// 
// 

// 
// displayAvailableSlots
// Prints the 8 daily slots, marking each as Available or Taken.
// A slot is taken if a non-cancelled appointment exists for the
// same doctor, date, and slot.
// 
void Menu::displayAvailableSlots(int                         doctorId,
    const char* date,
    const Storage<Appointment>& appointments)
{
    // The 8 fixed daily time slots
    const char* slots[8] = {
        "09:00", "10:00", "11:00", "12:00",
        "13:00", "14:00", "15:00", "16:00"
    };

    std::cout << " Available time slots for " << date << ":\n";

    for (int s = 0; s < 8; s++) {
        if (isSlotTaken(doctorId, date, slots[s], appointments)) {
            std::cout << "  " << slots[s] << " [Taken]\n";
        }
        else {
            std::cout << "  " << slots[s] << " [Available]\n";
        }
    }
}

// 
// isSlotTaken
// Returns true if there is a non-cancelled appointment for the
// given doctorId, date, and slot.
// We use the overloaded == (conflict check) on Appointment objects.
// 
bool Menu::isSlotTaken(int                         doctorId,
    const char* date,
    const char* slot,
    const Storage<Appointment>& appointments)
{
    // Build a probe appointment with the same doctor/date/slot
    // and status "pending" (not cancelled) so the == operator
    // triggers the conflict check correctly.
    Appointment probe(0, 0, doctorId, date, slot, "pending");

    const Appointment* apptArr = appointments.getAll();
    int                apptN = appointments.size();

    for (int i = 0; i < apptN; i++) {
        // Use the overloaded == operator: returns true on CONFLICT
        if (apptArr[i] == probe) {
            return true; // slot is already taken
        }
    }
    return false;
}

// 
// isBillOverdue
// Returns true if billDate is more than 7 days before todayDate.
// Both dates are in DD-MM-YYYY format.
// Uses DateTime::fromDateString() + DateTime::diffDays() so that
// NO 'struct' keyword appears in Menu.cpp.
// difftime() is still used internally inside DateTime.cpp.
// 
bool Menu::isBillOverdue(const char* billDate, const char* todayDate) {
    // Parse both date strings into DateTime objects.
    // DateTime::fromDateString() handles the DD-MM-YYYY layout
    // and sets the time fields to 00:00:00 (midnight).
    DateTime bill = DateTime::fromDateString(billDate);
    DateTime today = DateTime::fromDateString(todayDate);

    // diffDays(a, b) returns (b - a) in days.
    // A positive result means todayDate is AFTER billDate.
    // More than 7 days difference means the bill is overdue.
    double days = DateTime::diffDays(bill, today);
    return (days > 7.0);
}