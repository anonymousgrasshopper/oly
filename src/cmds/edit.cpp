#include <fstream>

#include "oly/cmds/edit.hpp"
#include "oly/config.hpp"
#include "oly/contest.hpp"
#include "oly/log.hpp"
#include "oly/utils.hpp"

Edit::Edit() {}

std::string Edit::parse_and_comment_metadata(const fs::path& solution_path) const {
	std::ifstream solution_file(solution_path);
	if (!solution_file.is_open())
		Log::CRITICAL("Could not open " + solution_path.string() + "!");

	std::string solution;
	std::string line;
	std::string metadata;
	if (get<std::string>("lang") == "typst")
		solution += "/*\n";
	while (getline(solution_file, line)) {
		if (!utils::is_yaml(line)) {
			if (get<std::string>("lang") == "typst")
				solution += "*/\n";
			solution += (line + '\n');
			break;
		} else {
			if (get<std::string>("lang") == "latex")
				solution += "% ";
			solution += (line + '\n');
			metadata += (line + '\n');
		}
	}
	while (getline(solution_file, line)) {
		solution += (line + '\n');
	}
	configuration::merge_config(YAML::Load(metadata));

	return solution;
}

std::string Edit::uncomment_metadata(std::string& input) const {
	if (get<std::string>("lang") == "typst") {
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
	std::string solution = parse_and_comment_metadata(source);

	utils::preview::create_preview_file();

	std::string pb_name = config["source"].as<std::string>();

	const fs::path tmp_path =
	    static_cast<fs::path>(get<std::string>("OLY_TMPDIR")) / pb_name;

	utils::figures::copy(tmp_path, source.parent_path());

	std::string input =
	    utils::input_file(tmp_path / ("solution" + utils::filetype_extension()), solution,
	                      false)
	        .filter_top_lines(std::regex("^\\s*$"));

	utils::figures::save(tmp_path, source.parent_path());

	return uncomment_metadata(input);
}

void Edit::edit_problem(const std::string& pb) const {
	const fs::path solution_path = get_problem_solution_path(pb);
	config["source"] = get_problem_name(pb);
	std::string sol = get_solution(solution_path);
	utils::file::overwrite(solution_path, sol);
}

int Edit::execute() {
	load_config_file();

	if (positional_args.empty()) {
		for (const std::string& problem : utils::prompt_user_for_problems()) {
			positional_args.push_back(problem);
		}
	}

	for (std::string source : positional_args)
		edit_problem(source);

	return 0;
}
