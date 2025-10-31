#include <filesystem>
#include <print>

#include "oly/cmds/show.hpp"
#include "oly/contest.hpp"
#include "oly/log.hpp"

namespace fs = std::filesystem;

Show::Show() {}

void Show::print_statement(const std::string& pb) {
	fs::path path(get_problem_path(pb));
	if (!fs::exists(path)) {
		Log::ERROR(path.string() + "does not exist !");
	} else {
		std::println("sorry, not implemented yet !");
	}
}

int Show::execute() {
	for (const std::string& pb : positional_args) {
		print_statement(pb);
	}
	return 0;
}
