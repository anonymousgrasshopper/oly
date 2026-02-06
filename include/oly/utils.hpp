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
std::string expand_vars(const std::string& str, auto&& f)
  requires requires(std::string& s) {
	  { f(s) } -> std::convertible_to<std::string>;
  };

[[nodiscard]]
std::string expand_vars(const std::string& str, bool expand_config_vars = true,
                        bool expand_env_vars = true);

[[nodiscard]]
std::string expand_env_vars(const std::string& str);

[[nodiscard]]
std::string filetype_extension();

void set_log_level(std::string level);

[[nodiscard]]
bool is_separator(const std::string& line);

[[nodiscard]]
bool is_yaml(const std::string& line);

[[nodiscard]]
bool should_ignore(const std::string& line);

bool copy_dir(const fs::path& from, const std::string& to);

[[nodiscard]]
std::vector<std::string> prompt_user_for_problems();

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

namespace file {
void create(const fs::path& filepath, const std::string& contents = "");

void edit(const fs::path& filepath, std::string editor = "");

void overwrite(const fs::path& filepath, const std::string& content);
} // namespace file

namespace yaml {
[[nodiscard]]
std::optional<YAML::Node> load(const fs::path& filepath);

[[nodiscard]]
std::optional<YAML::Node> load(const std::string& yaml, std::string source = "");
} // namespace yaml

namespace preview {
void create_preview_file();
}

namespace figures {
bool copy(const fs::path& tmp_path, const std::string& pb_name);

bool save(const fs::path& tmp_path, const std::string& pb_name);
} // namespace figures

} // namespace utils

std::string utils::expand_vars(const std::string& str, auto&& f)
  requires requires(std::string& s) {
	  { f(s) } -> std::convertible_to<std::string>;
  }
{
	std::string fmt = str;

	static std::regex var("\\$\\{([^}]+)\\}");
	std::smatch match;
	while (std::regex_search(fmt, match, var)) {
		fmt.replace(match[0].first, match[0].second, f(match[1].str()));
	}

	return fmt;
}
