#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>

#include "oly/config.hpp"
#include "oly/log.hpp"
#include "oly/utils.hpp"

#include "yaml-cpp/yaml.h"

namespace fs = std::filesystem;

static fs::path config_file;
static std::string editor;

static std::string get_editor() {
	const char* editor = std::getenv("EDITOR");
	if (!editor)
		editor = std::getenv("VISUAL");
	if (!editor)
		editor = "vim";
	return editor;
}

static fs::path get_config_file_path() {
	const char* xdg = std::getenv("XDG_CONFIG_HOME");
	const char* home = std::getenv("HOME");

	if (xdg) {
		return fs::path(xdg) / "oly" / "config.yaml";
	} else if (home) {
		return fs::path(home) / ".config" / "oly" / "config.yaml";
	} else {
		Log::Log(severity::CRITICAL, "Nor $XDG_CONFIG_HOME nor $HOME are set.");
	}
}

static void edit_config() {
	std::string cmd = editor + " " + config_file.string();
	std::system(cmd.c_str());
}

static void create_default_config() {
	fs::create_directories(config_file.parent_path());
	constexpr char DEFAULT_CONFIG_BYTES[] = {
#embed "../assets/default_config.yaml"
	};
	std::ofstream out(config_file);
	out << DEFAULT_CONFIG_BYTES;
	out.close();
}

static std::optional<YAML::Node> parse_config() {
	try {
		YAML::Node config = YAML::LoadFile(config_file);
		return config;
	} catch (const YAML::ParserException& e) {
		Log::Log(severity::ERROR,
		         "YAML syntax error in config file: " + static_cast<std::string>(e.what()),
		         logopt::WAIT);
	} catch (const YAML::BadFile& e) {
		Log::Log(severity::ERROR, "Could not open config file: " + config_file.string(),
		         logopt::WAIT);
	}
	return std::nullopt;
}

static bool has_required_fields(const std::optional<YAML::Node>& config) {
	std::vector<std::string> required_fields = {"author", "base_path"};
	for (const std::string& field : required_fields)
		if (!config.value()[field]) {
			Log::Log(severity::ERROR,
			         "the " + field + " field must be configured in config.yaml", logopt::WAIT);
			return false;
		}
	return true;
}

void add_defaults(YAML::Node& config) {
	if (!config["editor"])
		config["editor"] = editor;

	if (!config["preamble"]) {
		constexpr char DEFAULT_PREAMBLE_BYTES[] = {
#embed "../assets/default_preamble.tex"
		};
		config["preamble"] = DEFAULT_PREAMBLE_BYTES;
	}

	if (!config["metadata"]) {
		constexpr char DEFAULT_METADATA_BYTES[] = {
#embed "../assets/default_metadata.yaml"
		};
		config["metadata"] = DEFAULT_METADATA_BYTES;
	}
}

YAML::Node load_config(std::string config_file_path) {
	if (config_file_path != "") {
		config_file = expand_env_vars(config_file_path);
	} else {
		config_file = get_config_file_path();
	}

	editor = get_editor();
	while (!fs::exists(config_file)) {
		create_default_config();
		edit_config();
	}

	std::optional<YAML::Node> config = parse_config();
	while (!config || !has_required_fields(config)) {
		edit_config();
		config = parse_config();
	}

	add_defaults(config.value());
	// yaml-cpp sometimes parses an incorrect file successfully, resulting in BIG TROUBLE

	return config.value();
}
