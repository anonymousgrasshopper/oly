#pragma once

#include <deque>
#include <filesystem>

#include "oly/cmds/command.hpp"
#include "yaml-cpp/node/node.h"

class Add : public Command {
private:
	void add_problem(std::string pb);

	std::deque<std::string> user_file_input(const std::filesystem::path& filename,
	                                        const std::string& contents = "");
	void create_preview_file();
	std::deque<std::string> get_solution_body();
	YAML::Node get_solution_metadata();
	void create_solution_file(const std::deque<std::string>& body,
	                          const YAML::Node& metadata);

public:
	Add();
	int execute() override;

	static constexpr const std::string cmd_name = "add";
};
