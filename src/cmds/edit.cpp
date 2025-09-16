#include <fstream>

#include "oly/cmds/edit.hpp"
#include "oly/config.hpp"
#include "oly/log.hpp"
#include "oly/utils.hpp"

Edit::Edit() {}

std::string Edit::comment_metadata() const {
	std::ifstream solution_file(
	    utils::get_problem_path(config["source"].as<std::string>()));
	if (!solution_file.is_open())
		Log::CRITICAL("Could not open " +
		              utils::get_problem_path(config["source"].as<std::string>()).string());

	std::string solution;
	std::string line;
	if (get<std::string>("language") == "typst")
		solution += "/*\n";
	while (getline(solution_file, line)) {
		if (!utils::is_yaml(line)) {
			if (get<std::string>("language") == "typst")
				solution += "*/\n";
			solution += (line + '\n');
			break;
		} else {
			if (get<std::string>("language") == "latex")
				solution += "% ";
			solution += (line + '\n');
		}
	}
	while (getline(solution_file, line)) {
		solution += (line + '\n');
	}
	return solution;
}

std::string Edit::uncomment_metadata(std::string& input) const {
	if (get<std::string>("language") == "typst") {
		if (input.starts_with("/*\n")) {
			input = input.substr(3);
		}
		unsigned long closing = input.find("*/\n");
		if (closing != std::string::npos) {
			input = input.substr(0, closing) + input.substr(closing + 3);
		}
	} else {
		static const std::regex re(R"(^\s*%\s*(.*)$)", std::regex::multiline);
		return std::regex_replace(input, re, "$1");
	}

	return input;
}

std::string Edit::get_solution() const {
	utils::preview::create_preview_file();

	std::string solution = comment_metadata();

	std::string input = utils::input_file("/tmp/oly/" + config["source"].as<std::string>() +
	                                          "/solution" + utils::filetype_extension(),
	                                      solution)
	                        .filter_top_lines(std::regex("^\\s*$"));

	return uncomment_metadata(input);
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
