#include <iostream>

#include "oly/cmds/search.hpp"
#include "oly/utils.hpp"

Search::Search() {}

int Search::execute() {
	load_config_file();

	if (positional_args.empty()) {
		for (const std::string& problem : utils::prompt_user_for_problems()) {
			std::cout << problem << '\n';
		}
	}
	return 0;
}
