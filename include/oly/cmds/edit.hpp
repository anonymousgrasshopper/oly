#pragma once

#include <filesystem>

#include "oly/cmds/command.hpp"

namespace fs = std::filesystem;

class Edit : public Command {
private:
	void edit_problem(const fs::path& source) const;
	std::string get_solution(const fs::path& source) const;

	std::string comment_metadata(const fs::path& source) const;
	std::string uncomment_metadata(std::string& input) const;

public:
	Edit();
	int execute() override;

	static constexpr const std::string cmd_name = "edit";
};
