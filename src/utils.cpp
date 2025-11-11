#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <print>
#include <regex>
#include <stdexcept>

#include "oly/config.hpp"
#include "oly/log.hpp"
#include "oly/utils.hpp"

namespace fs = std::filesystem;

namespace utils {
void print_help() {
	std::println("usage: oly <cmd> [args [...]].\n");
	std::println(R"(Available subcommands:
    add                          - add a problem to the database
    edit                         - edit an entry from the database
    gen                          - generate a PDF from a problem
    search                       - search problems by contest, metadata...
    show                         - print a problem statement
    alias                        - link a problem to another one
    rm                           - remove a problem and its solution file
		mv                           - rename a problem
  Run oly <cmd> --help for more information regarding a specific subcommand
)");
	std::println(R"(Arguments:
    --help              -h       - Show this help message
    --config-file FILE  -c FILE  - Specify config file to use
    --verify-config              - Check wether the config has any errors
    --version           -v       - Print this binary's version
    --log-level LEVEL            - Set the log level)");
}

std::string expand_vars(const std::string& str, bool expand_config_vars,
                        bool expand_env_vars) {
	auto formatter = [&](const std::string& match) -> std::string {
		if (expand_config_vars && config[match]) {
			return config[match].as<std::string>();
		} else if (expand_env_vars) {
			const char* s = getenv(match.c_str());
			return (s == NULL ? "" : s);
		} else {
			return "";
		}
	};

	std::string fmt = utils::expand_vars(str, formatter);

	if (expand_env_vars && fmt.starts_with("~/")) {
		const char* home = getenv("HOME");
		fmt = std::string(home == NULL ? "" : home) + fmt.substr(1);
	}

	return fmt;
};

std::string expand_env_vars(const std::string& str) {
	return utils::expand_vars(str, false, true);
}

std::string filetype_extension() {
	return config["language"].as<std::string>() == "latex" ? ".tex" : ".typ";
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

bool is_separator(const std::string& line) {
	std::regex separator_pattern;
	if (config["lang"].as<std::string>() == "latex") {
		separator_pattern = R"(^\\hrulebar\s*$)";
	} else {
		separator_pattern = R"(^#hrule\s*$)";
	}
	return std::regex_match(line, separator_pattern);
}

bool is_yaml(const std::string& line) {
	std::regex yaml_pattern(R"(^[A-Za-z]+:\s*.+$)");
	return std::regex_match(line, yaml_pattern);
}

bool should_ignore(const std::string& line) {
	if (config["language"].as<std::string>() == "typst") {
		if (line.starts_with("#import")) {
			return true;
		}
	}
	if (std::regex_match(line, std::regex(R"(^\s*$)"))) {
		return true;
	}
	return false;
}

std::optional<YAML::Node> load_yaml(const fs::path& filepath) {
	try {
		YAML::Node data = YAML::LoadFile(filepath);
		return data;
	} catch (const YAML::ParserException& e) {
		Log::ERROR("YAML syntax error in file " + filepath.string() + ": " +
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
		Log::ERROR("YAML syntax error " + source + ": " + static_cast<std::string>(e.what()),
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
	                           "preview" + filetype_extension());
	if (config["language"].as<std::string>() == "latex") {
		constexpr char PREVIEW_FILE_CONTENTS[] = {
#embed "../assets/preview.tex"
		};
		constexpr size_t PREVIEW_FILE_SIZE = sizeof(PREVIEW_FILE_CONTENTS);
		std::string default_config(PREVIEW_FILE_CONTENTS, PREVIEW_FILE_SIZE);
		utils::create_file(preview_file_path, utils::expand_vars(default_config));
	} else {
		constexpr char PREVIEW_FILE_CONTENTS[] = {
#embed "../assets/preview.typ"
		};
		constexpr size_t PREVIEW_FILE_SIZE = sizeof(PREVIEW_FILE_CONTENTS);
		std::string default_config(PREVIEW_FILE_CONTENTS, PREVIEW_FILE_SIZE);
		utils::create_file(preview_file_path, utils::expand_vars(default_config));
	}
}
} // namespace preview
} // namespace utils
