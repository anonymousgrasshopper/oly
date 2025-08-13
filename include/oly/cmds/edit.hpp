#pragma once

#include "oly/cmds/cmds.hpp"

class Edit : public Command {
public:
	Edit();
	int execute() override;
};
