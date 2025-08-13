#pragma once

#include <string>

enum class severity { CRITICAL, ERROR, WARNING, INFO, HINT, DEBUG, TRACE };

inline bool operator<(severity a, severity b);
inline bool operator>(severity a, severity b);
inline bool operator<=(severity a, severity b);
inline bool operator>=(severity a, severity b);

enum class logopt : unsigned {
	NONE = 0,
	WAIT = 1 << 0,     // Wait for user input
	HELP = 1 << 1,     // Advise to use oly --help
	CMD_HELP = 1 << 2, // Advise to use oly <cmd> --help
};

inline logopt operator|(logopt a, logopt b);
inline logopt& operator|=(logopt& a, logopt b);

namespace Log {
extern inline severity log_level;
void Log(severity level, const std::string& message, logopt opts, std::string cmd = "");
} // namespace Log
