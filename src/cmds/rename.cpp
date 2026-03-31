#include <filesystem>

#include "oly/cmds/rename.hpp"
#include "oly/contest.hpp"
#include "oly/log.hpp"

namespace fs = std::filesystem;

Rename::Rename() {
	add("--alias,-a", "Alias the new file to the old one", false);
}

void Rename::move(const fs::path& from, const fs::path& to) {
	try {
		fs::create_directories(to.parent_path());
		fs::rename(from, to);
	} catch (const std::filesystem::filesystem_error& e) {
		Log::CRITICAL(e.what());
	}
	if (fs::exists(to)) {
		if (get<bool>("--alias")) {
			fs::create_symlink(to, from);
		}
	} else {
		Log::ERROR("cannot rename " + from.string() + " to " + to.string());
	}
}

int Rename::execute() {
	if (positional_args.empty()) {
		Log::CRITICAL("missing file operand", logopt::HELP | logopt::NO_PREFIX);
	} else if (positional_args.size() == 1) {
		Log::CRITICAL("missing destination file operand after '" + positional_args[0] + "'",
		              logopt::HELP | logopt::NO_PREFIX);
	} else if (positional_args.size() > 2) {
		Log::CRITICAL("too many arguments provided: expected exactly two",
		              logopt::HELP | logopt::NO_PREFIX);
	}

	fs::path target(get_problem_path(positional_args.front()));
	if (!fs::exists(target)) {
		Log::CRITICAL("cannot find " + target.string() + ": no such file or directory");
	}

	move(target, get_problem_path(positional_args[1]));
	return 0;
}
