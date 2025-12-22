#include <cctype>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "oly/cmds/show.hpp"
#include "oly/contest.hpp"
#include "oly/log.hpp"
#include "oly/utils.hpp"

namespace fs = std::filesystem;

Show::Show() {}

constexpr const static std::string get_color(const std::string& hex_code) {
	if (hex_code.length() != 7) {
		throw std::invalid_argument(hex_code + "is not a valid hex code !");
	}

	auto digit = [&](const char& c) -> int {
		if (std::isalpha(c)) {
			return std::toupper(c) - 'A' + 10;
		} else if (std::isdigit(c)) {
			return c - '0';
		} else {
			throw std::invalid_argument(hex_code + "is not a valid hex code !");
		}
	};
	int r = 16 * digit(hex_code[1]) + digit(hex_code[2]);
	int g = 16 * digit(hex_code[3]) + digit(hex_code[2]);
	int b = 16 * digit(hex_code[5]) + digit(hex_code[6]);
	return std::format("\x1b[38;2;{};{};{}m", r, g, b);
}

constexpr const static std::string colorize(const std::string& input) {
	// replace $ by escape codes
	const static std::string MATH_MODE_OPEN = get_color("#7fb4ca");
	const static std::string COLOR_RESET = "\x1b[0m";
	const static std::set<char> operators = {'+', '-', '*', '<', '>', '!', '='};
	const static std::string OPERATOR_COLOR = get_color("#c0a36e");
	bool in_math_mode = false;
	std::string formatted = "";
	for (const char& c : input) {
		if (c == '$') {
			formatted += in_math_mode ? COLOR_RESET : MATH_MODE_OPEN;
			in_math_mode = !in_math_mode;
		} else if (operators.contains(c)) {
			formatted += OPERATOR_COLOR;
			formatted += c;
			formatted += MATH_MODE_OPEN;
		} else {
			formatted += c;
		}
	}
	return formatted;
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
			return colorize(pb_statement);
		} else {
			pb_statement += (line + '\n');
		}
	}
	while (!pb_statement.empty() && pb_statement.back() == '\n') {
		pb_statement.pop_back();
	}

	return colorize(pb_statement);
}

void Show::print_statement(const std::string& pb) const {
	fs::path path(get_problem_path(pb));
	if (!fs::exists(path)) {
		Log::ERROR(path.string() + " does not exist !");
	} else {
		std::cout << get_statement(path);
	}
}

int Show::execute() {
	load_config_file();

	if (positional_args.empty()) {
		for (const std::string& problem : utils::prompt_user_for_problems()) {
			positional_args.push_back(problem);
		}
	}

	for (const std::string& pb : positional_args) {
		print_statement(pb);
		if (&pb != &positional_args.back()) {
			std::cout << std::endl;
		}
	}
	return 0;
}
