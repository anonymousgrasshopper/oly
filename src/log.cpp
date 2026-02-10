#include <cstdlib>
#include <iostream>
#include <limits>
#include <print>
#include <string>

#include "oly/config.hpp"
#include "oly/log.hpp"

bool operator<(severity a, severity b) {
	return static_cast<unsigned>(a) < static_cast<unsigned>(b);
}
bool operator>(severity a, severity b) {
	return static_cast<unsigned>(a) > static_cast<unsigned>(b);
}
bool operator<=(severity a, severity b) {
	return static_cast<unsigned>(a) <= static_cast<unsigned>(b);
}
bool operator>=(severity a, severity b) {
	return static_cast<unsigned>(a) >= static_cast<unsigned>(b);
}

logopt operator|(logopt a, logopt b) {
	return static_cast<logopt>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}
logopt& operator|=(logopt& a, logopt b) {
	a = a | b;
	return a;
}

static constexpr bool has_option(logopt opts, logopt flag) {
	return (static_cast<unsigned>(opts) & static_cast<unsigned>(flag)) != 0;
}

static constexpr std::string severity_name(severity lvl) {
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
static constexpr std::string severity_color(severity lvl) {
	switch (lvl) {
	case severity::CRITICAL:
		return "\x1b[38;2;0;0;0m\x1b[48;2;232;36;36m";
	case severity::ERROR:
		return "\x1b[38;2;232;36;36m";
	case severity::WARNING:
		return "\x1b[38;2;255;158;59m";
	case severity::INFO:
		return "\x1b[38;2;106;149;137m";
	case severity::HINT:
		return "\x1b[38;2;101;133;148m";
	case severity::DEBUG:
		return "\x1b[38;2;113;124;124m";
	case severity::TRACE:
		return "\x1b[38;2;220;215;186m";
	}
}
constexpr const std::string COLOR_RESET = "\x1b[0m";

namespace Log {
static void Log(severity level, const std::string& message, logopt opts) {
	if (level < log_level)
		return;

	if (!has_option(opts, logopt::NO_PREFIX))
		std::print(std::cerr, "{}{}{}: ", severity_color(level), severity_name(level),
		           COLOR_RESET);

	std::println(std::cerr, "{}", message);

	if (has_option(opts, logopt::HELP)) {
		int padding =
		    has_option(opts, logopt::NO_PREFIX) ? 0 : severity_name(level).length() + 2;
		std::string cmd_str = config["cmd"] && config["cmd"].as<std::string>() != "default"
		                          ? config["cmd"].as<std::string>() + " "
		                          : "";
		std::println(std::cerr, "{:{}}use oly {}--help for more information", "", padding,
		             cmd_str);
	}

	if (has_option(opts, logopt::WAIT)) {
		Wait();
	}
}

void Wait() {
	std::print(std::cerr, "\033[0;90mPress Enter to continue...\033[0m");
	std::cerr.flush();
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void CRITICAL(const std::string& message, logopt opts) {
	Log(severity::CRITICAL, message, opts);
	std::exit(EXIT_FAILURE);
}
void ERROR(const std::string& message, logopt opts) {
	Log(severity::ERROR, message, opts);
}
void WARNING(const std::string& message, logopt opts) {
	Log(severity::WARNING, message, opts);
}
void INFO(const std::string& message, logopt opts) {
	Log(severity::INFO, message, opts);
}
void DEBUG(const std::string& message, logopt opts) {
	Log(severity::DEBUG, message, opts);
}
void TRACE(const std::string& message, logopt opts) {
	Log(severity::TRACE, message, opts);
}
} // namespace Log
