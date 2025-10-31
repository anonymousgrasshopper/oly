#include <fstream>

#include "oly/cmds/edit.hpp"
#include "oly/config.hpp"
#include "oly/contest.hpp"
#include "oly/log.hpp"
#include "oly/utils.hpp"

Edit::Edit() {}

std::string Edit::comment_metadata(const fs::path& solution_path) const {
	std::ifstream solution_file(solution_path);
	if (!solution_file.is_open())
		Log::CRITICAL("Could not open " + solution_path.string() + "!");

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

std::string Edit::get_solution(const fs::path& source) const {
	utils::preview::create_preview_file();

	std::string solution = comment_metadata(source);

	std::string input = utils::input_file("/tmp/oly/" + config["source"].as<std::string>() +
	                                          "/solution" + utils::filetype_extension(),
	                                      solution, false)
	                        .filter_top_lines(std::regex("^\\s*$"));

	return uncomment_metadata(input);
}

void Edit::edit_problem(const fs::path& source) const {
	config["source"] = source.filename().string();
	std::string sol = get_solution(source);
	utils::overwrite_file(source, sol);
}

int Edit::execute() {
	load_config_file();

	for (std::string source : positional_args) {
		edit_problem(get_problem_path(source));
	}
	return 0;
}
