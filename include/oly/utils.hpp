#pragma once

#include <filesystem>
namespace fs = std::filesystem;

void print_help();

std::string expand_env_vars(const std::string& str);

fs::path parse_pb_name(const std::string& pb_name);

fs::path get_path(const std::string& pb_name);

void create_file(const fs::path& filepath, const std::string& contents = "");

void set_log_level(std::string level);
