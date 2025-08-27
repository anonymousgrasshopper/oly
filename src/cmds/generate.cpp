#include <filesystem>
#include <fstream>
#include <optional>

#include "oly/cmds/generate.hpp"
#include "oly/config.hpp"
#include "oly/log.hpp"
#include "oly/utils.hpp"

namespace fs = std::filesystem;

Generate::Generate() {}

std::vector<std::string> Generate::get_solution_bodies(std::string source) {
	std::ifstream file(get_problem_path(source));
	if (!file.is_open()) {
		Log::Log(severity::ERROR, "Could not open " + get_problem_path(source).string());
		return std::vector<std::string>();
	}

	std::vector<std::string> bodies;
	std::string body;
	std::string line;
	while (getline(file, line)) {
		if (!is_yaml(line)) {
			if (is_separator(line)) {
				bodies.push_back(body);
				body = "";
			} else {
				body += (line + '\n');
			}
			break;
		}
	}
	while (getline(file, line)) {
		if (is_separator(line)) {
			bodies.push_back(body);
			body = "";
		} else {
			body += (line + '\n');
		}
	}
	bodies.push_back(body);

	return bodies;
}

YAML::Node Generate::get_solution_metadata(std::string source) {
	std::ifstream file(get_problem_path(source));
	if (!file.is_open()) {
		Log::Log(severity::ERROR, "Could not open " + get_problem_path(source).string());
		return YAML::Node();
	}

	std::string yaml;
	std::string line;
	while (getline(file, line)) {
		if (is_yaml(line))
			yaml += (line + '\n');
		else
			break;
	}
	std::optional<YAML::Node> data = load_yaml(yaml);
	if (!data) {
		Log::Log(severity::ERROR, "Could not get metadata from " + source);
		return YAML::Node();
	}
	return data.value();
}

void Generate::create_latex_file(std::filesystem::path latex_file_path) {
	fs::create_directories(latex_file_path.parent_path());
	std::ofstream out(latex_file_path);

	constexpr char LATEX_PREAMBLE[] = {
#embed "../../assets/latex_preamble.tex"
	};
	constexpr size_t LATEX_PREAMBLE_SIZE = sizeof(LATEX_PREAMBLE);
	std::string latex_preamble(LATEX_PREAMBLE, LATEX_PREAMBLE_SIZE);
	out << expand_vars(latex_preamble);

	for (std::string problem : positional_args) {
		std::vector<std::string> bodies = get_solution_bodies(problem);
		YAML::Node metadata = get_solution_metadata(problem);

		if (positional_args.size() > 1)
			out << "\\begin{problem}";
		else
			out << "\\begin{problem*}";
		if (metadata["source"])
			out << "[" + config["source"].as<std::string>() + "]";
		out << "\n";
		if (bodies.size() > 0)
			out << bodies[0];
		if (positional_args.size() > 1)
			out << "\\end{problem}";
		else
			out << "\\end{problem*}";
		if (metadata["url"])
			out << "\\noindent\\emph{Link}: \\url{" << metadata["url"].as<std::string>() << "}"
			    << "\n";
		for (size_t i = 1; i < bodies.size(); ++i)
			out << "\n" << "\\hrulebar" << "\n\n" << bodies[i];
		out << "\\pagebreak" << "\n\n";
	}

	out << "\\end{document}";

	out.close();
}

void Generate::create_pdf(std::filesystem::path latex_file_path) {
	for (auto program : {std::string("latexmk"), config["pdf_viewer"].as<std::string>()})
		if (std::system(("which " + program + " >/dev/null 2>&1").c_str()))
			Log::Log(severity::CRITICAL, program + " is not executable");

	std::string cmd = "latexmk -pdf -outdir=\"" + latex_file_path.parent_path().string() +
	                  '\"' + " -quiet -pv -e '$pdf_previewer=q[zathura %S];' \"" +
	                  latex_file_path.replace_extension(".tex").string() + '"';
	Log::Log(severity::DEBUG, cmd);
	std::system(cmd.c_str()); // pray for the filename not to contain quotes
}

int Generate::execute() {
	if (positional_args.empty())
		return 0;

	std::string source;
	for (std::string problem : positional_args) {
		source += get_problem_id(problem) + " - ";
	}
	source = source.substr(0, source.length() - 3);
	config["source"] = source;

	fs::path output_path(expand_vars(config["output_directory"].as<std::string>()));
	create_latex_file(output_path / (source + ".tex"));
	create_pdf(output_path / (source + ".pdf"));

	return 0;
}
