#pragma once

#include <deque>
#include <filesystem>
#include <regex>
#include <string>

#include "yaml-cpp/yaml.h"

namespace fs = std::filesystem;

namespace utils {
void print_help();

[[nodiscard]]
std::string expand_vars(std::string str, bool expand_config_vars = true,
                        bool expand_env_vars = true);

[[nodiscard]]
std::string expand_env_vars(std::string str);

[[nodiscard]]
std::string filetype();

void create_file(const fs::path& filepath, const std::string& contents = "");

void edit(const fs::path& filepath, std::string editor = "");

void overwrite_file(const fs::path& filepath, const std::string& content);

void set_log_level(std::string level);

[[nodiscard]]
std::string get_problem_id(const std::string& source);

[[nodiscard]]
fs::path get_problem_path(const std::string& source, const bool process_source = true);

[[nodiscard]]
bool is_separator(const std::string& line);

[[nodiscard]]
bool is_yaml(const std::string& line);

[[nodiscard]]
std::optional<YAML::Node> load_yaml(const fs::path& filepath);

[[nodiscard]]
std::optional<YAML::Node> load_yaml(const std::string& yaml, std::string source = "");

struct input_file {
private:
	void create_file();
	bool remove;

public:
	fs::path filepath;
	std::string contents;

	input_file(fs::path filepath, std::string contents, bool remove = true);
	~input_file();

	[[nodiscard]]
	std::deque<std::string> lines();

	[[nodiscard]]
	std::string filter_top_lines(const std::regex& reg);

	void edit();
};

namespace preview {
void create_preview_file();
}
} // namespace utils
