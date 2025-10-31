#include <filesystem>
#include <regex>
#include <string>

#include "oly/config.hpp"
#include "oly/contest.hpp"
#include "oly/utils.hpp"

namespace fs = std::filesystem;

static std::string get_topic(const char& letter) {
	switch (letter) {
	case 'A':
		return "Algebra";
		break;
	case 'C':
		return "Combinatorics";
		break;
	case 'G':
		return "Geometry";
		break;
	case 'N':
		return "Number Theory";
		break;
	default:
		return "";
	}
}

static std::string source_get_contest(const std::string& source) {
	// Contest name: a string of letters and spaces beginning and ending with letters
	std::string contest;
	std::smatch match;

	std::regex contest_regex(R"(\b([A-Za-z](?:[A-Za-z ]*)[A-Za-z])\b)");
	if (std::regex_search(source, match, contest_regex)) {
		contest = match.str(1);
		if (config["abbreviations"][contest]) {
			contest = config["abbreviations"][contest].as<std::string>();
		} else {
			if (!config["contest_format"][contest]) {
				if (contest.length() <= 4 && contest.find(' ') == std::string::npos) {
					std::transform(contest.begin(), contest.end(), contest.begin(), ::toupper);
				} else {
					for (size_t i = 0; i < contest.length(); ++i) {
						if (i == 0 || contest[i - 1] == ' ') {
							contest[i] = std::toupper(contest[i]);
						} else {
							contest[i] = std::tolower(contest[i]);
						}
					}
				}
			}
		}
	}

	return contest;
}

static std::string source_get_year(const std::string& source) {
	// Year: 4 digits or 2 digits not part of a longer digit sequence
	std::string year;
	std::smatch match;

	std::regex year_regex(R"((\b\d{2}\b|\b\d{4}\b))");
	if (std::regex_search(source, match, year_regex)) {
		if (match.str(1).size() == 4) {
			year = match.str(1);
		} else if (match.str(1).size() == 2) {
			year = "20" + match.str(1);
		}
	}

	return year;
}

static std::string source_get_problem(const std::string& source) {
	// Problem: single digit, preceded by non-digit or letter, not followed by digit
	std::string problem;
	std::smatch match;

	std::regex problem_regex(R"((\b[A-Za-z]?\d\b))");
	if (std::regex_search(source, match, problem_regex)) {
		if (match.str(1).size() == 1) {
			problem = "P" + match.str(1);
		} else if (match.str(1).size() == 2) {
			problem = match.str(1);
			problem[0] = std::toupper(problem[0]);
			config["topic"] = get_topic(problem[0]);
		}
	}

	return problem;
}

static std::string source_get_date(const std::string& source) {
	// Date: between 2 and 3 sequences of at most two digits with slashes separating them
	std::smatch match;
	std::regex date_regex(R"((\d{1,2})/(\d{1,2})(?:/(\d{2,4}))?)");

	int day, month, year;

	if (std::regex_search(source, match, date_regex)) {
		day = std::stoi(match[1].str());
		month = std::stoi(match[2].str());

		// Determine year
		if (match[3].matched) {
			year = std::stoi(match[3].str());
			if (year < 100) // 2-digit year → assume 20xx
				year += 2000;
		} else {
			// No year given → use current year
			auto now = std::chrono::system_clock::now();
			std::time_t t = std::chrono::system_clock::to_time_t(now);
			std::tm tm = *std::localtime(&t);
			year = tm.tm_year + 1900;
		}
	} else {
		// No date found → use current day, month, year
		auto now = std::chrono::system_clock::now();
		std::time_t t = std::chrono::system_clock::to_time_t(now);
		std::tm tm = *std::localtime(&t);
		day = tm.tm_mday;
		month = tm.tm_mon + 1;
		year = tm.tm_year + 1900;
	}

	std::ostringstream oss;
	oss << std::setfill('0') << std::setw(2) << day << '-' << std::setw(2) << month << '-'
	    << std::setw(4) << year;

	return oss.str();
}

static fs::path get_path(const std::string& source) {
	std::string contest{source_get_contest(source)};
	if (config["contest_format"][contest]) {
		auto expander = [&](const std::string& var) -> std::string {
			if (var == "date") {
				return source_get_date(source);
			} else if (var == "contest") {
				return contest;
			} else if (var == "year") {
				return source_get_year(source);
			} else if (var == "problem") {
				return source_get_problem(source);
			} else if (var == "source") {
				return source;
			} else {
				return "";
			}
		};
		return utils::expand_vars(config["contest_format"][contest].as<std::string>(),
		                          expander);
	} else if (contest.length()) {
		std::string year{source_get_year(source)};
		if (year.length()) {
			std::string problem{source_get_problem(source)};
			if (problem.length()) {
				return contest + "/" + contest + " " + year + "/" + contest + " " + year + " " +
				       problem;
			} else {
				return contest + "/" + source;
			}
		} else {
			return contest + "/" + source;
		}
	} else {
		return source;
	}
};

std::string get_problem_relative_path(const std::string& source) {
	fs::path path = get_path(source);
	return fs::path(path.replace_extension(utils::filetype_extension()));
}

fs::path get_problem_path(const std::string& source) {
	fs::path path = get_path(source);
	return fs::path(
	    fs::path(utils::expand_env_vars(config["base_path"].as<std::string>())) /
	    (path.replace_extension(utils::filetype_extension())));
}
