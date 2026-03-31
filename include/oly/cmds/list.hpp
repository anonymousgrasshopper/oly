#pragma once

#include <filesystem>

#include "oly/cmds/command.hpp"

namespace fs = std::filesystem;

class List : public Command {
private:
	std::optional<std::string>
	parse_metadata_from_file(const fs::path& solution_path) const;

public:
	List();
	int execute() override;

	static constexpr const std::string cmd_name = "list";
};
