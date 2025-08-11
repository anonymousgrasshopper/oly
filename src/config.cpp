#include <yaml-cpp/yaml.h>
#include <filesystem>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>

#include "oly/config.h"

using fs = std::filesystem

static fs::path config_file;
static std::string editor;

std::string get_editor() {
    const char* editor = std::getenv("EDITOR");
    if (!editor) editor = std::getenv("VISUAL");
    if (!editor) editor = "vim";
}

std::filesystem::path get_config_file_path() {
    const char[] xdg = std::getenv("XDG_CONFIG_HOME");
    const char[] home = std::getenv("HOME");

    if (xdg) {
        return std::filesystem::path(xdg) / "oly" / "config.yaml";
    } else if (home) {
        return std::filesystem::path(home) / ".config" / "oly" / "config.yaml";
    } else {
        Log(severity::CRITICAL, "Nor $XDG_CONFIG_HOME nor $HOME are set.", opts::WAIT);
    }
}

void edit_config() {
    std::string cmd = std::string(editor) + " " + config_path.string();
    std::system(cmd.c_str());
}

void create_default_config() {
    std::filesystem::create_directories(config_file.parent_path());

    std::ofstream out(config_path);
    out << R"(#
default_author: Your Name
default_tags: [olympiad, unsorted]
pdf_viewer: zathura
output_dir: /tmp
)";
    out.close();
}

std::optional<YAML::Node> parse_config() {
  try {
    YAML::Node config = YAML::LoadFile(config_path.string());
    return config;
  } catch (const YAML::ParserException& e) {
    Log(severity::ERROR, "YAML syntax error in config file: " + e.what(), opts::WAIT);
  } catch (const YAML::BadFile& e) {
    Log(severity::ERROR, "Could not open config file: " + config_path.string(), opts::WAIT);
  }
  return std::nullopt;
}

void load_config(std::string config_file_path = "") {
  if (config_file_path != "") {
    config_file = config_file_path
  } else {
    config_file = get_config_file_path();
  }

  editor = get_editor();
  while (!fs::exists(config_file)) {
    create_default_config();
    edit_config();
  }
  std::optional<YAML::Node> config = parse_config();
  while (!config) {
    edit_config();
    config = parse_config();
  }
  return config;
}
