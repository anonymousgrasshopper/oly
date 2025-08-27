#pragma once

#include <deque>
#include <filesystem>
namespace fs = std::filesystem;

void print_help();

std::string expand_vars(std::string str, bool expand_config_vars = true,
                        bool expand_env_vars = true);

std::string expand_env_vars(std::string str);

fs::path parse_pb_name(const std::string& pb_name);

fs::path get_path(const std::string& pb_name);

void create_file(const fs::path& filepath, const std::string& contents = "");

void set_log_level(std::string level);

struct input_file {
private:
	void create_file();
	bool remove;

public:
	fs::path filepath;
	std::string contents;

	input_file(fs::path filepath, std::string contents, bool remove = true);
	~input_file();

	std::deque<std::string> lines();

	void edit();
};
