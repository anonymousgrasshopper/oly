#include <algorithm>
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

namespace parsers {
static std::string get_contest(const std::string& source) {
	// Contest name: a string of letters and spaces beginning and ending with letters
	std::string contest;
	std::smatch match;

	static std::regex contest_regex(R"(\b([^0-9/\-_ ](?:[^0-9/\-_]*)[^0-9/\-_ ])\b)");
	if (std::regex_search(source, match, contest_regex)) {
		contest = match.str(1);
		if (opts.abbreviations.contains(contest)) {
			contest = opts.abbreviations[contest];
		} else {
			if (!opts.contest_format.contains(contest)) {
				if (contest.length() <= 4 && !contest.contains(' ')) {
					std::ranges::transform(contest, contest.begin(), ::toupper);
					// } else {
					// 	for (size_t i = 0; i < contest.length(); ++i) {
					// 		if (i == 0 || contest[i - 1] == ' ') {
					// 			contest[i] = std::toupper(contest[i]);
					// 		} else {
					// 			contest[i] = std::tolower(contest[i]);
					// 		}
					// 	}
				}
			}
		}
	}

	return contest;
}

static std::string get_year(const std::string& source) {
	// Year: 4 digits or 2 digits not part of a longer digit sequence
	std::string year;
	std::smatch match;

	static std::regex year_regex_4(R"((\b\d{4}\b))");
	static std::regex year_regex_2(R"((\b\d{2}\b))");
	if (std::regex_search(source, match, year_regex_4)) {
		year = match.str(1);
	} else if (std::regex_search(source, match, year_regex_2)) {
		year = "20" + match.str(1);
	} else {
		// No year found → use current year
		auto now = std::chrono::system_clock::now();
		std::time_t t = std::chrono::system_clock::to_time_t(now);
		std::tm tm = *std::localtime(&t);
		year = std::to_string(tm.tm_year + 1900);
	}

	return year;
}

static std::string get_problem(const std::string& source) {
	// Problem: single digit, possibly preceded by a letter among
	// P, A, C, G, N or a slash, followed by whitespace or end of string
	std::string problem;
	std::smatch match;

	static std::regex problem_regex(R"((^|\s)(([PACGN]|/)?(\d))(?=\s|$))");
	if (std::regex_search(source, match, problem_regex)) {
		if (!match[3].matched) {
			problem = "P" + match[4].str();
		} else {
			problem = match[3].str() + match[4].str();
			shared["topic"] = get_topic(match[3].str()[0]);
		}
	}

	return problem;
}

static std::string get_date(const std::string& source) {
	// Date: two sequaences of one or two digits optionally followed by a year
	// which is 2 or 4 digits, separated with slashes or dashes
	std::smatch match;
	static std::regex date_regex(R"((\d{1,2})[/-](\d{1,2})(?:[/-](\d{2,4}))?)");

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
} // namespace parsers

static std::map<std::string, fs::path> source_to_path;
static std::map<std::string, fs::path> source_to_name;

static fs::path get_path(const std::string& source) {
	if (source_to_path.contains(source))
		return source_to_path[source];

	std::string contest{parsers::get_contest(source)};
	if (opts.contest_format.contains(contest)) {
		auto expander = [&](const std::string& var) -> std::string {
			if (var == "date") {
				return parsers::get_date(source);
			} else if (var == "contest") {
				return contest;
			} else if (var == "year") {
				return parsers::get_year(source);
			} else if (var == "problem") {
				return parsers::get_problem(source);
			} else if (var == "source") {
				return source;
			} else {
				return "";
			}
		};
		source_to_path[source] = utils::expand_vars(opts.contest_format[contest], expander);
	} else if (!contest.empty()) {
		std::string year{parsers::get_year(source)};
		if (!year.empty()) {
			std::string problem{parsers::get_problem(source)};
			if (!problem.empty()) {
				source_to_path[source] = contest + "/" + contest + " " + year + "/" + contest +
				                         " " + year + " " + problem;
			} else {
				source_to_path[source] = contest + "/" + source;
			}
		} else {
			source_to_path[source] = contest + "/" + source;
		}
	} else {
		source_to_path[source] = source;
	}

	return source_to_path[source];
};

fs::path get_problem_path(const std::string& source) {
	fs::path base_path = utils::expand_env_vars(opts.base_path);
	fs::path source_path = get_path(source);

	return fs::path(base_path / source_path);
}

fs::path get_problem_solution_path(const std::string& source) {
	fs::path path = get_problem_path(source);
	path = path / ("solution" + utils::filetype_extension());
	return path;
}

std::string get_problem_name(const std::string& source) {
	if (source_to_name.contains(source))
		return source_to_name[source];

	std::string contest{parsers::get_contest(source)};
	if (opts.contest_format.contains(contest)) {
		std::string name = contest;

		std::set<std::string> ignored;
		std::string contest_format = opts.contest_format[contest];
		if (contest_format.contains("${date}"))
			// if date will be extracted, don't expand the year as well
			ignored.insert("year");

		static std::regex var(R"(\$\{([^}]+)\})");
		std::sregex_iterator it(contest_format.begin(), contest_format.end(), var);
		std::sregex_iterator end;

		for (; it != end; ++it) {
			const std::smatch& match = *it;
			std::string str = match[1].str();

			if (ignored.contains(str))
				continue;
			else
				ignored.insert(str);

			if (str == "date") {
				name.append(" " + parsers::get_date(source));
			} else if (str == "year") {
				name.append(" " + parsers::get_year(source));
			} else if (str == "problem") {
				name.append(" " + parsers::get_problem(source));
			} else if (str == "source") {
				name = source;
				break;
			}
		}

		source_to_name[source] = name;
	} else if (!contest.empty()) {
		std::string year{parsers::get_year(source)};
		if (!year.empty()) {
			std::string problem{parsers::get_problem(source)};
			if (!problem.empty()) {
				source_to_name[source] = contest + " " + year + " " + problem;
			}
		}
	}

	if (!source_to_name.contains(source))
		source_to_name[source] = source;

	return source_to_name[source];
}
