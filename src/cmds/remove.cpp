#include <filesystem>
#include <iostream>
#include <print>

#include "oly/cmds/remove.hpp"
#include "oly/config.hpp"
#include "oly/contest.hpp"
#include "oly/log.hpp"
#include "oly/utils.hpp"

namespace fs = std::filesystem;

Remove::Remove() {
	add("--confirm,-i", "prompt before deleting file", [] { config["confirm"] = true; });
	add("--force,-f", "do not prompt before deleting file",
	    [] { config["confirm"] = false; });
}

bool Remove::prompt_before_deletion(const fs::path& path) {
	std::print("Are you sure you want to remove '{}' ? [y/n] ", path.string());

	std::string input;
	if (!std::getline(std::cin, input)) {
		return false; // EOF or error
	}

	if (input.empty()) {
		return false; // default is No
	}

	char c = static_cast<char>(std::tolower(input[0]));
	if (c == 'y')
		return true;

	return false;
}

void Remove::remove_pb(const fs::path& path) {
	if (!fs::exists(path)) {
		Log::ERROR(path.string() + " doesn't exist !");
	} else {
		if (prompt_before_deletion(path)) {
			if (!fs::remove(path)) {
				Log::ERROR(path.string() + " couldn't be removed...");
			}
		}
	}
}

int Remove::execute() {
	load_config_file();

	if (positional_args.empty()) {
		for (const std::string& problem : utils::prompt_user_for_problems()) {
			positional_args.push_back(problem);
		}
	}

	for (const std::string& arg : positional_args) {
		remove_pb(get_problem_path(arg));
	}
	return 0;
}
