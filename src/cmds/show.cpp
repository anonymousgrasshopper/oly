#include <filesystem>
#include <fstream>
#include <iostream>

#include "oly/cmds/show.hpp"
#include "oly/contest.hpp"
#include "oly/log.hpp"
#include "oly/utils.hpp"

namespace fs = std::filesystem;

Show::Show() {}

std::string Show::get_statement(const fs::path& pb) const {
	std::ifstream file(pb);
	if (!file.is_open())
		throw std::runtime_error("Could not open " + pb.string() + "!");

	std::string pb_statement;
	std::string line;
	while (getline(file, line)) {
		if (!utils::is_yaml(line) && !utils::should_ignore(line)) {
			if (!utils::is_separator(line)) {
				pb_statement += (line + '\n');
			}
			break;
		}
	}
	while (getline(file, line)) {
		if (utils::is_separator(line)) {
			return pb_statement;
		} else {
			pb_statement += (line + '\n');
		}
	}
	while (pb_statement.back() == '\n') {
		pb_statement.pop_back();
	}

	return pb_statement;
}

void Show::print_statement(const std::string& pb) const {
	fs::path path(get_problem_path(pb));
	if (!fs::exists(path)) {
		Log::ERROR(path.string() + "does not exist !");
	} else {
		std::cout << get_statement(path);
	}
}

int Show::execute() {
	load_config_file();

	for (const std::string& pb : positional_args) {
		print_statement(pb);
		if (&pb != &positional_args.back()) {
			std::cout << std::endl;
		}
	}
	return 0;
}
