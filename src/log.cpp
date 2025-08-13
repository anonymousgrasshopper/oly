#include <cstdlib>
#include <iostream>
#include <limits>
#include <print>
#include <string>

#include "oly/log.hpp"

inline bool operator<(severity a, severity b) {
	return static_cast<unsigned>(a) < static_cast<unsigned>(b);
}
inline bool operator>(severity a, severity b) {
	return static_cast<unsigned>(a) > static_cast<unsigned>(b);
}
inline bool operator<=(severity a, severity b) {
	return static_cast<unsigned>(a) <= static_cast<unsigned>(b);
}
inline bool operator>=(severity a, severity b) {
	return static_cast<unsigned>(a) >= static_cast<unsigned>(b);
}

inline logopt operator|(logopt a, logopt b) {
	return static_cast<logopt>(
			static_cast<unsigned>(a) | static_cast<unsigned>(b));
}
inline logopt& operator|=(logopt& a, logopt b) {
	a = a | b;
	return a;
}

static bool has_option(logopt opts, logopt flag) {
	return (static_cast<unsigned>(opts) & static_cast<unsigned>(flag)) != 0;
}

static const char* severity_name(severity lvl) {
	switch (lvl) {
	case severity::CRITICAL:
		return "CRITICAL";
	case severity::ERROR:
		return "ERROR";
	case severity::WARNING:
		return "WARNING";
	case severity::INFO:
		return "INFO";
	case severity::HINT:
		return "HINT";
	case severity::DEBUG:
		return "DEBUG";
	case severity::TRACE:
		return "TRACE";
	}
}
static const char* severity_color(severity lvl) {
	switch (lvl) {
	case severity::CRITICAL:
		return "\033[1;41;97m"; // white on red bg
	case severity::ERROR:
		return "\033[1;31m"; // bright red
	case severity::WARNING:
		return "\033[1;33m"; // bright yellow
	case severity::INFO:
		return "\033[1;36m"; // bright cyan
	case severity::HINT:
		return "\033[0;36m"; // cyan
	case severity::DEBUG:
		return "\033[0;35m"; // magenta
	case severity::TRACE:
		return "\033[0;90m"; // gray
	}
}
constexpr const char* COLOR_RESET = "\033[0m";

namespace Log {
inline severity log_level = severity::INFO;

void Log(severity level, const std::string& message,
				 logopt opts = logopt::NONE, const std::string& cmd = "") {
	if (level < log_level)
		return;
	std::println(std::cerr, "{}{}{}: {}", severity_color(level),
							 severity_name(level), COLOR_RESET, message);

	if (has_option(opts, logopt::HELP)) {
		std::println(std::cerr, "{:{}}{}", "", severity_name(level) + 2,
								 "use oly --help for more information");
	}

	if (has_option(opts, logopt::CMD_HELP)) {
		std::string cmd_str = cmd.empty() ? "<cmd>" : cmd;
		std::println(std::cerr, "{:{}}use oly {} --help for more information",
								 "", severity_name(level) + 2, cmd_str);
	}

	if (has_option(opts, logopt::WAIT)) {
		std::print(std::cerr, "\033[0;90mPress Enter to continue...\033[0m");
		std::cerr.flush();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}

	if (level == severity::CRITICAL) {
		std::exit(EXIT_FAILURE);
	}
}
} // namespace Log
