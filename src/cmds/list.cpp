#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>

#include "oly/cmds/list.hpp"
#include "oly/config.hpp"
#include "oly/log.hpp"
#include "oly/utils.hpp"

List::List() {}

std::optional<std::string> List::parse_metadata(const fs::path& solution_path) const {
	std::ifstream solution_file(solution_path);
	if (!solution_file.is_open())
		Log::CRITICAL("Could not open " + solution_path.string() + "!");

	std::string line;
	std::string metadata;
	while (getline(solution_file, line)) {
		if (utils::is_yaml(line)) {
			metadata += (line + '\n');
		} else {
			break;
		}
	}

	try {
		std::optional<YAML::Node> yaml = utils::yaml::load(metadata, solution_path.string());
		if (yaml.has_value()) {
			YAML::Node source = yaml.value()["source"];
			if (source && source.IsScalar()) {
				return source.as<std::string>();
			}
		}
	} catch (std::exception& e) {
		Log::ERROR(e.what());
	}
	return std::nullopt;
}

int List::execute(std::vector<std::string>& args) {
	load_config_file(args);
	parse(args);

	fs::path base_path = opts.base_path;
	try {
		for (const auto& entry : fs::recursive_directory_iterator(base_path)) {
			if (entry.is_regular_file()) {
				std::optional<std::string> source = parse_metadata(entry.path());
				if (source.has_value()) {
					std::cout << source.value() << '\n';
				}
			}
		}
		std::cout.flush();
	} catch (const fs::filesystem_error& e) {
		Log::ERROR(e.what());
	}
	return 0;
}
