#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>

#include "oly/cmds/generate.hpp"
#include "oly/config.hpp"
#include "oly/contest.hpp"
#include "oly/log.hpp"
#include "oly/utils.hpp"

namespace fs = std::filesystem;

Generate::Generate() {
	add("--preview", "open the generated pdf", [] { config["preview"] = true; });
	add("--no-preview", "do not open the generated pdf", [] { config["preview"] = false; });
	add("--clean", "remove the auxiliary files", false);
	add("--no-pdf", "only generate a tex file", false);
	add("--no-source", "remove the source file and induce --clean", false);
	add("--cwd", "create the files in the current directory",
	    [] { config["output_directory"] = std::string(getenv("PWD")); });
}

std::vector<std::string> Generate::get_solution_bodies(const fs::path& source) {
	std::ifstream file(source);
	if (!file.is_open())
		throw std::runtime_error("Could not open " + source.string());

	std::vector<std::string> bodies;
	std::string body;
	std::string line;
	while (getline(file, line)) {
		if (!utils::is_yaml(line)) {
			if (utils::is_separator(line)) {
				bodies.push_back(body);
				body = "";
			} else {
				body += (line + '\n');
			}
			break;
		}
	}
	while (getline(file, line)) {
		if (utils::is_separator(line)) {
			bodies.push_back(body);
			body = "";
		} else {
			body += (line + '\n');
		}
	}
	bodies.push_back(body);

	return bodies;
}

YAML::Node Generate::get_solution_metadata(const fs::path& source) {
	std::ifstream file(source);
	if (!file.is_open())
		throw std::runtime_error("Could not open " + source.string());

	std::string yaml;
	std::string line;
	while (getline(file, line)) {
		if (utils::is_yaml(line))
			yaml += (line + '\n');
		else
			break;
	}
	std::optional<YAML::Node> data = utils::load_yaml(yaml);
	if (!data) {
		Log::ERROR("Could not get metadata from " + source.string());
		return YAML::Node();
	}
	return data.value();
}

void Generate::create_latex_file(const fs::path& latex_file_path) {
	fs::create_directories(latex_file_path.parent_path());
	std::ofstream out(latex_file_path);

	constexpr char LATEX_PREAMBLE[] = {
#embed "../../assets/preamble.tex"
	};
	constexpr size_t LATEX_PREAMBLE_SIZE = sizeof(LATEX_PREAMBLE);
	std::string latex_preamble(LATEX_PREAMBLE, LATEX_PREAMBLE_SIZE);
	out << utils::expand_vars(latex_preamble);

	for (std::string problem : positional_args) {
		std::vector<std::string> bodies = get_solution_bodies(problem);
		YAML::Node metadata = get_solution_metadata(problem);

		if (positional_args.size() > 1)
			out << "\\begin{problem}";
		else
			out << "\\begin{problem*}";
		if (metadata["source"])
			out << " [" + config["source"].as<std::string>() + "]";
		out << "\n";
		if (bodies.size() > 0)
			out << bodies[0];
		if (positional_args.size() > 1)
			out << "\\end{problem}";
		else
			out << "\\end{problem*}";
		out << "\n\n";
		if (metadata["url"])
			out << "\\noindent\\emph{Link}: \\url{" << metadata["url"].as<std::string>() << "}"
			    << "\n\n";
		for (size_t i = 1; i < bodies.size(); ++i)
			out << "\\hrulebar" << "\n\n" << bodies[i];
		out << "\n" << "\\pagebreak" << "\n\n";
	}

	out << "\\end{document}" << '\n';

	out.close();
}

void Generate::create_pdf_from_latex(fs::path latex_file_path) {
	if (get<bool>("--no-pdf"))
		return;

	for (auto program : {std::string("latexmk"), config["pdf_viewer"].as<std::string>()})
		if (std::system(("which " + program + " >/dev/null 2>&1").c_str()))
			Log::CRITICAL(program + " is not executable");

	if (latex_file_path.string().contains('"'))
		throw std::invalid_argument("double quotes not allowed in file paths !");

	std::string preview_cmd = config["preview"].as<bool>()
	                              ? "-pv -e '$pdf_previewer=q[" +
	                                    config["pdf_viewer"].as<std::string>() + " %S];' "
	                              : "";
	std::string cmd = "latexmk -pdf -outdir=\"" + latex_file_path.parent_path().string() +
	                  '\"' + " -quiet " + preview_cmd + "\"" + latex_file_path.string() +
	                  '"';
	std::system(cmd.c_str());

	// cleanup
	if (get<bool>("--clean") || get<bool>("--no-tex")) {
		fs::remove(latex_file_path.replace_extension(".out"));
		fs::remove(latex_file_path.replace_extension(".log"));
	}
}

void Generate::create_typst_file(const fs::path& typst_file_path) {}
void Generate::create_pdf_from_typst(const fs::path& typst_file_path) {}

int Generate::execute() {
	load_config_file();

	if (positional_args.empty())
		return 0;

	std::string source;
	for (std::string problem : positional_args) {
		fs::path source = get_problem_relative_path(problem);
		config["source"] = source.stem().string();

		fs::path output_path(
		    utils::expand_vars(config["output_directory"].as<std::string>()));
		if (config["language"].as<std::string>() == "latex") {
			try {
				create_latex_file(output_path / source);
				create_pdf_from_latex(output_path / source);
			} catch (const std::runtime_error& e) {
				Log::ERROR(e.what());
			}
		} else {
			try {
				create_typst_file(output_path / source);
				create_pdf_from_typst(output_path / source);
			} catch (const std::runtime_error& e) {
				Log::ERROR(e.what());
			}
		}
	}

	return 0;
}
