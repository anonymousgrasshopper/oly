#pragma once

#include "oly/cmds/command.hpp"

class Edit : public Command {
private:
	void edit_problem(const std::string& source) const;
	std::string get_solution() const;

public:
	Edit();
	int execute() override;

	static constexpr const std::string cmd_name = "edit";
};
