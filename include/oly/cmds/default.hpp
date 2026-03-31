#pragma once

#include "oly/cmds/command.hpp"

class Default : public Command {
private:
	static void print_version();

public:
	Default();
	int execute() override;

	static constexpr const std::string cmd_name = "oly";
};
