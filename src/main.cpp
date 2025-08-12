#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "oly/commands.hpp"
#include "oly/config.hpp"
#include "oly/log.hpp"

static void print_help() {
    std::println("usage: oly <cmd> [arg [...]].\n");
    std::println(R"(Available subcommands:
    add                          - add a problem to the database
    preview                      - generate a pdf file for a problem and open it
    search                       - search problems by tag, difficulty, contest...
    Run oly <cmd> --help for more information regarding a specific subcommand
    )");
    std::println(R"(Arguments:
    --help              -h       - Show this help message
    --config FILE       -c FILE  - Specify config file to use
    --verify-config              - Check wether the config has any errors
    --version           -v       - Print this binary's version)");
}

int main(int argc, char* argv[]) {
  std::vector<std::string> args{argv +1, argv + argc};

  Command* cmd;
  bool remove_cmd_name{true};
  if (args.empty()) {
    print_help();
    return -1;
  } else if (args[0] == "add") {
    cmd = Add{};
  } else if (args[0] == "edit") {
    cmd = Edit{};
  } else if (args[0] == "show") {
    cmd = Show{};
  } else if (args[0] == "gen") {
    cmd = Generate{};
  } else if (args[0] == "search") {
    cmd = Search{};
  } else if (args[0].starts_with("-") {
    cmd = Default{}
    remove_cmd_name = false;
  } else
    Log(severity::CRITICAL, "Unrecognized subcommand: " + args[0], logopts::HELP);
  }

  if (remove_cmd_name)
    args.erase(args.begin());

  cmd->parse(args);
  cmd->load_config_file();
  return cmd->execute();
}
