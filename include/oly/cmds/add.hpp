#pragma once

#include "oly/cmds/command.hpp"

class Add : public Command {
public:
	Add();
	int execute() override;

	static inline const std::string cmd_name = "add";
};
