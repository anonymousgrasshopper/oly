#include <fstream>

#include "oly/cmds/edit.hpp"
#include "oly/config.hpp"
#include "oly/utils.hpp"

Edit::Edit() {}

std::string Edit::get_solution() const {
	utils::preview::create_preview_file();

	std::ifstream solution_file(
	    utils::get_problem_path(config["source"].as<std::string>()));
	std::stringstream old_content;
	old_content << solution_file.rdbuf();
	solution_file.close();

	std::string input = utils::input_file("/tmp/oly/" + config["source"].as<std::string>() +
	                                          "/solution." + utils::filetype(),
	                                      old_content.str())
	                        .filter_top_lines(std::regex("^%.*$|^$"));
	return input;
}

void Edit::edit_problem(const std::string& source) const {
	config["source"] = source;
	std::string sol = get_solution();
	utils::overwrite_file(utils::get_problem_path(source), sol);
}

int Edit::execute() {
	load_config_file();

	for (std::string source : positional_args) {
		edit_problem(utils::get_problem_id(source));
	}
	return 0;
}
