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

std::string get_problem_id(const std::string& source) {
	std::smatch match;
	std::string contest, year, problem;

	// 1. Contest name: at least 2 consecutive letters
	std::regex contest_regex(R"(([A-Za-z]{2,}))");
	if (std::regex_search(source, match, contest_regex)) {
		contest = match.str(1) + " ";
		std::transform(contest.begin(), contest.end(), contest.begin(), ::toupper);
	}

	// 2. Year: 4 digits or 2 digits not part of a longer digit sequence
	std::regex year_regex(R"((\b\d{2}\b|\b\d{4}\b))");
	if (std::regex_search(source, match, year_regex)) {
		if (match.str(1).size() == 4) {
			year = match.str(1) + " ";
		} else if (match.str(1).size() == 2) {
			year = "20" + match.str(1) + " ";
		}
	}

	// 3. Problem: single digit, not part of year, preceded by non-digit or letter, not
	// followed by digit
	std::regex problem_regex(R"((\b[A-Za-z]?\d\b))");
	if (std::regex_search(source, match, problem_regex)) {
		if (match.str(1).size() == 1) {
			problem = "P" + match.str(1);
		} else if (match.str(1).size() == 2) {
			problem = match.str(1);
		}
	}

	return contest + year + problem;
}

fs::path get_problem_path(const std::string& source) {
	return fs::path(expand_env_vars(config["base_path"].as<std::string>()) +
	                get_problem_id(source) + ".tex");
}

bool is_separator(const std::string& line) {
	return line == config["separator"].as<std::string>();
}

bool is_yaml(const std::string& line) {
	if (std::regex_match(line, std::regex(R"(^\s*$)")))
		return true; // true if blank

	std::regex yaml_pattern(R"(^([^:\s][^:]*):\s*.+$)");
	return std::regex_match(line, yaml_pattern);
}

std::optional<YAML::Node> load_yaml(const fs::path& filepath) {
	try {
		YAML::Node data = YAML::LoadFile(filepath);
		return data;
	} catch (const YAML::ParserException& e) {
		Log::Log(severity::ERROR,
		         "YAML syntax error in file" + filepath.string() + ": " +
		             static_cast<std::string>(e.what()),
		         logopt::WAIT);
	} catch (const YAML::BadFile& e) {
		Log::Log(severity::ERROR, "Could not open file: " + filepath.string(), logopt::WAIT);
	}
	return std::nullopt;
}

std::optional<YAML::Node> load_yaml(const std::string& yaml, std::string source) {
	try {
		YAML::Node data = YAML::Load(yaml);
		return data;
	} catch (const YAML::ParserException& e) {
		if (!source.empty())
			source = " in file " + source;
		Log::Log(severity::ERROR,
		         "YAML syntax error" + source + ": " + static_cast<std::string>(e.what()),
		         logopt::WAIT);
	}
	return std::nullopt;
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
