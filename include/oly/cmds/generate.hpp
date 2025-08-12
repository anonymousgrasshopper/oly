#pragma once

#include "oly/cmds/cmds.hpp"

class Generate: public Command {
public:
  Generate();
  int execute() override;
};
