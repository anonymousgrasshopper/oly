#pragma once

#include "oly/cmds/command.hpp"

class Remove : public Command {
public:
	Remove();
	int execute() override;

	static constexpr const std::string cmd_name = "rm";
};
