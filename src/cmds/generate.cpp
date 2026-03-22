#include <exception>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include "oly/cmds/generate.hpp"
#include "oly/config.hpp"
#include "oly/contest.hpp"
#include "oly/log.hpp"
#include "oly/utils.hpp"

namespace fs = std::filesystem;

Generate::Generate() {
	add("--preview", "open the generated pdf", [] { opts.preview = true; });
	add("--no-preview", "do not open the generated pdf", [] { opts.preview = false; });
	add("--clean", "remove auxiliary files", false);
	add("--no-pdf", "only generate a source file", false);
	add("--no-source", "remove the source file and induce --clean", false);
	add("--cwd", "create the pdf in the current directory", false);
}

static int run(const std::vector<std::string>& args, bool silent = false) {
	std::vector<char*> c_args;
	for (const auto& s : args)
		c_args.push_back(const_cast<char*>(s.c_str()));
	c_args.push_back(nullptr);

	pid_t pid = fork();

	if (pid == 0) {
		// child
		if (silent) {
			int devnull = open("/dev/null", O_WRONLY);
			if (devnull != -1) {
				dup2(devnull, STDOUT_FILENO);
				dup2(devnull, STDERR_FILENO);
				close(devnull);
			}
		}

		execvp(c_args[0], c_args.data());
		_exit(127); // exec failed
	}

	int status;
	waitpid(pid, &status, 0);

	if (WIFEXITED(status))
		return WEXITSTATUS(status);

	if (WIFSIGNALED(status))
		return 128 + WTERMSIG(status);

	return -1;
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
			out << " [" + shared["source"] + "]";
		out << "\n";
		if (bodies.size() > 0)
			out << bodies[0];
		if (positional_args.size() > 1)
			out << "\\end{problem}";
		else
			out << "\\end{problem*}";
		out << "\n\n";
		if (metadata["url"] and !metadata["url"].IsNull())
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

	if (run({"which", "latexmk"}, true))
		Log::CRITICAL("latexmk is not executable");
	if (opts.preview && run({"which", opts.pdf_viewer}, true))
		Log::CRITICAL(opts.pdf_viewer + " is not executable");

	std::vector<std::string> cmd{
	    "latexmk",
	    "-pdf",
	    "-silent",
	};
	if (opts.preview) {
		cmd.push_back("-pv");
		cmd.push_back("-e");
		cmd.push_back("'$pdf_previewer=q[" + opts.pdf_viewer + " %S];'");
	}
	std::string outdir = get<bool>("--cwd") ? fs::current_path().string()
	                                        : latex_file_path.parent_path().string();
	cmd.push_back("-outdir=" + outdir);
	cmd.push_back(latex_file_path.string());
	run(cmd);

	// cleanup
	if (get<bool>("--clean") || get<bool>("--cwd") || get<bool>("--no-source")) {
		std::error_code ec;
		std::vector<std::string> exts{"aux", "fdb_latexmk", "fls", "log", "pre"};
		if (get<bool>("--no-source"))
			exts.push_back("tex");

		for (const std::string& ext : exts)
			fs::remove(outdir / latex_file_path.filename().replace_extension(ext), ec);
	}
}

void Generate::create_typst_file(const fs::path& typst_file_path) {
	fs::create_directories(typst_file_path.parent_path());
	std::ofstream out(typst_file_path);

	constexpr char LATEX_PREAMBLE[] = {
#embed "../../assets/typst/preamble.typ"
	};
	constexpr size_t LATEX_PREAMBLE_SIZE = sizeof(LATEX_PREAMBLE);
	std::string latex_preamble(LATEX_PREAMBLE, LATEX_PREAMBLE_SIZE);
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
				out << "(\"" << get_problem_name(problem) << "\")";
			out << "[\n";
		}
		if (bodies.size() > 0)
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

	if (run({"which", "typst"}, true))
		Log::CRITICAL("typst is not executable");
	if (opts.preview && run({"which", opts.pdf_viewer}, true))
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
	if (opts.preview) {
		cmd.push_back("--open");
		cmd.push_back(opts.pdf_viewer);
	}
	cmd.push_back(typst_file_path.string());
	if (get<bool>("--cwd")) {
		fs::path pdf = typst_file_path.filename().replace_extension("pdf");
		cmd.push_back((fs::current_path() / pdf).string());
	}
	run(cmd);

	if (get<bool>("--no-source")) {
		fs::remove(typst_file_path);
	}
}

int Generate::execute(std::vector<std::string>& args) {
	load_config_file(args);
	parse(args);

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

	if (opts.lang == configuration::lang::latex) {
		try {
			create_latex_file(output_path / (source + ".tex"));
			create_pdf_from_latex(output_path / (source + ".tex"));
		} catch (const std::exception& e) {
			Log::ERROR(e.what());
		}
	} else {
		try {
			create_typst_file(output_path / (source + ".typ"));
			create_pdf_from_typst(output_path / (source + ".typ"));
		} catch (const std::exception& e) {
			Log::ERROR(e.what());
		}
	}

	return 0;
}
