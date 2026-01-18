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

	if (config["lang"]) {
		if (!config["lang"].IsScalar() || (config["lang"].as<std::string>() != "latex" &&
		                                   config["lang"].as<std::string>() != "typst")) {
			Log::ERROR("lang has to be one of latex or typst");
			valid = false;
		}
	}

	if (!valid) {
		Log::Wait();
	}

	return valid;
}

static void add_defaults(YAML::Node& config) {
	std::string cache_home;
	const char* xdg_cache_home = std::getenv("XDG_CACHE_HOME");
	if (!xdg_cache_home) {
		const char* home = std::getenv("HOME");
		if (!home) {
			Log::CRITICAL("Nor $HOME nor $XDG_CACHE_HOME are set !");
		} else {
			cache_home = static_cast<std::string>(home) + "/.cache";
		}
	} else {
		cache_home = static_cast<std::string>(xdg_cache_home);
	}
	const char* tmpdir = std::getenv("TMPDIR");
	if (!tmpdir) {
		tmpdir = "/tmp";
	}

	std::unordered_map<std::string, std::variant<bool, std::string>> default_options = {
	    {"lang", "latex"},
	    {"language", "en"},
	    {"editor", editor},
	    {"preview", true},
	    {"confirm", false},
	    {"output_directory", cache_home + "/oly/${source}"},
	    {"OLY_TMPDIR", static_cast<std::string>(tmpdir) + "/oly/"}};
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
		if (config["lang"].as<std::string>() == "latex") {
			constexpr char DEFAULT_PREAMBLE_BYTES[] = {
#embed "../assets/tex/preamble.tex"
			};
			constexpr size_t DEFAULT_PREAMBLE_SIZE = sizeof(DEFAULT_PREAMBLE_BYTES);
			std::string preamble(DEFAULT_PREAMBLE_BYTES, DEFAULT_PREAMBLE_SIZE);
			config["preamble"] = preamble;
		} else {
			constexpr char DEFAULT_PREAMBLE_BYTES[] = {
#embed "../assets/typst/preamble.typ"
			};
			constexpr size_t DEFAULT_PREAMBLE_SIZE = sizeof(DEFAULT_PREAMBLE_BYTES);
			std::string preamble(DEFAULT_PREAMBLE_BYTES, DEFAULT_PREAMBLE_SIZE);
			config["preamble"] = preamble;
		}
	}

	if (!config["contents"]) {
		if (config["lang"].as<std::string>() == "latex") {
			constexpr char DEFAULT_CONTENTS_BYTES[] = {
#embed "../assets/tex/contents.tex"
			};
			constexpr size_t DEFAULT_CONTENTS_SIZE = sizeof(DEFAULT_CONTENTS_BYTES);
			std::string contents(DEFAULT_CONTENTS_BYTES, DEFAULT_CONTENTS_SIZE);
			config["contents"] = contents;
		} else {
			constexpr char DEFAULT_CONTENTS_BYTES[] = {
#embed "../assets/typst/contents.typ"
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

	std::string base_path = config["base_path"].as<std::string>();
	if (base_path.starts_with("~")) {
		const char* home = getenv("HOME");
		if (home) {
			config["base_path"] = std::string(home) + base_path.substr(1);
		}
	}
}

namespace configuration {
void load_config(std::string config_file_path) {
	config_file = utils::expand_env_vars(config_file_path);

	editor = get_editor();
	while (!fs::exists(config_file)) {
		create_default_config();
		utils::edit(config_file, editor);
	}

	std::optional<YAML::Node> userconfig = utils::yaml::load(config_file);
	while (!userconfig || !is_valid(userconfig.value())) {
		utils::edit(config_file, editor);
		userconfig = utils::yaml::load(config_file);
	}

	merge_config(userconfig.value(), false);
	add_defaults(config);
}

void merge_config(const YAML::Node& extend, bool override) {
	if (!extend.IsDefined())
		return;

	if (extend.IsMap()) {
		for (auto it : extend) {
			const std::string key = it.first.as<std::string>();
			const YAML::Node& value = it.second;
			if (override || !config[key]) {
				config[key] = value;
			}
		}
	}
}
} // namespace configuration
