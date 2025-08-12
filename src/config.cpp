#include <filesystem>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>

#include "yaml-cpp/yaml.h"
#include "oly/config.hpp"
#include "oly/log.hpp"

using fs = std::filesystem

static fs::path config_file;
static std::string editor;

static std::string get_editor() {
    const char* editor = std::getenv("EDITOR");
    if (!editor) editor = std::getenv("VISUAL");
    if (!editor) editor = "vim";
    return editor;
}

static std::filesystem::path get_config_file_path() {
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

static void edit_config() {
    std::string cmd = editor + " " + config_file.string();
    std::system(cmd.c_str());
}

static void create_default_config() {
    std::filesystem::create_directories(config_file.parent_path());
    inline constexpr char DEFAULT_CONFIG_BYTES[] = {
#embed "../assets/default_config.yaml"
    };
    std::ofstream out(config_file);
    out << DEFAULT_CONFIG_BYTES;
    out.close();
}

static std::optional<YAML::Node> parse_config() {
  try {
    YAML::Node config = YAML::LoadFile(config_file.string());
    return config;
  } catch (const YAML::ParserException& e) {
    Log(severity::ERROR, "YAML syntax error in config file: " + e.what(), opts::WAIT);
  } catch (const YAML::BadFile& e) {
    Log(severity::ERROR, "Could not open config file: " + config_file.string(), opts::WAIT);
  }
  return std::nullopt;
}

static bool has_required_fields(const YAML::Node& config) {
  std::vector<std::string> required_fields = { "author", "base_path" };
  for (const std::string& field : required_fields)
    if (!config[field])
        return false;
  return true;
}

YAML::Node load_config(std::string config_file_path = "") {
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
  while (!config && !check_required_fields(config)) {
    edit_config();
    config = parse_config();
  }
  if (!config["editor"])
      config["editor"] = editor;

  return config.value();
}
