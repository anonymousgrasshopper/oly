#include "oly/utils.hpp"
#include <cstdlib>
#include <print>
#include <regex>

// displays a help message
void print_help() {
	std::println("usage: oly <cmd> [args [...]].\n");
	std::println(R"(Available subcommands:
    add                          - add a problem to the database
    edit                         - edit an entry in the database
    show                         - generate a pdf file for a problem and open it
    gen                          - generate a LaTeX or PDF file from a problem
    search                       - search problems by tag, difficulty, contest...
    Run oly <cmd> --help for more information regarding a specific subcommand
    )");
	std::println(R"(Arguments:
    --help              -h       - Show this help message
    --config FILE       -c FILE  - Specify config file to use
    --verify-config              - Check wether the config has any errors
    --version           -v       - Print this binary's version)");
}

// expands tilde and environment variables in a string
std::string expand_env_vars(const std::string& str) {
	std::string fmt = str;
	// expand tilde
	if (fmt.starts_with("~/")) {
		fmt = std::string(getenv("HOME")) + fmt.substr(1);
	}
	// expand env vars
	static std::regex env("\\$\\{([^}]+)\\}");
	std::smatch match;
	while (std::regex_search(fmt, match, env)) {
		const char* s = getenv(match[1].str().c_str());
		const std::string var(s == NULL ? "" : s);
		fmt.replace(match[0].first, match[0].second, var);
	}
	return fmt;
}
