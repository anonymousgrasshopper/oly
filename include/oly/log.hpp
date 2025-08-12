#pragma once

#include <string>

enum class severity;

inline bool operator<(severity a, severity b);
inline bool operator>(severity a, severity b);
inline bool operator<=(severity a, severity b);
inline bool operator>=(severity a, severity b);

enum class logopt : unsigned;

inline logopt operator|(logopt a, logopt b);
inline logopt& operator|=(logopt& a, logopt b);

namespace Log {
inline severity log_level;
void Log(severity level, const std::string& message, logopt opts = logopt::NONE, std::string cmd = "");
}
