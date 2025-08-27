#include <cstdlib>
#include <fstream>
#include <print>
#include <regex>

#include "oly/config.hpp"
#include "oly/log.hpp"
#include "oly/utils.hpp"

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

std::string expand_vars(std::string str, bool expand_config_vars, bool expand_env_vars) {
	std::string fmt = str;

	if (expand_env_vars && fmt.starts_with("~/")) {
		const char* home = getenv("HOME");
		fmt = std::string(home == NULL ? "" : home) + fmt.substr(1);
	}

	static std::regex var("\\$\\{([^}]+)\\}");
	std::smatch match;
	while (std::regex_search(fmt, match, var)) {
		std::string var = "";
		if (expand_config_vars && config[match[1].str()]) {
			var = config[match[1].str()].as<std::string>();
		} else if (expand_env_vars) {
			const char* s = getenv(match[1].str().c_str());
			var = (s == NULL ? "" : s);
		}
		fmt.replace(match[0].first, match[0].second, var);
	}
	return fmt;
};

std::string expand_env_vars(std::string str) {
	return expand_vars(str, false, true);
}

void create_file(const fs::path& filepath, const std::string& contents) {
	fs::create_directories(filepath.parent_path());
	std::ofstream out(filepath);
	out << contents;
	out.close();
}

void set_log_level(std::string level) {
	std::transform(level.begin(), level.end(), level.begin(),
	               [](unsigned char c) { return std::toupper(c); });

	if (level == "CRITICAL") {
		Log::log_level = severity::CRITICAL;
	} else if (level == "ERROR") {
		Log::log_level = severity::ERROR;
	} else if (level == "WARNING") {
		Log::log_level = severity::WARNING;
	} else if (level == "INFO") {
		Log::log_level = severity::INFO;
	} else if (level == "HINT") {
		Log::log_level = severity::HINT;
	} else if (level == "DEBUG") {
		Log::log_level = severity::DEBUG;
	} else if (level == "TRACE") {
		Log::log_level = severity::TRACE;
	} else {
		Log::Log(severity::ERROR,
		         level + "is not a valid log level. Using default severity INFO.");
		Log::log_level = severity::INFO;
	}
}

void input_file::create_file() {
	fs::create_directories(filepath.parent_path());
	std::ofstream out(filepath);
	out << contents;
	out.close();
}

input_file::input_file(fs::path filepath, std::string contents, bool remove)
    : remove(remove), filepath(filepath), contents(contents) {
	create_file();
	edit();
};
input_file::~input_file() {
	if (remove && fs::exists(filepath)) {
		fs::remove(filepath);
	}
}

std::deque<std::string> input_file::lines() {
	std::ifstream file(filepath);
	std::deque<std::string> lines;
	std::string line;
	if (!file.is_open())
		Log::Log(severity::CRITICAL, "unable to open " + filepath.string() + "!");
	while (getline(file, line)) {
		lines.push_back(line);
	}
	return lines;
}

void input_file::edit() {
	std::system((config["editor"].as<std::string>() + " \"" + filepath.string() + "\"")
	                .c_str()); // pray for the filetype not to contain quotes
}
