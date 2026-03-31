#include <exception>
#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "oly/cmds/generate.hpp"
#include "oly/config.hpp"
#include "oly/contest.hpp"
#include "oly/log.hpp"
#include "oly/utils.hpp"

namespace fs = std::filesystem;

Generate::Generate() {
	add("--open", "Open the generated pdf", [] { opts.open = true; });
	add("--no-open", "Do not open the generated pdf", [] { opts.open = false; });
	add("--clean", "Remove auxiliary files", false);
	add("--no-pdf", "Only generate a source file", false);
	add("--no-source", "Remove the source file and induce --clean", false);
	add("--cwd", "Create the pdf in the current directory", false);
}

std::vector<std::string> Generate::get_solution_bodies(const fs::path& source) {
	std::ifstream file(source);
	if (!file.is_open())
		throw std::runtime_error("Could not open " + source.string() + "!");

	std::vector<std::string> bodies;
	std::string body;
	std::string line;
	while (getline(file, line)) {
		if (!utils::is_yaml(line) && !utils::should_ignore(line)) {
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
	std::optional<YAML::Node> data = utils::yaml::load(yaml);
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
#embed "../../assets/tex/preamble.tex"
	};
	constexpr size_t LATEX_PREAMBLE_SIZE = sizeof(LATEX_PREAMBLE);
	std::string latex_preamble(LATEX_PREAMBLE, LATEX_PREAMBLE_SIZE);
	out << utils::expand_vars(latex_preamble);

	for (const std::string& problem : positional_args) {
		const fs::path pb_path = get_problem_solution_path(problem);
		std::vector<std::string> bodies = get_solution_bodies(pb_path);
		YAML::Node metadata = get_solution_metadata(pb_path);

		if (positional_args.size() > 1)
			out << "\\begin{problem}";
		else
			out << "\\begin{problem*}";
		if (metadata["source"])
			out << " [" + metadata["source"].as<std::string>() + "]";
		out << "\n";
		if (!bodies.empty())
			out << bodies[0];
		if (positional_args.size() > 1)
			out << "\\end{problem}";
		else
			out << "\\end{problem*}";
		out << "\n\n";
		if (metadata["url"] and !metadata["url"].IsNull())
			out << R"(\noindent\emph{Link}: \url{)" << metadata["url"].as<std::string>() << "}"
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

	if (utils::run({"which", "latexmk"}, true))
		Log::CRITICAL("latexmk is not executable");
	if (opts.open && utils::run({"which", opts.pdf_viewer}, true))
		Log::CRITICAL(opts.pdf_viewer + " is not executable");

	std::vector<std::string> cmd{
	    "latexmk",
	    "-pdf",
	    "-silent",
	};
	if (opts.open) {
		cmd.emplace_back("-pv");
		cmd.emplace_back("-e");
		cmd.emplace_back("'$pdf_previewer=q[" + opts.pdf_viewer + " %S];'");
	}
	std::string outdir = get<bool>("--cwd") ? fs::current_path().string()
	                                        : latex_file_path.parent_path().string();
	cmd.emplace_back("-outdir=" + outdir);
	cmd.emplace_back(latex_file_path.string());
	utils::run(cmd);

	// cleanup
	if (get<bool>("--clean") || get<bool>("--cwd") || get<bool>("--no-source")) {
		std::error_code ec;
		std::vector<std::string> exts{"aux", "fdb_latexmk", "fls", "log", "pre"};
		if (get<bool>("--no-source"))
			exts.emplace_back("tex");

		for (const std::string& ext : exts)
			fs::remove(outdir / latex_file_path.filename().replace_extension(ext), ec);
	}
}

void Generate::create_typst_file(const fs::path& typst_file_path) {
	fs::create_directories(typst_file_path.parent_path());
	std::ofstream out(typst_file_path);

	constexpr char TYPST_PREAMBLE[] = {
#embed "../../assets/typst/preamble.typ"
	};
	constexpr size_t LATEX_PREAMBLE_SIZE = sizeof(TYPST_PREAMBLE);
	std::string latex_preamble(TYPST_PREAMBLE, LATEX_PREAMBLE_SIZE);
	if (positional_args.size() != 1) {
		out << utils::expand_vars(latex_preamble);
	}

	for (const std::string& problem : positional_args) {
		const fs::path pb_path = get_problem_solution_path(problem);
		std::vector<std::string> bodies = get_solution_bodies(pb_path);
		YAML::Node metadata = get_solution_metadata(pb_path);
		if (positional_args.size() == 1) {
			utils::yaml::merge_metadata(metadata);
			out << utils::expand_vars(latex_preamble);
		}

		if (!metadata["title"]) {
			if (positional_args.size() > 1)
				out << "#problem";
			else
				out << "#_problem";
			if (metadata["source"])
				out << "(\"" << metadata["source"] << "\")";
			out << "[\n";
		}
		if (!bodies.empty())
			out << bodies[0];
		if (!metadata["title"]) {
			out << "]";
		}
		out << "\n\n";
		if (metadata["url"] and !metadata["url"].IsNull())
			out << "#link(\"" << metadata["url"].as<std::string>() << "\")[_"
			    << metadata["url"].as<std::string>() << " _]"
			    << "\n\n";
		for (size_t i = 1; i < bodies.size(); ++i) {
			if (i == 1) {
				out << bodies[i];
			} else {
				out << "#hrule" << "\n\n" << bodies[i];
			}
		}

		if (&problem != &positional_args.back()) {
			out << "\n" << "#pagebreak()" << "\n\n";
		}
	}

	out.close();
}

void Generate::create_pdf_from_typst(const fs::path& typst_file_path) {
	if (get<bool>("--no-pdf"))
		return;

	if (utils::run({"which", "typst"}, true))
		Log::CRITICAL("typst is not executable");
	if (opts.open && utils::run({"which", opts.pdf_viewer}, true))
		Log::CRITICAL(opts.pdf_viewer + " is not executable");

	// BUG: unhandled conflicts (figures with the same name)
	for (std::string problem : positional_args) {
		utils::figures::copy(typst_file_path.parent_path(), get_problem_path(problem));
	}

	std::vector<std::string> cmd{
	    "typst",
	    "compile",
	    "--root",
	    typst_file_path.parent_path().string(),
	};
	if (opts.open) {
		cmd.emplace_back("--open");
		cmd.emplace_back(opts.pdf_viewer);
	}
	cmd.emplace_back(typst_file_path.string());
	if (get<bool>("--cwd")) {
		fs::path pdf = typst_file_path.filename().replace_extension("pdf");
		cmd.emplace_back((fs::current_path() / pdf).string());
	}
	utils::run(cmd);

	if (get<bool>("--no-source")) {
		fs::remove(typst_file_path);
	}
}

int Generate::execute() {
	if (positional_args.empty()) {
		for (const std::string& problem : utils::prompt_user_for_problems()) {
			positional_args.push_back(problem);
		}
	}

	std::string source;
	for (std::string problem : positional_args) {
		source += get_problem_name(problem) + " - ";
	}
	source = source.substr(0, source.length() - 3);
	shared["source"] = source;

	fs::path output_path(fs::path(utils::expand_vars(opts.output_directory)) / source);

	try {
		if (opts.lang == configuration::lang::latex) {
			create_latex_file(output_path / (source + ".tex"));
			create_pdf_from_latex(output_path / (source + ".tex"));
		} else {
			create_typst_file(output_path / (source + ".typ"));
			create_pdf_from_typst(output_path / (source + ".typ"));
		}
	} catch (const std::exception& e) {
		Log::ERROR(e.what());
	}

	return 0;
}
