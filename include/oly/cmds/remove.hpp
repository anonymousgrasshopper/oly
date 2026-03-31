#pragma once

#include <filesystem>

#include "oly/cmds/command.hpp"

class Remove : public Command {
private:
	bool prompt_before_deletion(const std::filesystem::path& path);

	void delete_problem(const std::filesystem::path& path);

public:
	Remove();
	int execute() override;

	static constexpr const std::string cmd_name = "rm";
};
