#include "oly/cmds/default.hpp"
#include <print>

void Default::print_version() {
#if defined(OLY_NAME) && defined(OLY_VERSION) && defined(OLY_BUILD_TYPE)
	std::println("{} version {}", OLY_NAME, OLY_VERSION);
	std::println("Build type: {}", OLY_BUILD_TYPE);
#else
	static_assert(false, "ERROR: OLY_NAME, OLY_VERSION and OLY_BUILD_TYPE preprocessor "
	                     "macros must be defined");
#endif
}

Default::Default() {
	add("--version,-v", "print program version and related info",
	    [this] { this->print_version(); });
	add("--verify-config", "check config file", [this] {
		this->load_config_file();
		std::println("All good !");
	});
}

int Default::execute() {
	return 0;
}
