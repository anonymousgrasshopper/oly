#pragma once

#include "oly/cmds/command.hpp"

class Show : public Command {
private:
	void print_statement(const std::string& pb);

public:
	Show();
	int execute() override;

	static constexpr const std::string cmd_name = "show";
};
