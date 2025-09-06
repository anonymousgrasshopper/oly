#pragma once

#include <filesystem>

#include "oly/cmds/command.hpp"

class Alias : public Command {
private:
	void link(const std::filesystem::path& from, const std::filesystem::path& to);

public:
	Alias();
	int execute() override;

	static constexpr const std::string cmd_name = "alias";
};
