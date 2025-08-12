#include <iostream>
#include <string>
#include <limits>
#include <print>
#include <cstdlib>

#include "oly/log.hpp"

inline logopt operator|(logopt a, logopt b) {
    return static_cast<logopt>(
        static_cast<unsigned>(a) | static_cast<unsigned>(b)
    );
}
inline logopt& operator|=(logopt& a, logopt b) {
    a = a | b;
    return a;
}

static bool has_option(logopt opts, logopt flag) {
    return (static_cast<unsigned>(opts) & static_cast<unsigned>(flag)) != 0;
}

static const char* severity_color(severity lvl) {
    switch (lvl) {
        case severity::CRITICAL: return "\033[1;41;97m"; // white on red bg
        case severity::ERROR:    return "\033[1;31m";     // bright red
        case severity::WARNING:  return "\033[1;33m";     // bright yellow
        case severity::INFO:     return "\033[1;36m";     // bright cyan
        case severity::HINT:     return "\033[0;36m";     // cyan
        case severity::DEBUG:    return "\033[0;35m";     // magenta
        case severity::TRACE:    return "\033[0;90m";     // gray
    }
}
static constexpr const char* COLOR_RESET = "\033[0m";

void Log(severity level, const std::string& message,
         logopt opts = logopt::NONE, const std::string& cmd = "") {
    const char* sev_names[] = {
        "CRITICAL", "ERROR", "WARNING", "INFO", "HINT", "DEBUG", "TRACE"
    };

    const char* sev_str = sev_names[static_cast<int>(level)];
    std::println(std::cerr, "{}{}{}: {}", severity_color(level),
                 sev_str, COLOR_RESET, message);

    if (has_option(opts, logopt::HELP)) {
        std::println(std::cerr, "{:{}}{}", "", std::strlen(sev_str) + 2,
                     "use oly --help for more information");
    }

    if (has_option(opts, logopt::CMD_HELP)) {
        std::string cmd_str = cmd.empty() ? "<cmd>" : cmd;
        std::println(std::cerr, "{:{}}use oly {} --help for more information",
                     "", std::strlen(sev_str) + 2, cmd_str);
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
