#pragma once

#include "oly/cmds/cmds.hpp"

class Show: public Command {
public:
  Show();
  int execute() override;
};
