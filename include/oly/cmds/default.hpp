#pragma once

#include "oly/cmds/command.hpp"

class Default : public Command {
public:
	Default();
	int execute() override;

	static inline const std::string cmd_name = "";
};
