#pragma once

#include "oly/cmds/command.hpp"
#include <filesystem>

class Generate : public Command {
private:
	void generate(std::string source);

	void create_latex_file(std::filesystem::path latex_file_path);
	void create_pdf(std::filesystem::path latex_file_path);
	void open_pdf_viewer(std::filesystem::path latex_file_path);

public:
	Generate();
	int execute() override;

	static inline const std::string cmd_name = "gen";
};
