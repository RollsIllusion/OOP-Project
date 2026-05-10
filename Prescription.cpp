// ============================================================
// Prescription.cpp
// Implements the Prescription class.
// ============================================================

#include "Prescription.h"
#include <iostream>

// Internal helpers
static void prCopy(char* dest, const char* src, int maxLen) {
    int i = 0;
    while (src[i] != '\0' && i < maxLen - 1) { dest[i] = src[i]; i++; }
    dest[i] = '\0';
}

// ============================================================
// Constructors / Destructor
// ============================================================

Prescription::Prescription()
    : prescriptionId(0), appointmentId(0), patientId(0), doctorId(0)
{
    date[0] = '\0';
    medicines[0] = '\0';
    notes[0] = '\0';
}

Prescription::Prescription(int prescriptionId, int appointmentId,
    int patientId, int doctorId,
    const char* date, const char* medicines,
    const char* notes)
    : prescriptionId(prescriptionId), appointmentId(appointmentId),
    patientId(patientId), doctorId(doctorId)
{
    prCopy(this->date, date, 12);
    prCopy(this->medicines, medicines, 500);
    prCopy(this->notes, notes, 300);
}

Prescription::~Prescription() {}

// ============================================================
// Getters
// ============================================================

int         Prescription::getPrescriptionId() const { return prescriptionId; }
int         Prescription::getAppointmentId()  const { return appointmentId; }
int         Prescription::getPatientId()      const { return patientId; }
int         Prescription::getDoctorId()       const { return doctorId; }
const char* Prescription::getDate()           const { return date; }
const char* Prescription::getMedicines()      const { return medicines; }
const char* Prescription::getNotes()          const { return notes; }

// getId() — required by Storage<Prescription>
int Prescription::getId() const { return prescriptionId; }

// ============================================================
// << operator
// ============================================================
std::ostream& operator<<(std::ostream& out, const Prescription& pr) {
    out << "PrescID: " << pr.prescriptionId
        << " | ApptID: " << pr.appointmentId
        << " | Date: " << pr.date
        << "\n  Medicines: " << pr.medicines
        << "\n  Notes: " << pr.notes;
    return out;
}

// ============================================================
// toCSV
// prescriptionId,appointmentId,patientId,doctorId,date,medicines,notes
// ============================================================
void Prescription::toCSV(char* buffer, int maxLen) const {
    int pos = 0;

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

    writeInt(prescriptionId); buffer[pos++] = ',';
    writeInt(appointmentId);  buffer[pos++] = ',';
    writeInt(patientId);      buffer[pos++] = ',';
    writeInt(doctorId);       buffer[pos++] = ',';
    writeStr(date);           buffer[pos++] = ',';
    writeStr(medicines);      buffer[pos++] = ',';
    writeStr(notes);

    buffer[pos] = '\0';
}

// ============================================================
// compareDate — DD-MM-YYYY: compare year → month → day
// ============================================================
int Prescription::compareDate(const Prescription& other) const {
    // Year: positions 6-9
    int y1 = (date[6] - '0') * 1000 + (date[7] - '0') * 100
        + (date[8] - '0') * 10 + (date[9] - '0');
    int y2 = (other.date[6] - '0') * 1000 + (other.date[7] - '0') * 100
        + (other.date[8] - '0') * 10 + (other.date[9] - '0');
    if (y1 != y2) return y1 - y2;

    // Month: positions 3-4
    int m1 = (date[3] - '0') * 10 + (date[4] - '0');
    int m2 = (other.date[3] - '0') * 10 + (other.date[4] - '0');
    if (m1 != m2) return m1 - m2;

    // Day: positions 0-1
    int d1 = (date[0] - '0') * 10 + (date[1] - '0');
    int d2 = (other.date[0] - '0') * 10 + (other.date[1] - '0');
    return d1 - d2;
}