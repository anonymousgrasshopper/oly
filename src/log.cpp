#include <string>
#include <iostream>

#include <oly/log.hpp>

inline logopt operator|(logopt a, logopt b) {
    return static_cast<logopt>(
        static_cast<unsigned>(a) | static_cast<unsigned>(b)
    );
}
inline logopt& operator|=(logopt& a, logopt b) {
    a = a | b;
    return a;
}

inline bool has_option(logopt opts, logopt flag) {
    return (static_cast<unsigned>(opts) & static_cast<unsigned>(flag)) != 0;
}

inline const char* severity_color(severity lvl) {
    switch (lvl) {
        case severity::CRITICAL: return "\033[1;41;97m"; // white on red bg
        case severity::ERROR:    return "\033[1;31m";     // bright red
        case severity::WARNING:  return "\033[1;33m";     // bright yellow
        case severity::INFO:     return "\033[1;36m";     // bright cyan
        case severity::HINT:     return "\033[0;36m";     // cyan
        case severity::DEBUG:    return "\033[0;35m";     // magenta
        case severity::TRACE:    return "\033[0;90m";     // gray
    }
    return "\033[0m"; // reset
}

void Log(severity level, const std::string& message, logopt opts = logopt::NONE) {
    std::ostream& out = has_option(opts, logopt::TO_STDERR) ? std::cerr : std::cout;

    // Apply severity color if prefix is printed
    if (!has_option(opts, logopt::NO_PREFIX)) {
        static const char* sev_names[] = {
            "CRITICAL", "ERROR", "WARNING", "INFO", "HINT", "DEBUG", "TRACE"
        };
        out << severity_color(level)
            << "[" << sev_names[static_cast<int>(level)] << "] "
            << "\033[0m"; // reset color
    }

    // Message in uppercase if requested
    if (has_option(opts, logopt::UPPERCASE)) {
        for (char c : message) out << static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    } else {
        out << message;
    }
    out << "\n";

    if (has_option(opts, logopt::WAIT)) {
        out << "\033[0;90mPress Enter to continue...\033[0m";
        out.flush();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}
