#pragma once

#include "oly/cmds/command.hpp"

class Generate : public Command {
public:
	Generate();
	int execute() override;

	static inline const std::string cmd_name = "gen";
};
