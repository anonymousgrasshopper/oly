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
#include "oly/log.hpp"
#include "oly/utils.hpp"

int main(int argc, char* argv[]) {
	std::vector<std::string> args{argv + 1, argv + argc};

	std::unique_ptr<Command> cmd;
	bool remove_cmd_name{true};

	if (args.empty()) {
		print_help();
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
		remove_cmd_name = false;
	} else {
		Log::Log(severity::CRITICAL, "Unrecognized subcommand: " + args[0], logopt::HELP);
	}

	if (remove_cmd_name)
		args.erase(args.begin());

	cmd->parse(args);
	cmd->load_config_file();
	return cmd->execute();
}
