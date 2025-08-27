#include "oly/cmds/command.hpp"
#include "oly/config.hpp"
#include "oly/log.hpp"
#include "oly/utils.hpp"

#include <cassert>
#include <iomanip>
#include <iostream>
#include <memory>
#include <ostream>
#include <print>
#include <stdexcept>
#include <string>
#include <vector>

Command::Command() {
	add("--config-file,-c", "Specify config file to use", "~/.config/oly/config.yaml");
	add("--help,-h", "Show help", [this]() { Command::print_help(); });
	add("--log-level", "Specify log level (default INFO)",
	    [](std::string level) { set_log_level(level); });
}

Command::~Command() = default;

int Command::execute() {
	return 0;
}

template <typename T>
  requires((std::same_as<std::remove_cvref_t<T>, bool> ||
            std::constructible_from<std::string, T &&>) &&
           (!std::invocable<T>))
void Command::add(std::string flags, std::string desc, T&& default_value) {
	auto v = std::forward<T>(default_value);
	auto opt = std::make_shared<Option>(std::move(desc),
	                                    std::variant<bool, std::string>{std::move(v)});

	std::stringstream ss(flags);
	std::string flag;
	while (std::getline(ss, flag, ',')) {
		opt->names.push_back(flag);
	}
	for (auto& n : opt->names)
		lookup[n] = opt;
	storage.push_back(std::move(opt));
}

template <typename Callable>
  requires std::invocable<Callable, std::string>
void Command::add(std::string flags, std::string desc, Callable&& callback) {
	auto opt = std::make_shared<Option>(
	    std::move(desc),
	    std::function<void(std::string)>(std::forward<Callable>(callback)));

	std::stringstream ss(flags);
	std::string flag;
	while (std::getline(ss, flag, ',')) {
		opt->names.push_back(flag);
	}
	for (auto& n : opt->names)
		lookup[n] = opt;
	storage.push_back(std::move(opt));
}

template <std::invocable Callable>
void Command::add(std::string flags, std::string desc, Callable&& callback) {
	auto opt = std::make_shared<Option>(
	    std::move(desc), std::function<void()>(std::forward<Callable>(callback)));

	std::stringstream ss(flags);
	std::string flag;
	while (std::getline(ss, flag, ',')) {
		opt->names.push_back(flag);
	}
	for (auto& n : opt->names)
		lookup[n] = opt;
	storage.push_back(std::move(opt));
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
				Log::Log(severity::CRITICAL, "Unknown flag : " + flag, logopt::CMD_HELP,
				         cmd_name);

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
			if (opt_ptr->has_callback) {
				if (opt_ptr->requires_arg) {
					// Pass the argument value to the callback
					opt_ptr->arg_callback(std::get<std::string>(opt_ptr->value));
				} else {
					opt_ptr->callback();
				}
			}
		}

		// Short options: -f or -abc
		else if (arg.size() > 1 && arg[0] == '-' && arg[1] != '-') {
			for (size_t j = 1; j < arg.size(); ++j) {
				std::string short_flag = "-" + std::string(1, arg[j]);
				if (!has(short_flag))
					Log::Log(severity::CRITICAL, "Unknown flag : " + short_flag, logopt::CMD_HELP,
					         cmd_name);

				auto opt_ptr = lookup[short_flag];
				if (opt_ptr->requires_arg) {
					if (j + 1 < arg.size()) {
						set(short_flag, arg.substr(j + 1));
						break; // rest of string consumed as argument
					} else {
						if (i + 1 >= args.size())
							Log::Log(severity::CRITICAL, short_flag + " requires an argument",
							         logopt::CMD_HELP, cmd_name);
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

	std::println("available arguments for {}:", cmd_name);

	for (size_t idx = 0; idx < storage.size(); ++idx) {
		const auto& opt = storage[idx];
		const auto& alias_str = alias_strings[idx];

		std::cout << std::left << std::setw(max_len + 2) << alias_str << opt->desc << "\n";
	}
}
