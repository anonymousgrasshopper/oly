#pragma once

#include <string>

#include "yaml-cpp/yaml.h"

YAML::Node load_config(std::string config_file_path = "")
