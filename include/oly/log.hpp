#pragma once

#include <string>

enum class severity {
    CRITICAL, ERROR, WARNING, INFO, HINT, DEBUG, TRACE
};

enum class logopt : unsigned {
    NONE       = 0,
    WAIT       = 1 << 0, // Wait for user input
    TO_STDERR  = 1 << 1, // Print to std::cerr
    NO_PREFIX  = 1 << 2, // Don't print severity prefix
    UPPERCASE  = 1 << 3  // Uppercase the message
};

inline logopt operator|(logopt a, logopt b);
inline logopt& operator|=(logopt& a, logopt b);

void Log(severity level, const std::string& message, logopt opts = logopt::NONE);
