#include <filesystem>
#include <string>

#include "oly/cmds/add.hpp"
#include "oly/config.hpp"
#include "oly/contest.hpp"
#include "oly/log.hpp"
#include "oly/utils.hpp"

namespace fs = std::filesystem;

Add::Add() {
	add("--overwrite,-o", "Overwrite previous database entry for the problem", false);
}

std::string Add::get_solution_body() const {
	utils::preview::create_preview_file();
	std::string input =
	    utils::input_file("/tmp/oly/" + config["source"].as<std::string>() + "/solution" +
	                          utils::filetype_extension(),
	                      utils::expand_vars(config["contents"].as<std::string>()), false)
	        .filter_top_lines(std::regex("^\\s*$"));
	return input;
}

YAML::Node Add::get_solution_metadata() const {
	utils::input_file file("/tmp/oly/" + config["source"].as<std::string>() +
	                           "/metadata.yaml",
	                       utils::expand_vars(config["metadata"].as<std::string>()));

	auto metadata = utils::load_yaml(file.filepath);
	while (!metadata) {
		file.edit();
		metadata = utils::load_yaml(file.filepath);
	}
	return metadata.value();
}

void Add::create_solution_file(const fs::path& path, const std::string& body,
                               const YAML::Node& metadata) const {
	std::string contents;

	YAML::Emitter out;
	out << metadata;
	contents.append(out.c_str());

	contents.append("\n\n");
	contents.append(body);
	utils::create_file(path, contents);
}

void Add::add_problem(const fs::path& pb) const {
	std::string pb_name = pb.stem().string();
	if (!get<bool>("--overwrite") && fs::exists(pb)) {
		Log::CRITICAL("cannot add " + pb_name + ": entry already present in database" + "\n" +
		              "Use oly edit " + pb_name + " to edit it" + "\n" +
		              "Or use --overwrite / -o to ignore this");
	}
	config["source"] = pb_name;
	std::string body = get_solution_body();
	YAML::Node metadata = get_solution_metadata();

	create_solution_file(pb, body, metadata);
}

int Add::execute() {
	load_config_file();

	for (std::string source : positional_args) {
		add_problem(get_problem_path(source));
	}
	return 0;
}
