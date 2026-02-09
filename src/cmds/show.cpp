#include <cctype>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "oly/cmds/show.hpp"
#include "oly/config.hpp"
#include "oly/contest.hpp"
#include "oly/log.hpp"
#include "oly/utils.hpp"

namespace fs = std::filesystem;

Show::Show() {}

constexpr const static std::string hex_to_rgb(const std::string& hex_code) {
	if (hex_code.length() != 7) {
		throw std::invalid_argument(hex_code + "is not a valid hex code !");
	}

	auto digit = [&](const char& c) -> int {
		if (std::isalpha(c)) {
			return std::toupper(c) - 'A' + 10;
		} else if (std::isdigit(c)) {
			return c - '0';
		} else {
			throw std::invalid_argument(hex_code + " is not a valid hex code !");
		}
	};
	int r = 16 * digit(hex_code[1]) + digit(hex_code[2]);
	int g = 16 * digit(hex_code[3]) + digit(hex_code[4]);
	int b = 16 * digit(hex_code[5]) + digit(hex_code[6]);
	return std::format("\x1b[38;2;{};{};{}m", r, g, b);
}

constexpr const static std::string get_color(const std::string& field) {
	static const std::map<std::string, std::string> default_hex{
	    {"math_mode", hex_to_rgb("#7fb4ca")},
	    {"operator", hex_to_rgb("#c0a36e")},
	    {"digit", hex_to_rgb("#d27e99")},
	    {"punctuation", hex_to_rgb("#9cabca")},
	};
	if (config["colorscheme"] && config["colorscheme"][field] &&
	    config["colorscheme"][field].IsScalar()) {
		return hex_to_rgb(config["colorscheme"][field].as<std::string>());
	} else {
		return default_hex.at(field);
	}
}

constexpr const static std::string colorize(const std::string& input) {
	const static std::string COLOR_RESET = "\x1b[0m";
	const static std::string MATH_MODE_OPEN = get_color("math_mode");
	const static std::string OPERATOR_COLOR = get_color("operator");
	const static std::string DIGIT_COLOR = get_color("digit");
	const static std::string PUNCTUATION_COLOR = get_color("punctuation");
	const static std::set<char> operators = {'+', '-', '*', '^', '_', '<', '>', '!', '='};
	const static std::set<char> punctuation = {'.', ';', ':', '(', ')', '[', ']', '{', '}'};
	bool in_math_mode = false;
	std::string formatted = "";
	bool escaped = false;
	for (const char& c : input) {
		if (c == '$' && not escaped) {
			formatted += in_math_mode ? COLOR_RESET : MATH_MODE_OPEN;
			in_math_mode = !in_math_mode;
		} else if (c == '\\' && not escaped) {
			escaped = true;
		} else if (c == '\\' && escaped) {
			formatted += '\\';
		} else if (escaped) {
			formatted += "⏎";
		} else if (operators.contains(c) && in_math_mode) {
			formatted += OPERATOR_COLOR;
			formatted += c;
			formatted += MATH_MODE_OPEN;
		} else if (punctuation.contains(c) && in_math_mode) {
			formatted += PUNCTUATION_COLOR;
			formatted += c;
			formatted += MATH_MODE_OPEN;
		} else if (std::isdigit(c)) {
			formatted += DIGIT_COLOR;
			formatted += c;
			formatted += in_math_mode ? MATH_MODE_OPEN : COLOR_RESET;
		} else {
			formatted += c;
		}
	}
	return formatted;
}

constexpr const static std::string trim_trailing_newlines(std::string& input) {
	while (input.back() == '\n') {
		input = input.substr(0, input.length() - 1);
	}
	return input + '\n';
}

std::string Show::get_statement(const fs::path& pb) const {
	std::ifstream file(pb);
	if (!file.is_open())
		throw std::runtime_error("Could not open " + pb.string() + "!");

	std::string pb_statement;
	std::string line;
	while (getline(file, line)) {
		if (!utils::is_yaml(line) && !utils::should_ignore(line)) {
			if (!utils::is_separator(line)) {
				pb_statement += (line + '\n');
			}
			break;
		}
	}
	while (getline(file, line)) {
		if (utils::is_separator(line)) {
			return colorize(trim_trailing_newlines(pb_statement));
		} else {
			pb_statement += (line + '\n');
		}
	}
	while (!pb_statement.empty() && pb_statement.back() == '\n') {
		pb_statement.pop_back();
	}

	return colorize(trim_trailing_newlines(pb_statement));
}

bool Show::print_statement(const fs::path& source_path) const {
	if (!fs::exists(source_path)) {
		Log::ERROR(source_path.string() + " does not exist !");
		return false;
	} else {
		try {
			std::cout << get_statement(source_path);
		} catch (std::runtime_error& e) {
			Log::ERROR(e.what());
			return false;
		}
		return true;
	}
}

int Show::execute() {
	load_config_file();

	if (positional_args.empty()) {
		for (const std::string& problem : utils::prompt_user_for_problems()) {
			positional_args.push_back(problem);
		}
	}

	bool success = true;
	for (const std::string& pb : positional_args) {
		success = success && print_statement(get_problem_solution_path(pb));
		if (&pb != &positional_args.back()) {
			std::cout << std::endl;
		}
	}
	return success ? 0 : 1;
}
