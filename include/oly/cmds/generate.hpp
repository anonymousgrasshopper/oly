#pragma once

#include <filesystem>

#include "oly/cmds/command.hpp"
#include "yaml-cpp/yaml.h"

namespace fs = std::filesystem;

class Generate : public Command {
private:
	std::vector<std::string> get_solution_bodies(const fs::path& source);
	YAML::Node get_solution_metadata(const fs::path& source);

	void create_latex_file(const fs::path& latex_file_path);
	void create_pdf_from_latex(fs::path latex_file_path);

	void create_typst_file(const fs::path& typst_file_path);
	void create_pdf_from_typst(const fs::path& typst_file_path);

public:
	Generate();
	int execute() override;

	static constexpr const std::string cmd_name = "gen";
};
