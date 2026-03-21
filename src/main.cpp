#include <memory>
#include <string>
#include <vector>

#include "oly/cmds/command.hpp"
#include "oly/cmds/get_cmd.hpp"
#include "oly/config.hpp"
#include "oly/utils.hpp"

int main(int argc, char* argv[]) {
	std::vector<std::string> args{argv + 1, argv + argc};

	if (args.empty()) {
		utils::print_help();
		return -1;
	}

	std::unique_ptr<Command> cmd;

	cmd = get_cmd(args[0]);

	if (!args[0].starts_with('-')) {
		shared["cmd"] = args[0];
		args.erase(args.begin());
	} else {
		shared["cmd"] = "default";
	}

	setenv("OLY", shared["cmd"].c_str(), 1);

	return cmd->execute(args);
}
