#pragma once

#include <string>

#include "yaml-cpp/yaml.h"

namespace configuration {
void load_config(std::string config_file_path = "");

void merge_config(const YAML::Node& node, bool override = true);

} // namespace configuration

inline YAML::Node config;
