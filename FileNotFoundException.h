#pragma once
#ifndef FILENOTFOUNDEXCEPTION_H
#define FILENOTFOUNDEXCEPTION_H

// ============================================================
// FileNotFoundException.h
// Thrown when a required .txt data file cannot be opened
// during program startup or any file I/O operation.
// Inherits from HospitalException.
// ============================================================

#include "HospitalException.h"

class FileNotFoundException : public HospitalException {
public:
    // Constructor accepts the filename that could not be opened
    // and builds a descriptive error message stored in message[]
    FileNotFoundException(const char* filename);

    // Overrides what() to return the file-not-found message
    const char* what() const override;
};

#endif // FILENOTFOUNDEXCEPTION_H