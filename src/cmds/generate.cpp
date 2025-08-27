#include <filesystem>

#include "oly/cmds/generate.hpp"
#include "oly/config.hpp"
#include "oly/utils.hpp"

namespace fs = std::filesystem;

Generate::Generate() {}

void create_latex_file(std::filesystem::path latex_file_path) {}
void create_pdf(std::filesystem::path latex_file_path) {}
void open_pdf_viewer(std::filesystem::path latex_file_path) {}

void Generate::generate(std::string source) {
	config["source"] = source;
	fs::path latex_file_path(expand_vars(config["output_directory"].as<std::string>()));
	create_latex_file(latex_file_path);
	create_pdf(latex_file_path);
	open_pdf_viewer(latex_file_path);
}

int Generate::execute() {
	for (std::string source : positional_args) {
		generate(source);
	}
	return 0;
}
