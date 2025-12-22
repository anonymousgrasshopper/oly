#include <iostream>

#include "oly/cmds/search.hpp"
#include "oly/log.hpp"
#include "oly/utils.hpp"

Search::Search() {}

int Search::execute() {
	if (positional_args.empty()) {
		for (const std::string& problem : utils::prompt_user_for_problems()) {
			std::cout << problem << '\n';
		}
	}
	return 0;
}
