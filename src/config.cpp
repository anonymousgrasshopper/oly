#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <unordered_map>
#include <variant>

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
#embed "../assets/config.yaml"
	};
	constexpr size_t DEFAULT_CONFIG_SIZE = sizeof(DEFAULT_CONFIG_BYTES);
	std::string default_config(DEFAULT_CONFIG_BYTES, DEFAULT_CONFIG_SIZE);
	std::ofstream out(config_file);
	out << default_config;
	out.close();
}

static bool is_valid(const YAML::Node& config) {
	bool valid = true;
	std::vector<std::string> required_fields = {"author", "base_path", "pdf_viewer"};
	std::vector<std::string> missing_fields;

	for (const std::string& field : required_fields)
		if (!config[field])
			missing_fields.push_back(field);

	if (!missing_fields.empty()) {
		std::string missing = missing_fields[0];
		for (size_t i = 1; i < missing_fields.size(); i++) {
			if (i == missing_fields.size() - 1) {
				missing.append(" and " + missing_fields[i]);
			} else {
				missing.append(", " + missing_fields[i]);
			}
		}
		Log::ERROR(missing + " must be configured in config.yaml");
		valid = false;
	}

	if (config["language"]) {
		if (!config["language"].IsScalar() ||
		    (config["language"].as<std::string>() != "latex" &&
		     config["language"].as<std::string>() != "typst")) {
			Log::ERROR("language has to be one of latex or typst");
			valid = false;
		}
	}

	if (!valid) {
		Log::Wait();
	}

	return valid;
}

static void add_defaults(YAML::Node& config) {
	std::unordered_map<std::string, std::variant<bool, std::string>> default_options = {
	    {"editor", editor},          {"output_directory", "~/.cache/oly/${source}"},
	    {"separator", "\\hrulebar"}, {"preview", true},
	    {"confirm", false},          {"language", "latex"}};
	for (auto [key, value] : default_options) {
		if (!config[key]) {
			if (std::holds_alternative<bool>(value)) {
				config[key] = std::get<bool>(value);
			} else {
				config[key] = std::get<std::string>(value);
			}
		}
	}

	if (!config["preamble"]) {
		if (config["language"].as<std::string>() == "latex") {
			constexpr char DEFAULT_PREAMBLE_BYTES[] = {
#embed "../assets/preamble.tex"
			};
			constexpr size_t DEFAULT_PREAMBLE_SIZE = sizeof(DEFAULT_PREAMBLE_BYTES);
			std::string preamble(DEFAULT_PREAMBLE_BYTES, DEFAULT_PREAMBLE_SIZE);
			config["preamble"] = preamble;
		} else {
			constexpr char DEFAULT_PREAMBLE_BYTES[] = {
#embed "../assets/preamble.typ"
			};
			constexpr size_t DEFAULT_PREAMBLE_SIZE = sizeof(DEFAULT_PREAMBLE_BYTES);
			std::string preamble(DEFAULT_PREAMBLE_BYTES, DEFAULT_PREAMBLE_SIZE);
			config["preamble"] = preamble;
		}
	}

	if (!config["contents"]) {
		if (config["language"].as<std::string>() == "latex") {
			constexpr char DEFAULT_CONTENTS_BYTES[] = {
#embed "../assets/contents.tex"
			};
			constexpr size_t DEFAULT_CONTENTS_SIZE = sizeof(DEFAULT_CONTENTS_BYTES);
			std::string contents(DEFAULT_CONTENTS_BYTES, DEFAULT_CONTENTS_SIZE);
			config["contents"] = contents;
		} else {
			constexpr char DEFAULT_CONTENTS_BYTES[] = {
#embed "../assets/contents.typ"
			};
			constexpr size_t DEFAULT_CONTENTS_SIZE = sizeof(DEFAULT_CONTENTS_BYTES);
			std::string contents(DEFAULT_CONTENTS_BYTES, DEFAULT_CONTENTS_SIZE);
			config["contents"] = contents;
		}
	}

	if (!config["metadata"]) {
		constexpr char DEFAULT_METADATA_BYTES[] = {
#embed "../assets/metadata.yaml"
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
	while (!config || !is_valid(config.value())) {
		utils::edit(config_file, editor);
		config = utils::load_yaml(config_file);
	}

	add_defaults(config.value());

	return config.value();
}
