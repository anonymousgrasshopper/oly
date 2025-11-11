#include <filesystem>

#include "oly/cmds/alias.hpp"
#include "oly/contest.hpp"
#include "oly/log.hpp"

namespace fs = std::filesystem;

Alias::Alias() {}

void Alias::link(const fs::path& from, const fs::path& to) {
	fs::create_symlink(from, to);
}

int Alias::execute() {
	load_config_file();

	fs::path target(get_problem_path(positional_args.front()));
	if (!fs::exists(target)) {
		Log::CRITICAL("cannot find " + target.string() + ": no such file or directory");
	}

	for (size_t i = 1; i < positional_args.size(); ++i) {
		link(target, get_problem_path(positional_args[i]));
	}
	return 0;
}
