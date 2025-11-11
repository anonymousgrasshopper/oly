#pragma once

#include "oly/cmds/command.hpp"

#include <filesystem>

namespace fs = std::filesystem;

class Rename : public Command {
private:
	void move(const fs::path& from, const fs::path& to);

public:
	Rename();
	int execute() override;

	static constexpr const std::string cmd_name = "mv";
};
