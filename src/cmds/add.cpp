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

std::string Add::get_solution_body(const fs::path& base_path) const {
	utils::preview::create_preview_file();
	std::string input =
	    utils::input_file(base_path / ("solution" + utils::filetype_extension()),
	                      utils::expand_vars(config["contents"].as<std::string>()), false)
	        .filter_top_lines(std::regex("^\\s*$"));
	return input;
}

YAML::Node Add::get_solution_metadata(const fs::path& base_path) const {
	utils::input_file file(base_path / "metadata.yaml",
	                       utils::expand_vars(config["metadata"].as<std::string>()));

	auto metadata = utils::yaml::load(file.filepath);
	while (!metadata) {
		file.edit();
		metadata = utils::yaml::load(file.filepath);
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

	const fs::path tmp_path =
	    static_cast<fs::path>(get<std::string>("OLY_TMPDIR")) / pb_name;
	std::string body = get_solution_body(tmp_path);
	YAML::Node metadata = get_solution_metadata(tmp_path);

	create_solution_file(pb, body, metadata);

	utils::save_figures(tmp_path, pb_name);
}

int Add::execute() {
	load_config_file();

	if (positional_args.size() == 0) {
		Log::ERROR("Expected problem name", logopt::HELP);
		return 1;
	}

	for (std::string source : positional_args)
		add_problem(get_problem_path(source));

	return 0;
}
