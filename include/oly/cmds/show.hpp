#pragma once

#include "oly/cmds/command.hpp"

class Show : public Command {
public:
	Show();
	int execute() override;

	static inline const std::string cmd_name = "show";
};
