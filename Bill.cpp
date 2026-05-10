// ============================================================
// Bill.cpp
// Implements the Bill class — one bill per appointment.
// ============================================================

#include "Bill.h"
#include <iostream>

// Internal helpers (file-scope)
static void billCopy(char* dest, const char* src, int maxLen) {
    int i = 0;
    while (src[i] != '\0' && i < maxLen - 1) { dest[i] = src[i]; i++; }
    dest[i] = '\0';
}

// ============================================================
// Constructors / Destructor
// ============================================================

Bill::Bill() : billId(0), patientId(0), appointmentId(0), amount(0.0f) {
    status[0] = '\0';
    date[0] = '\0';
}

Bill::Bill(int billId, int patientId, int appointmentId,
    float amount, const char* status, const char* date)
    : billId(billId), patientId(patientId),
    appointmentId(appointmentId), amount(amount)
{
    billCopy(this->status, status, 12);
    billCopy(this->date, date, 12);
}

Bill::~Bill() {}

// ============================================================
// Getters
// ============================================================

int         Bill::getBillId()        const { return billId; }
int         Bill::getPatientId()     const { return patientId; }
int         Bill::getAppointmentId() const { return appointmentId; }
float       Bill::getAmount()        const { return amount; }
const char* Bill::getStatus()        const { return status; }
const char* Bill::getDate()          const { return date; }

// getId() — required by Storage<Bill> template
int Bill::getId() const { return billId; }

// ============================================================
// Setter
// ============================================================

void Bill::setStatus(const char* newStatus) {
    billCopy(status, newStatus, 12);
}

// ============================================================
// << operator
// ============================================================
std::ostream& operator<<(std::ostream& out, const Bill& b) {
    out << "BillID: " << b.billId
        << " | PatID: " << b.patientId
        << " | ApptID: " << b.appointmentId
        << " | Amount: PKR " << b.amount
        << " | Status: " << b.status
        << " | Date: " << b.date;
    return out;
}

// ============================================================
// toCSV — billId,patientId,appointmentId,amount,status,date
// ============================================================
void Bill::toCSV(char* buffer, int maxLen) const {
    int pos = 0;

    auto writeInt = [&](int v) {
        char tmp[20]; int len = 0;
        if (v == 0) { buffer[pos++] = '0'; return; }
        while (v > 0 && len < 19) { tmp[len++] = (char)('0' + v % 10); v /= 10; }
        for (int k = len - 1; k >= 0 && pos < maxLen - 1; k--)
            buffer[pos++] = tmp[k];
        };

    auto writeFloat = [&](float v) {
        if (v < 0.0f) { buffer[pos++] = '-'; v = -v; }
        int intPart = (int)v;
        writeInt(intPart);
        buffer[pos++] = '.';
        int frac = (int)((v - (float)intPart) * 100.0f + 0.5f);
        buffer[pos++] = (char)('0' + frac / 10);
        buffer[pos++] = (char)('0' + frac % 10);
        };

    auto writeStr = [&](const char* s) {
        for (int i = 0; s[i] != '\0' && pos < maxLen - 1; i++)
            buffer[pos++] = s[i];
        };

    writeInt(billId);        buffer[pos++] = ',';
    writeInt(patientId);     buffer[pos++] = ',';
    writeInt(appointmentId); buffer[pos++] = ',';
    writeFloat(amount);      buffer[pos++] = ',';
    writeStr(status);        buffer[pos++] = ',';
    writeStr(date);

    buffer[pos] = '\0';
}