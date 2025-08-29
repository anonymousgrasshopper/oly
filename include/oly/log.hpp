#pragma once

#include <string>

enum class severity {
	TRACE,
	DEBUG,
	HINT,
	INFO,
	WARNING,
	ERROR,
	CRITICAL,
};

bool operator<(severity a, severity b);
bool operator>(severity a, severity b);
bool operator<=(severity a, severity b);
bool operator>=(severity a, severity b);

enum class logopt : unsigned {
	NONE = 0,
	WAIT = 1 << 0,     // Wait for user input
	HELP = 1 << 1,     // Advise to use oly --help
	NO_PREFIX = 1 << 2 // Don't add a 'SEVERITY:' prefix
};

logopt operator|(logopt a, logopt b);
logopt& operator|=(logopt& a, logopt b);

namespace Log {
inline severity log_level = severity::INFO;

void CRITICAL(const std::string& message, logopt opts = logopt::NONE,
              const std::string& cmd = "");
void ERROR(const std::string& message, logopt opts = logopt::NONE,
           const std::string& cmd = "");
void WARNING(const std::string& message, logopt opts = logopt::NONE,
             const std::string& cmd = "");
void INFO(const std::string& message, logopt opts = logopt::NONE,
          const std::string& cmd = "");
void DEBUG(const std::string& message, logopt opts = logopt::NONE,
           const std::string& cmd = "");
void TRACE(const std::string& message, logopt opts = logopt::NONE,
           const std::string& cmd = "");
} // namespace Log
