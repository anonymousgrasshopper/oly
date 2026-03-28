#include <memory>
#include <string>

#include "oly/cmds/add.hpp"
#include "oly/cmds/alias.hpp"
#include "oly/cmds/default.hpp"
#include "oly/cmds/edit.hpp"
#include "oly/cmds/generate.hpp"
#include "oly/cmds/get_cmd.hpp"
#include "oly/cmds/list.hpp"
#include "oly/cmds/remove.hpp"
#include "oly/cmds/rename.hpp"
#include "oly/cmds/search.hpp"
#include "oly/cmds/show.hpp"
#include "oly/log.hpp"

std::unique_ptr<Command> get_cmd(const std::string& cmd_name) {
	std::unique_ptr<Command> cmd;

	if (cmd_name == Add::cmd_name) {
		cmd = std::make_unique<Add>();
	} else if (cmd_name == Edit::cmd_name) {
		cmd = std::make_unique<Edit>();
	} else if (cmd_name == Generate::cmd_name) {
		cmd = std::make_unique<Generate>();
	} else if (cmd_name == List::cmd_name) {
		cmd = std::make_unique<List>();
	} else if (cmd_name == Search::cmd_name) {
		cmd = std::make_unique<Search>();
	} else if (cmd_name == Show::cmd_name) {
		cmd = std::make_unique<Show>();
	} else if (cmd_name == Alias::cmd_name) {
		cmd = std::make_unique<Alias>();
	} else if (cmd_name == Remove::cmd_name) {
		cmd = std::make_unique<Remove>();
	} else if (cmd_name == Rename::cmd_name) {
		cmd = std::make_unique<Rename>();
	} else if (cmd_name.starts_with("-")) {
		cmd = std::make_unique<Default>();
	} else {
		Log::CRITICAL("Unrecognized subcommand: " + cmd_name,
		              logopt::HELP | logopt::NO_PREFIX);
	}

	return cmd;
}
