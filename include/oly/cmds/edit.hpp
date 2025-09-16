#pragma once

#include "oly/cmds/command.hpp"

class Edit : public Command {
private:
	void edit_problem(const std::string& source) const;
	std::string get_solution() const;

	std::string comment_metadata() const;
	std::string uncomment_metadata(std::string& input) const;

public:
	Edit();
	int execute() override;

	static constexpr const std::string cmd_name = "edit";
};
