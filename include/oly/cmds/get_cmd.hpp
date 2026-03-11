#include <memory>
#include <string>

#include "oly/cmds/command.hpp"

std::unique_ptr<Command> get_cmd(const std::string& cmd_name);
