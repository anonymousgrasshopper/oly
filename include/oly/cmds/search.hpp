#pragma once

#include "oly/cmds/command.hpp"

class Search : public Command {
public:
	Search();
	int execute() override;

	static constexpr const std::string cmd_name = "search";
};
