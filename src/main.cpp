#include <memory>
#include <string>
#include <vector>

#include "oly/cmds/add.hpp"
#include "oly/cmds/command.hpp"
#include "oly/cmds/default.hpp"
#include "oly/cmds/edit.hpp"
#include "oly/cmds/generate.hpp"
#include "oly/cmds/search.hpp"
#include "oly/cmds/show.hpp"
#include "oly/config.hpp"
#include "oly/log.hpp"
#include "oly/utils.hpp"

int main(int argc, char* argv[]) {
	std::vector<std::string> args{argv + 1, argv + argc};

	std::unique_ptr<Command> cmd;
	bool has_cmd_name{true};

	if (args.empty()) {
		utils::print_help();
		return -1;
	} else if (args[0] == Add::cmd_name) {
		cmd = std::make_unique<Add>();
	} else if (args[0] == Edit::cmd_name) {
		cmd = std::make_unique<Edit>();
	} else if (args[0] == Show::cmd_name) {
		cmd = std::make_unique<Show>();
	} else if (args[0] == Generate::cmd_name) {
		cmd = std::make_unique<Generate>();
	} else if (args[0] == Search::cmd_name) {
		cmd = std::make_unique<Search>();
	} else if (args[0].starts_with("-")) {
		cmd = std::make_unique<Default>();
		has_cmd_name = false;
	} else {
		Log::CRITICAL("Unrecognized subcommand: " + args[0],
		              logopt::HELP | logopt::NO_PREFIX);
	}

	if (has_cmd_name) {
		config["cmd"] = args[0];
		args.erase(args.begin());
	} else {
		config["cmd"] = "default";
	}

	setenv("OLY", config["cmd"].as<std::string>().c_str(), 1);

	cmd->parse(args);
	return cmd->execute();
}
