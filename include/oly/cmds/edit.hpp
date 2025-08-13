#pragma once

#include "oly/cmds/command.hpp"

class Edit : public Command {
public:
	Edit();
	int execute() override;

	static inline const std::string cmd_name = "edit";
};
