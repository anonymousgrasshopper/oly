#pragma once

#include <concepts>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "yaml-cpp/yaml.h"

struct Option {
	std::vector<std::string> names;
	std::string desc;
	std::variant<bool, std::string> value;
	std::function<void()> callback;
	std::function<void(std::string)> arg_callback;
	bool requires_arg;
	bool has_callback;

	Option(std::string desc, std::variant<bool, std::string> val)
	    : desc(std::move(desc)), value(std::move(val)), has_callback(false) {
		requires_arg = std::holds_alternative<std::string>(value);
	}

	Option(std::string desc, std::function<void()> callback)
	    : desc(std::move(desc)), callback(std::move(callback)), requires_arg(false),
	      has_callback(true) {}

	Option(std::string desc, std::function<void(std::string)> callback)
	    : desc(std::move(desc)), arg_callback(std::move(callback)), requires_arg(true),
	      has_callback(true) {}
};

class Command {
protected:
	static inline const std::string cmd_name;
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

	template <typename T> T get(const std::string& flag) const;

	bool has(const std::string& flag) const;

	void set(const std::string& flag, std::variant<bool, std::string> val);

	void parse(std::vector<std::string> args);

	void load_config_file();

	void print_help() const;
};
