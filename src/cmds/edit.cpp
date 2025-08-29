#include "oly/cmds/edit.hpp"
#include "oly/utils.hpp"

Edit::Edit() {}

int Edit::execute() {
	load_config_file();

	for (std::string source : positional_args) {
		utils::edit(utils::get_problem_path(source));
	}
	return 0;
}
