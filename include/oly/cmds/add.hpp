#pragma once

#include <filesystem>

#include "oly/cmds/command.hpp"
#include "yaml-cpp/yaml.h"

namespace fs = std::filesystem;

class Add : public Command {
private:
	void add_problem(const std::string& source) const;

	std::string user_file_input(const fs::path& filename,
	                            const std::string& contents = "") const;
	std::string get_solution_body(const fs::path& base_path) const;
	YAML::Node get_solution_metadata(const fs::path& base_path) const;
	void create_solution_file(const fs::path& path, const std::string& body,
	                          const YAML::Node& metadata) const;

public:
	Add();
	int execute() override;

	static constexpr const std::string cmd_name = "add";
};
