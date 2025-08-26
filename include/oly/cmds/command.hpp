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
	std::string desc;
	std::vector<std::string> names;
	std::variant<bool, std::string> value;
	std::function<void()> callback; // Optional callback
	bool requires_arg;

	Option(std::string desc, std::variant<bool, std::string> val)
	    : desc(std::move(desc)), value(std::move(val)) {
		requires_arg = std::holds_alternative<std::string>(value);
	}

	Option(std::string desc, std::function<void()> callback)
	    : desc(std::move(desc)), callback(std::move(callback)), requires_arg(false) {}
};

class Command {
protected:
	static inline const std::string cmd_name;
	std::vector<std::string> positional_args;
	YAML::Node config;

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

	template <typename F>
	  requires(std::invocable<F>)
	void add(std::string flags, std::string desc, F&& callback);

	template <typename T> T get(const std::string& flag) const;

	bool has(const std::string& flag) const;

	void set(const std::string& flag, std::variant<bool, std::string> val);

	void parse(std::vector<std::string> args);

	void load_config_file();

	void print_help() const;
};
