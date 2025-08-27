#include <deque>
#include <filesystem>
#include <fstream>
#include <regex>
#include <string>
#include <vector>

#include "oly/cmds/add.hpp"
#include "oly/config.hpp"
#include "oly/log.hpp"
#include "oly/utils.hpp"

namespace fs = std::filesystem;

Add::Add() {}

struct input_file {
private:
	void create_file() {
		fs::create_directories(filepath.parent_path());
		std::ofstream out(filepath);
		out << contents;
		out.close();
	}
	bool remove;

public:
	fs::path filepath;
	std::string contents;

	input_file(fs::path filepath, std::string contents, bool remove = true)
	    : remove(remove), filepath(filepath), contents(contents) {
		create_file();
		edit();
	};
	~input_file() {
		if (remove && fs::exists(filepath)) {
			fs::remove(filepath);
		}
	}

	std::deque<std::string> lines() {
		std::ifstream file(filepath);
		std::deque<std::string> lines;
		std::string line;
		if (!file.is_open())
			Log::Log(severity::CRITICAL, "unable to open " + filepath.string() + "!");
		while (getline(file, line)) {
			lines.push_back(line);
		}
		return lines;
	}

	void edit() {
		std::system((config["editor"].as<std::string>() + " \"" + filepath.string() + "\"")
		                .c_str()); // pray for the filetype not to contain quotes
	}
};

void Add::create_preview_file() {
	fs::path preview_file_path("/tmp/oly/" + config["pb_name"].as<std::string>() + "/" +
	                           "preview.tex");
	const std::string PREVIEW_FILE_CONTENTS = R"(\documentclass[11pt]{scrartcl}
\usepackage[sexy,diagrams]{evan}
\author{)" + config["author"].as<std::string>() +
	                                          R"(}
\title{VON Preview}
\begin{document}
\input{)" + preview_file_path.string() +
	                                          R"(/}
\end{document}
)";
	create_file(preview_file_path, PREVIEW_FILE_CONTENTS);
}

std::deque<std::string> Add::get_solution_body() {
	create_preview_file();
	std::deque<std::string> input =
	    input_file("/tmp/oly/" + config["pb_name"].as<std::string>() + "/solution.tex",
	               config["preamble"].as<std::string>())
	        .lines();
	while (input.front().starts_with("%")) {
		input.pop_front();
	}
	return input;
}

YAML::Node Add::get_solution_metadata() {
	auto expand_vars = [&](std::string str) -> std::string {
		std::string fmt = str;

		static std::regex var("\\$\\{([^}]+)\\}");
		std::smatch match;
		while (std::regex_search(fmt, match, var)) {
			if (!config[match[1].str()]) {
				Log::Log(severity::WARNING,
				         "In metadata config: ${" + match[1].str() + "} is not a valid value !");
				fmt.replace(match[0].first, match[0].second, "");
			} else {
				const std::string var(config[match[1].str()].as<std::string>());
				fmt.replace(match[0].first, match[0].second, var);
			}
		}
		return fmt;
	};

	input_file file("/tmp/oly/" + config["pb_name"].as<std::string>() + "/metadata.yaml",
	                expand_vars(config["metadata"].as<std::string>()));
	auto load_yaml = [&]() -> std::optional<YAML::Node> {
		try {
			YAML::Node metadata = YAML::LoadFile(file.filepath);
			return metadata;
		} catch (const YAML::ParserException& e) {
			Log::Log(severity::ERROR,
			         "YAML syntax error in config file: " + static_cast<std::string>(e.what()),
			         logopt::WAIT);
			return std::nullopt;
		} catch (const YAML::BadFile& e) {
			Log::Log(severity::ERROR, "Could not open config file: " + file.filepath.string(),
			         logopt::WAIT);
			return std::nullopt;
		}
	};
	Log::Log(severity::INFO, "attempting to load yaml !");
	auto metadata = load_yaml();
	while (!metadata) {
		file.edit();
		metadata = load_yaml();
	}
	return metadata.value();
}

void Add::create_solution_file(const std::deque<std::string>& body,
                               const YAML::Node& metadata) {
	const fs::path path(expand_env_vars(config["base_path"].as<std::string>()) +
	                    config["pb_name"].as<std::string>() + ".tex");
	std::string contents;
	const std::vector<std::string> fields = {"name", "author", "difficulty", "tags"};
	for (const std::string& field : fields) {
		if (metadata[field]) {
			contents.append(field + ": " + metadata[field].as<std::string>() + '\n');
		}
	}
	contents.append("\n\n\n");
	for (std::string line : body) {
		contents.append(line);
	}
	create_file(path, contents);
}

void Add::add_problem(std::string pb_name) {
	config["pb_name"] = pb_name;
	std::deque<std::string> body = get_solution_body();
	Log::Log(severity::INFO, "got solution body !");
	YAML::Node metadata = get_solution_metadata();
	Log::Log(severity::INFO, "got solution metadata !");

	create_solution_file(body, metadata);
}

int Add::execute() {
	for (std::string pb_name : positional_args) {
		add_problem(pb_name);
	}
	return 0;
}
