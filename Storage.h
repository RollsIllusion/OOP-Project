#pragma once
#ifndef STORAGE_H
#define STORAGE_H

// ============================================================
// Storage.h
// A generic template container that stores up to 100 objects
// of type T in a static array (no std::vector allowed).
//
// T must expose:   int getId() const;
// so that find-by-ID and remove-by-ID work correctly.
//
// Usage examples:
//   Storage<Patient>      patientStore;
//   Storage<Doctor>       doctorStore;
//   Storage<Appointment>  appointmentStore;
//   Storage<Bill>         billStore;
//   Storage<Prescription> prescriptionStore;
// ============================================================

template <typename T>
class Storage {
private:
    T   data[100]; // fixed-size array — no std::vector anywhere
    int count;     // number of currently stored items (0..100)

public:
    // ----------------------------------------------------------
    // Constructor — initialises count to 0
    // ----------------------------------------------------------
    Storage() : count(0) {}

    // ----------------------------------------------------------
    // add
    // Appends a copy of 'item' to the array.
    // Returns true on success, false if the array is full (100 items).
    // ----------------------------------------------------------
    bool add(const T& item) {
        if (count >= 100) return false;
        data[count++] = item;
        return true;
    }

    // ----------------------------------------------------------
    // removeById
    // Finds the element whose getId() == id and removes it by
    // shifting all subsequent elements one position left.
    // Returns true if found and removed, false otherwise.
    // ----------------------------------------------------------
    bool removeById(int id) {
        for (int i = 0; i < count; i++) {
            if (data[i].getId() == id) {
                // Shift every element after position i one step left
                for (int j = i; j < count - 1; j++) {
                    data[j] = data[j + 1];
                }
                count--;
                return true;
            }
        }
        return false; // id not found
    }

    // ----------------------------------------------------------
    // findById
    // Returns a pointer to the element with getId() == id,
    // or nullptr if not found.
    // ----------------------------------------------------------
    T* findById(int id) {
        for (int i = 0; i < count; i++) {
            if (data[i].getId() == id) {
                return &data[i];
            }
        }
        return nullptr;
    }

    // Const overload — used in const contexts
    const T* findById(int id) const {
        for (int i = 0; i < count; i++) {
            if (data[i].getId() == id) {
                return &data[i];
            }
        }
        return nullptr;
    }

    // ----------------------------------------------------------
    // getAll
    // Returns a pointer to the internal array so callers can
    // iterate over all stored elements.
    // ----------------------------------------------------------
    T* getAll() {
        return data;
    }

    const T* getAll() const {
        return data;
    }

    // ----------------------------------------------------------
    // size
    // Returns how many items are currently stored.
    // ----------------------------------------------------------
    int size() const {
        return count;
    }

    // ----------------------------------------------------------
    // clear
    // Resets count to 0 (useful when reloading from file).
    // ----------------------------------------------------------
    void clear() {
        count = 0;
    }

    // ----------------------------------------------------------
    // getMaxId
    // Iterates all stored items and returns the maximum getId()
    // value found. Returns 0 if the store is empty.
    // Used to generate the next unique ID (maxId + 1).
    // ----------------------------------------------------------
    int getMaxId() const {
        int maxId = 0;
        for (int i = 0; i < count; i++) {
            if (data[i].getId() > maxId) {
                maxId = data[i].getId();
            }
        }
        return maxId;
    }
};

#endif // STORAGE_H