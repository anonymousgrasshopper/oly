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

Option::Option(std::string d, std::variant<bool, std::string>&& v)
    : desc(std::move(d)), value(std::forward<decltype(v)>(v)) {
	has_callback = false;
	if (std::holds_alternative<std::string>(value))
		requires_arg = true;
}
Option::Option(std::string d,
               std::variant<std::function<void()>, std::function<void(std::string)>>&& v)
    : desc(std::move(d)), callback(std::forward<decltype(v)>(v)) {
	has_callback = true;
	if (std::holds_alternative<std::function<void(std::string)>>(callback))
		requires_arg = true;
}

Command::Command() {
	add("--config-file,-c", "Specify config file to use", "~/.config/oly/config.yaml");
	add("--help,-h", "Show help", [this]() { Command::print_help(); });
	add("--log-level", "Specify log level (default INFO)",
	    [](std::string level) { utils::set_log_level(level); });
}

Command::~Command() = default;

int Command::execute() {
	return 0;
}

void Command::add(std::string flags, std::string desc, void (*callback)()) {
	add(flags, std::move(desc), std::function<void()>(callback));
}

void Command::add(std::string flags, std::string desc, void (*callback)(std::string)) {
	add(flags, std::move(desc), std::function<void(std::string)>(callback));
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
	std::string cmd = config["cmd"].as<std::string>();
	for (size_t i = 0; i < args.size(); ++i) {
		const std::string& arg = args[i];

		// Long option: --flag, --flag=value or --flag:value
		if (arg.starts_with("--")) {
			auto eq_pos = arg.find('=');
			if (eq_pos == std::string::npos)
				eq_pos = arg.find(':');
			std::string flag = (eq_pos != std::string::npos) ? arg.substr(0, eq_pos) : arg;

			if (!has(flag))
				Log::CRITICAL("Unknown flag : " + flag, logopt::HELP | logopt::NO_PREFIX, cmd);

			auto opt_ptr = lookup[flag];
			if (opt_ptr->requires_arg) {
				if (eq_pos != std::string::npos) {
					set(flag, arg.substr(eq_pos + 1));
				} else {
					if (i + 1 == args.size())
						Log::CRITICAL(flag + " requires an argument",
						              logopt::HELP | logopt::NO_PREFIX, cmd);
					set(flag, args[++i]);
				}
			} else {
				set(flag, true);
			}
			if (opt_ptr->has_callback) {
				if (opt_ptr->requires_arg) {
					// Pass the argument value to the callback
					std::get<std::function<void(std::string)>>(opt_ptr->callback)(
					    std::get<std::string>(opt_ptr->value));
				} else {
					std::get<std::function<void()>>(opt_ptr->callback)();
				}
			}
		}

		// Short options: -f or -abc
		else if (arg.size() > 1 && arg[0] == '-' && arg[1] != '-') {
			for (size_t j = 1; j < arg.size(); ++j) {
				std::string short_flag = "-" + std::string(1, arg[j]);
				if (!has(short_flag))
					Log::CRITICAL("Unknown flag : " + short_flag, logopt::HELP | logopt::NO_PREFIX,
					              cmd);

				auto opt_ptr = lookup[short_flag];
				if (opt_ptr->requires_arg) {
					if (j + 1 < arg.size()) {
						set(short_flag, arg.substr(j + 1));
						break; // rest of string consumed as argument
					} else {
						if (i + 1 >= args.size())
							Log::CRITICAL(short_flag + " requires an argument",
							              logopt::HELP | logopt::NO_PREFIX, cmd);
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
	std::string cmd = config["cmd"].as<std::string>();
	if (cmd == "default") {
		utils::print_help();
		return;
	}

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

	std::println("available arguments for {}:", cmd);

	for (size_t idx = 0; idx < storage.size(); ++idx) {
		const auto& opt = storage[idx];
		const auto& alias_str = alias_strings[idx];

		std::cout << std::left << std::setw(max_len + 2) << alias_str << opt->desc << "\n";
	}
}
