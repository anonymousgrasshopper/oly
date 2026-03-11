#include <iostream>
#include <print>

#include "oly/cmds/default.hpp"
#include "oly/cmds/get_cmd.hpp"
#include "oly/config.hpp"
#include "oly/contest.hpp"
#include "oly/log.hpp"

void Default::print_version() {
#if defined(OLY_NAME) && defined(OLY_VERSION) && defined(OLY_BUILD_TYPE) &&              \
    defined(OLY_COMMIT_HASH)
	std::println("{} version {}", OLY_NAME, OLY_VERSION);
	std::println("Build type: {}", OLY_BUILD_TYPE);
	std::println("Commit hash: {}", OLY_COMMIT_HASH);
#else
	static_assert(false, "ERROR: OLY_NAME, OLY_VERSION and OLY_BUILD_TYPE preprocessor "
	                     "macros must be defined");
#endif
}

Default::Default() {
	add("--version,-v", "print program version and related info",
	    [this] { this->print_version(); });
	add("--verify-config", "check config file", [this] {
		configuration::load_config(get<std::string>("--config-file"));
		std::println("All good !");
	});
	add(
	    "--scheme", "use the scheme handler",
	    [this](std::string request) {
		    configuration::load_config(get<std::string>("--config-file"));

		    if (request.empty()) {
			    std::string pb_name;
			    std::cout << "Enter problem name: " << std::flush;
			    std::getline(std::cin, pb_name);
			    fs::path pb_path = get_problem_path(pb_name);
			    if (fs::exists(pb_path)) {
				    request = "oly://edit?name=" + pb_name;
			    } else {
				    request = "oly://add?name=" + pb_name;
			    }
		    } else {
			    Log::INFO("received request: " + request);
		    }

		    std::string url = request.substr(6);
		    size_t mark = url.find("?");
		    size_t equals = url.find("=", mark);
		    if (mark == std::string::npos || equals == std::string::npos)
			    Log::CRITICAL("malformed query: expected format oly://cmd?name=<problem name>");

		    std::string cmd_name = url.substr(0, mark);
		    std::string pb_name = url.substr(equals + 1);
		    // std::system(("notify-send '" + cmd + "'").c_str());
		    // std::system(("notify-send '" + pb_name + "'").c_str());

		    std::vector<std::string> args{pb_name};
		    std::unique_ptr<Command> cmd = get_cmd(cmd_name);

		    setenv("OLY", shared["cmd"].c_str(), 1);

		    return cmd->execute(args);
	    },
	    false);
}

int Default::execute(std::vector<std::string>& args) {
	parse(args);
	return 0;
}
