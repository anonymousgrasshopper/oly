#include <deque>
#include <filesystem>
#include <string>

#include "oly/cmds/add.hpp"
#include "oly/config.hpp"
#include "oly/log.hpp"
#include "oly/utils.hpp"

namespace fs = std::filesystem;

Add::Add() {}

void Add::create_preview_file() {
	fs::path preview_file_path("/tmp/oly/" + config["source"].as<std::string>() + "/" +
	                           "preview.tex");
	const std::string PREVIEW_FILE_CONTENTS = expand_vars(R"(\documentclass[11pt]{scrartcl}
\usepackage[sexy,diagrams]{evan}
\author{${author}}
\title{${source}}
\begin{document}
\input{/tmp/oly/${source}/solution.tex}
\end{document}
)");
	create_file(preview_file_path, PREVIEW_FILE_CONTENTS);
}

std::deque<std::string> Add::get_solution_body() {
	create_preview_file();
	std::deque<std::string> input =
	    input_file("/tmp/oly/" + config["source"].as<std::string>() + "/solution.tex",
	               expand_vars(config["preamble"].as<std::string>()))
	        .lines();
	while (input.front().starts_with("%") || input.front().empty()) {
		input.pop_front();
	}
	return input;
}

YAML::Node Add::get_solution_metadata() {
	input_file file("/tmp/oly/" + config["source"].as<std::string>() + "/metadata.yaml",
	                expand_vars(config["metadata"].as<std::string>()));

	Log::Log(severity::INFO, "attempting to load yaml !");
	auto metadata = load_yaml(file.filepath);
	while (!metadata) {
		file.edit();
		metadata = load_yaml(file.filepath);
	}
	return metadata.value();
}

void Add::create_solution_file(const std::deque<std::string>& body,
                               const YAML::Node& metadata) {
	const fs::path path(expand_env_vars(config["base_path"].as<std::string>()) +
	                    config["source"].as<std::string>() + ".tex");
	std::string contents;
	for (auto field : metadata) {
		contents.append(field.first.as<std::string>() + ": " +
		                field.second.as<std::string>() + '\n');
	}
	contents.append("\n");
	for (std::string line : body) {
		contents.append(line + '\n');
	}
	create_file(path, contents);
}

void Add::add_problem(std::string source) {
	config["source"] = source;
	std::deque<std::string> body = get_solution_body();
	Log::Log(severity::INFO, "got solution body !");
	YAML::Node metadata = get_solution_metadata();
	Log::Log(severity::INFO, "got solution metadata !");

	create_solution_file(body, metadata);
}

int Add::execute() {
	for (std::string source : positional_args) {
		add_problem(get_problem_id(source));
	}
	return 0;
}
