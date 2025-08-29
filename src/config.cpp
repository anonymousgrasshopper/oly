#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <unordered_map>

#include "oly/config.hpp"
#include "oly/log.hpp"
#include "oly/utils.hpp"

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

static void create_default_config() {
	fs::create_directories(config_file.parent_path());
	constexpr char DEFAULT_CONFIG_BYTES[] = {
#embed "../assets/default_config.yaml"
	};
	constexpr size_t DEFAULT_CONFIG_SIZE = sizeof(DEFAULT_CONFIG_BYTES);
	std::string default_config(DEFAULT_CONFIG_BYTES, DEFAULT_CONFIG_SIZE);
	std::ofstream out(config_file);
	out << default_config;
	out.close();
}

static bool has_required_fields(const std::optional<YAML::Node>& config) {
	std::vector<std::string> required_fields = {"author", "base_path", "pdf_viewer"};
	for (const std::string& field : required_fields)
		if (!config.value()[field]) {
			Log::ERROR("the " + field + " field must be configured in config.yaml",
			           logopt::WAIT);
			return false;
		}
	return true;
}

void add_defaults(YAML::Node& config) {
	std::unordered_map<std::string, std::string> default_options = {
	    {"editor", editor},
	    {"output_directory", "~/.cache/oly/${source}"},
	    {"separator", "---"}};
	for (auto [key, value] : default_options) {
		if (!config[key])
			config[key] = value;
	}

	if (!config["preamble"]) {
		constexpr char DEFAULT_PREAMBLE_BYTES[] = {
#embed "../assets/default_preamble.tex"
		};
		constexpr size_t DEFAULT_PREAMBLE_SIZE = sizeof(DEFAULT_PREAMBLE_BYTES);
		std::string preamble(DEFAULT_PREAMBLE_BYTES, DEFAULT_PREAMBLE_SIZE);
		config["preamble"] = preamble;
	}

	if (!config["metadata"]) {
		constexpr char DEFAULT_METADATA_BYTES[] = {
#embed "../assets/default_metadata.yaml"
		};
		constexpr size_t DEFAULT_METADATA_SIZE = sizeof(DEFAULT_METADATA_BYTES);
		std::string metadata(DEFAULT_METADATA_BYTES, DEFAULT_METADATA_SIZE);
		config["metadata"] = metadata;
	}
}

YAML::Node load_config(std::string config_file_path) {
	config_file = utils::expand_env_vars(config_file_path);

	editor = get_editor();
	while (!fs::exists(config_file)) {
		create_default_config();
		utils::edit(config_file, editor);
	}

	std::optional<YAML::Node> config = utils::load_yaml(config_file);
	while (!config || !has_required_fields(config)) {
		utils::edit(config_file, editor);
		config = utils::load_yaml(config_file);
	}

	Log::WARNING("ok");
	// add_defaults(config.value());
	// yaml-cpp sometimes parses an incorrect file successfully, resulting in BIG TROUBLE

	return config.value();
}
