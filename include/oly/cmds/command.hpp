#pragma once

#include "oly/config.hpp"
#include <concepts>
#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

struct Option {
	std::vector<std::string> names;
	std::string desc;
	std::variant<bool, std::string> value;
	std::variant<std::function<void()>, std::function<void(std::string)>> callback;
	bool requires_arg;
	bool has_callback;

	Option(std::string d, std::variant<bool, std::string>&& v)
	    : desc(std::move(d)), value(std::forward<decltype(v)>(v)) {
		has_callback = false;
		if (std::holds_alternative<std::string>(value))
			requires_arg = true;
	}
	Option(std::string d,
	       std::variant<std::function<void()>, std::function<void(std::string)>>&& v)
	    : desc(std::move(d)), callback(std::forward<decltype(v)>(v)) {
		has_callback = true;
		if (std::holds_alternative<std::function<void(std::string)>>(callback))
			requires_arg = true;
	}
};

class Command {
protected:
	std::vector<std::string> positional_args;

	std::vector<std::shared_ptr<Option>> storage;
	std::unordered_map<std::string, std::shared_ptr<Option>> lookup;

public:
	Command();

	virtual ~Command();

	virtual int execute();

	template <typename T>
	  requires((std::same_as<std::remove_cvref_t<T>, bool> ||
	            std::constructible_from<std::string, T &&>) &&
	           (!std::invocable<T>))
	void add(std::string flags, std::string desc, T&& default_value);

	template <typename Callable>
	  requires std::invocable<Callable, std::string>
	void add(std::string flags, std::string desc, Callable&& callback);

	template <std::invocable Callable>
	void add(std::string flags, std::string desc, Callable&& callback);

	void add(std::string flags, std::string desc, void (*callback)());

	void add(std::string flags, std::string desc, void (*callback)(std::string));

	template <typename T> T get(const std::string& opt) const;

	bool has(const std::string& flag) const;

	void set(const std::string& flag, std::variant<bool, std::string> val);

	void parse(std::vector<std::string> args);

	void load_config_file();

	void print_help() const;
};

template <typename T>
  requires((std::same_as<std::remove_cvref_t<T>, bool> ||
            std::constructible_from<std::string, T &&>) &&
           (!std::invocable<T>))
void Command::add(std::string flags, std::string desc, T&& default_value) {
	auto opt = std::make_shared<Option>(
	    std::move(desc), std::variant<bool, std::string>{std::forward<T>(default_value)});

	std::stringstream ss(flags);
	std::string flag;
	while (std::getline(ss, flag, ','))
		opt->names.push_back(flag);
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
	while (std::getline(ss, flag, ','))
		opt->names.push_back(flag);
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
	while (std::getline(ss, flag, ','))
		opt->names.push_back(flag);
	for (auto& n : opt->names)
		lookup[n] = opt;
	storage.push_back(std::move(opt));
}

template <typename T> T Command::get(const std::string& opt) const {
	if (opt.starts_with('-')) {
		auto it = lookup.find(opt);
		if (it == lookup.end())
			throw std::invalid_argument{"Unknown option: " + opt};
		return std::get<T>(it->second->value);
	} else {
		return config[opt].as<T>();
	}
}
