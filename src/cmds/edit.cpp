#include "oly/cmds/edit.hpp"
#include "oly/utils.hpp"

Edit::Edit() {}

int Edit::execute() {
	for (std::string source : positional_args) {
		edit(get_problem_path(source));
	}
	return 0;
}
