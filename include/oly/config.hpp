#pragma once

#include "yaml-cpp/node/node.h"
#include <string>

YAML::Node load_config(std::string config_file_path = "");

inline YAML::Node config;
