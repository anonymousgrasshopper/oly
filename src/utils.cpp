#include <cstdlib>
#include <fstream>
#include <print>
#include <ranges>
#include <regex>
#include <stdexcept>

#include "oly/config.hpp"
#include "oly/log.hpp"
#include "oly/utils.hpp"

namespace utils {
void print_help() {
	std::println("usage: oly <cmd> [args [...]].\n");
	std::println(R"(Available subcommands:
    add                          - add a problem to the database
    edit                         - edit an entry in the database
    gen                          - generate a LaTeX or PDF file from a problem
    search                       - search problems by contest, metadata...
    show                         - print a problem statement
    alias                        - link a problem to another one
    rm                           - remove a problem and its solution file
  Run oly <cmd> --help for more information regarding a specific subcommand
)");
	std::println(R"(Arguments:
    --help              -h       - Show this help message
    --config-file FILE  -c FILE  - Specify config file to use
    --verify-config              - Check wether the config has any errors
    --version           -v       - Print this binary's version
    --log-level LEVEL            - Set the log level)");
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

void edit(const fs::path& filepath, std::string editor) {
	if (editor == "")
		editor = config["editor"].as<std::string>();
	std::string cmd = editor + " \"" + filepath.string() + "\"";
	if (filepath.string().contains('"'))
		throw std::invalid_argument("double quotes not allowed in file paths !");

	std::system(cmd.c_str());
}

void overwrite_file(const fs::path& filepath, const std::string& content) {
	std::ofstream file(filepath, std::ios::trunc);
	if (!file) {
		Log::ERROR("Could not open file: " + filepath.string(), logopt::WAIT);
	}

	file << content;
	file.close();
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
		Log::ERROR(level + "is not a valid log level. Using default severity INFO.");
		Log::log_level = severity::INFO;
	}
}

std::string get_topic(const char& letter) {
	switch (letter) {
	case 'A':
		return "Algebra";
		break;
	case 'C':
		return "Combinatorics";
		break;
	case 'G':
		return "Geometry";
		break;
	case 'N':
		return "Number Theory";
		break;
	default:
		return "";
	}
}

std::string get_problem_id(const std::string& source) {
	std::smatch match;
	std::string contest, year, problem;

	// 1. Contest name: a string of letters and spaces beginning and ending with letters
	std::regex contest_regex(R"(\b([A-Za-z](?:[A-Za-z ]*)[A-Za-z])\b)");
	if (std::regex_search(source, match, contest_regex)) {
		contest = match.str(1);
		if (contest.length() < 4 && contest.find(' ') == std::string::npos) {
			std::transform(contest.begin(), contest.end(), contest.begin(), ::toupper);
		} else {
			for (size_t i = 0; i < contest.length(); ++i) {
				if (i == 0 || contest[i - 1] == ' ') {
					contest[i] = std::toupper(contest[i]);
				} else {
					contest[i] = std::tolower(contest[i]);
				}
			}
		}
		if (config["abbreviations"][contest]) {
			contest = config["abbreviations"][contest].as<std::string>();
		}
		contest.append(" ");
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

	// 3. Problem: single digit, preceded by non-digit or letter, not followed by digit
	std::regex problem_regex(R"((\b[A-Za-z]?\d\b))");
	if (std::regex_search(source, match, problem_regex)) {
		if (match.str(1).size() == 1) {
			problem = "P" + match.str(1);
		} else if (match.str(1).size() == 2) {
			problem = match.str(1);
			problem[0] = std::toupper(problem[0]);
			config["topic"] = get_topic(problem[1]);
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

	std::regex yaml_pattern(R"(^[A-Za-z]+:\s*.+$)");
	return std::regex_match(line, yaml_pattern);
}

std::optional<YAML::Node> load_yaml(const fs::path& filepath) {
	try {
		YAML::Node data = YAML::LoadFile(filepath);
		return data;
	} catch (const YAML::ParserException& e) {
		Log::ERROR("YAML syntax error in file" + filepath.string() + ": " +
		               static_cast<std::string>(e.what()),
		           logopt::WAIT);
	} catch (const YAML::BadFile& e) {
		Log::ERROR("Could not open file: " + filepath.string(), logopt::WAIT);
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
		Log::ERROR("YAML syntax error" + source + ": " + static_cast<std::string>(e.what()),
		           logopt::WAIT);
	}
	return std::nullopt;
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

void input_file::create_file() {
	fs::create_directories(filepath.parent_path());
	std::ofstream out(filepath);
	out << contents;
	out.close();
}

std::deque<std::string> input_file::lines() {
	std::ifstream file(filepath);
	std::deque<std::string> lines;
	std::string line;
	if (!file.is_open())
		Log::CRITICAL("unable to open " + filepath.string() + "!");

	while (getline(file, line)) {
		lines.push_back(line);
	}
	return lines;
}

std::string input_file::filter_top_lines(const std::regex& reg) {
	std::ifstream file(filepath);
	std::string lines;
	std::string line;
	if (!file.is_open())
		Log::CRITICAL("unable to open " + filepath.string() + "!");

	while (getline(file, line)) {
		if (!regex_search(line, reg)) {
			lines.append(line + '\n');
			break;
		}
	}
	while (getline(file, line)) {
		lines.append(line + '\n');
	}
	return lines;
}

void input_file::edit() {
	std::string cmd = config["editor"].as<std::string>() + " \"" + filepath.string() + "\"";
	if (filepath.string().contains('"'))
		throw std::invalid_argument("double quotes not allowed in file paths !");

	std::system(cmd.c_str());
}

namespace preview {
void create_preview_file() {
	fs::path preview_file_path("/tmp/oly/" + config["source"].as<std::string>() + "/" +
	                           "preview.tex");
	const std::string PREVIEW_FILE_CONTENTS =
	    utils::expand_vars(R"(\documentclass[11pt]{scrartcl}
\usepackage[sexy,diagrams]{evan}
\author{${author}}
\title{${source}}
\begin{document}
\input{/tmp/oly/${source}/solution.tex}
\end{document}
)");
	utils::create_file(preview_file_path, PREVIEW_FILE_CONTENTS);
}
} // namespace preview
} // namespace utils
