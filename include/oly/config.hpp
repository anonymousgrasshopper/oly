#pragma once

#include <filesystem>
#include <string>

#include "yaml-cpp/yaml.h"

namespace fs = std::filesystem;

namespace configuration {
void load_config(std::string config_file_path);

enum class lang { latex, typst };
} // namespace configuration

struct Options {
	// the author's name in the preview and generated pdf
	std::string author;

	// the base path where solutions are stored
	fs::path base_path;

	// the path where figures are stored, inside of base_path
	fs::path figures_dir{"figures"};

	// the markup language the solutions are typed in
	// must be one of latex or typst (default latex)
	configuration::lang lang = configuration::lang::latex;

	// language setting passed to the typst preview and preamble files.
	// must be a two or three letters language code
	std::string language = "en";

	// the text editor to use
	// autodected from $EDITOR and $VISUAL, fallback to vim
	std::string editor;

	// the pdf viewer used for 'oly gen'
	std::string pdf_viewer;

	// wether to open the generated pdf with pdf_viewer when running 'oly gen'
	bool preview = true;

	// wether to confirm deletion or not in 'oly rm'
	bool confirm = false;

	// map contest names to other ones
	std::map<std::string, std::string> abbreviations{{"Shortlist", "ISL"}};

	// directory structure to store contests
	// available variables are: date, contest, year, problem and source
	std::map<std::string, std::string> contest_format{};

	// customize the colors used by the show command
	std::map<std::string, std::string> colorscheme{
	    {"math_mode", "#7fb4ca"},
	    {"operator", "#c0a36e"},
	    {"digit", "#d27e99"},
	    {"punctuation", "#9cabca"},
	};

	// where to output the generated pdf
	// defaults to ${XDG_CACHE_HOME:-~/.cache}/oly/${source}
	fs::path output_directory;

	// where temporary files are written
	// defaults to ${TMPDIR:-/tmp}/oly
	fs::path tmpdir;

	// added right after \documentclass in the preview file in latex
	// meant to specify which latex packages to use
	std::string packages;

	// preamble is what gets put in the preview file
	// contents is the content that is already written in the file
	// metadata is the metadata you get prompted for in 'oly add'
	std::string preamble;
	std::string contents;
	std::string metadata;

	void update(const YAML::Node& node) {
		if (node["author"])
			author = node["author"].as<std::string>();
		if (node["base_path"])
			base_path = fs::path(node["base_path"].as<std::string>());
		if (node["figures_dir"])
			figures_dir = fs::path(node["figures_dir"].as<std::string>());
		if (node["lang"]) {
			auto s = node["lang"].as<std::string>();
			if (s == "latex")
				lang = configuration::lang::latex;
			else if (s == "typst")
				lang = configuration::lang::typst;
			else
				throw std::runtime_error("Invalid value for 'lang': " + s);
		}
		if (node["language"])
			language = node["language"].as<std::string>();
		if (node["editor"])
			editor = node["editor"].as<std::string>();
		if (node["pdf_viewer"])
			pdf_viewer = node["pdf_viewer"].as<std::string>();
		if (node["preview"])
			preview = node["preview"].as<bool>();
		if (node["confirm"])
			confirm = node["confirm"].as<bool>();
		if (node["abbreviations"])
			abbreviations = node["abbreviations"].as<std::map<std::string, std::string>>();
		if (node["contest_format"])
			contest_format = node["contest_format"].as<std::map<std::string, std::string>>();
		if (node["colorscheme"])
			colorscheme = node["colorscheme"].as<std::map<std::string, std::string>>();
		if (node["output_directory"])
			output_directory = fs::path(node["output_directory"].as<std::string>());
		if (node["tmpdir"])
			tmpdir = fs::path(node["tmpdir"].as<std::string>());
		if (node["packages"])
			packages = node["packages"].as<std::string>();
		if (node["preamble"])
			preamble = node["preamble"].as<std::string>();
		if (node["contents"])
			contents = node["contents"].as<std::string>();
		if (node["metadata"])
			metadata = node["metadata"].as<std::string>();
	}
};

inline Options opts;
inline std::map<std::string, std::string> shared;
inline YAML::Node metadata;
