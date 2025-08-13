#include "oly/cmds/command.hpp"
#include "oly/config.hpp"
#include "oly/log.hpp"

#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

Option::Option(std::string desc, std::variant<bool, std::string> value)
    : desc(std::move(desc)), value(std::move(value)),
      requires_arg(std::holds_alternative<std::string>(this->value)) {}

Command::~Command() = default;
int Command::execute() {
	return 0;
}

template <typename... Aliases>
void Command::add(std::string primary_flag, Aliases... aliases, std::string desc,
                  std::variant<bool, std::string> default_value) {
	auto opt = std::make_shared<Option>(std::move(desc), std::move(default_value));
	opt->names.push_back(primary_flag);
	(opt->names.push_back(aliases), ...);

	storage.push_back(opt);

	lookup.emplace(primary_flag, opt);
	(lookup.emplace(aliases, opt), ...);
}

template <typename T> T Command::get(const std::string& flag) const {
	auto it = lookup.find(flag);
	if (it == lookup.end())
		throw std::invalid_argument{"Unknown option: " + flag};
	return std::get<T>(it->second->value);
}

bool Command::has(const std::string& flag) const {
	return lookup.find(flag) != lookup.end();
}

void Command::set(const std::string& flag, std::variant<bool, std::string> val) {
	auto it = lookup.find(flag);
	if (it == lookup.end())
		throw std::invalid_argument{"Unknown option: " + flag};
	it->second->value = std::move(val);
}

void Command::parse(std::vector<std::string> args) {
	for (size_t i = 0; i < args.size(); ++i) {
		const std::string& arg = args[i];

		// Long option: --flag, --flag=value or --flag:value
		if (arg.starts_with("--")) {
			auto eq_pos = arg.find('=');
			if (eq_pos == std::string::npos)
				eq_pos = arg.find(':');
			std::string flag = (eq_pos != std::string::npos) ? arg.substr(0, eq_pos) : arg;

			if (!has(flag))
				Log::Log(severity::CRITICAL, "Unknown flag : " + flag, logopt::CMD_HELP, cmd_name);

			auto opt_ptr = lookup[flag];
			if (opt_ptr->requires_arg) {
				if (eq_pos != std::string::npos) {
					set(flag, arg.substr(eq_pos + 1));
				} else {
					if (i + 1 == args.size())
						Log::Log(severity::CRITICAL, flag + " requires an argument", logopt::CMD_HELP,
						         cmd_name);
					set(flag, args[++i]);
				}
			} else {
				set(flag, true);
			}
		}

		// Short options: -f or -abc
		else if (arg.size() > 1 && arg[0] == '-' && arg[1] != '-') {
			for (size_t j = 1; j < arg.size(); ++j) {
				std::string short_flag = "-" + std::string(1, arg[j]);
				if (!has(short_flag))
					Log::Log(severity::CRITICAL, "Unknown flag : " + short_flag, logopt::CMD_HELP, cmd_name);

				auto opt_ptr = lookup[short_flag];
				if (opt_ptr->requires_arg) {
					if (j + 1 < arg.size()) {
						set(short_flag, arg.substr(j + 1));
						break; // rest of string consumed as argument
					} else {
						if (i + 1 >= args.size())
							Log::Log(severity::CRITICAL, short_flag + " requires an argument", logopt::CMD_HELP,
							         cmd_name);
						set(short_flag, args[++i]);
					}
				} else {
					set(short_flag, true);
				}
			}
		}

		// positional arg
		else {
			positional_args.push_back(arg);
		}
	}
}

void Command::load_config_file() {
	if (has("--config-file")) {
		config = load_config(get<std::string>("--config-file"));
	} else {
		config = load_config();
	}
}

void Command::print_help() const {
	size_t max_len = 0;
	std::vector<std::string> alias_strings;
	alias_strings.reserve(storage.size());

	for (const auto& opt : storage) {
		std::string joined;
		for (size_t i = 0; i < opt->names.size(); ++i) {
			joined += opt->names[i];
			if (i + 1 < opt->names.size())
				joined += ", ";
		}
		alias_strings.push_back(joined);
		max_len = std::max(max_len, joined.size());
	}

	for (size_t idx = 0; idx < storage.size(); ++idx) {
		const auto& opt = storage[idx];
		const auto& alias_str = alias_strings[idx];

		std::cout << std::left << std::setw(max_len + 2) << alias_str << opt->desc << " [default: "
		          << (opt->requires_arg ? std::get<std::string>(opt->value)
		                                : (std::get<bool>(opt->value) ? "true" : "false"))
		          << "]\n";
	}
}
