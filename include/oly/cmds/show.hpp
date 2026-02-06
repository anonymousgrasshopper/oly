#pragma once

#include <filesystem>
#include <string>

#include "oly/cmds/command.hpp"

namespace fs = std::filesystem;

class Show : public Command {
private:
	void print_statement(const fs::path& source_path) const;
	std::string get_statement(const fs::path& pb) const;

public:
	Show();
	int execute() override;

	static constexpr const std::string cmd_name = "show";
};
