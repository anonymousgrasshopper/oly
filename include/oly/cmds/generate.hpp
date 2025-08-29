#pragma once
#include <filesystem>

#include "oly/cmds/command.hpp"
#include "yaml-cpp/node/node.h"

class Generate : public Command {
private:
	std::vector<std::string> get_solution_bodies(std::string source);
	YAML::Node get_solution_metadata(std::string source);

	void create_latex_file(std::filesystem::path latex_file_path);
	void create_pdf(std::filesystem::path latex_file_path);

public:
	Generate();
	int execute() override;

	static inline const std::string cmd_name = "gen";
};
