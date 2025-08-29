#pragma once

#include <string>

#include "yaml-cpp/node/node.h"

YAML::Node load_config(std::string config_file_path = "");

inline YAML::Node config;
