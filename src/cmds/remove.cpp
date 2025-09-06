#include <filesystem>

#include "oly/cmds/remove.hpp"
#include "oly/log.hpp"
#include "oly/utils.hpp"

namespace fs = std::filesystem;

Remove::Remove() {
	add("--confirm,-i", "prompt before deleting file", [] { config["confirm"] = true; });
	add("--force,-f", "do not prompt before deleting file",
	    [] { config["confirm"] = false; });
}

int Remove::execute() {
	load_config_file();

	for (const std::string& arg : positional_args) {
		fs::path path = utils::get_problem_path(arg);
		if (!fs::remove(path)) {
			if (!fs::exists(path))
				Log::ERROR(path.string() + " doesn't exist !");
			else
				Log::ERROR(path.string() + " couldn't be removed...");
		}
	}
	return 0;
}
