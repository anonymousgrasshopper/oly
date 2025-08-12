#pragma once

#include <string>
#include <variant>
#include <vector>
#include <unordered_map>

#include "yaml-cpp/yaml.h"

struct Option {
    Option(std::string desc, std::variant<bool, std::string> value);
    std::string desc;
    std::variant<bool, std::string> value;
    bool requires_arg;
    std::vector<std::string> names
};

class Command {
protected:
    std::vector<std::shared_ptr<Option>> storage;
    std::unordered_map<std::string, std::shared_ptr<Option>> lookup;
    YAML::Node config;

public:
    virtual int execute();

    template <typename... Aliases>
    void add(std::string primary_flag, Aliases... aliases,
             std::string desc, std::variant<bool, std::string> default_value);

    template <typename T>
    T get(const std::string& flag) const;

    bool has(const std::string& flag) const;

    void set(const std::string& flag, std::variant<bool, std::string> val);

    void load_config_file();

    void print_help() const;
};

#include "oly/cmds/add.hpp"
#include "oly/cmds/preview.hpp"
#include "oly/cmds/search.hpp"
#include "oly/cmds/default.hpp"
