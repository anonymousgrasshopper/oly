#include <filesystem>

#include "oly/cmds/alias.hpp"
#include "oly/contest.hpp"
#include "oly/log.hpp"

namespace fs = std::filesystem;

Alias::Alias() {}

void Alias::link(const fs::path& from, const fs::path& to) {
	try {
		fs::create_directories(to.parent_path());
		fs::create_symlink(from, to);
	} catch (const std::filesystem::filesystem_error& e) {
		Log::ERROR(e.what());
	}
}

int Alias::execute() {
	load_config_file();

	if (positional_args.size() == 0) {
		Log::ERROR("Expected source and and at least a destination", logopt::HELP);
		return 1;
	} else if (positional_args.size() == 1) {
		Log::ERROR("No destination provided", logopt::HELP);
		return 1;
	}

	fs::path target(get_problem_path(positional_args.front()));
	if (!fs::exists(target)) {
		Log::CRITICAL("cannot find " + target.string() + ": no such file or directory");
	}

	for (size_t i = 1; i < positional_args.size(); ++i) {
		link(target, get_problem_path(positional_args[i]));
	}
	return 0;
}
