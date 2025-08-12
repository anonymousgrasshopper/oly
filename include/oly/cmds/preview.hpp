#pragma once

#include "oly/cmds/cmds.hpp"

class Preview : public Command {
public:
  Preview();
  int execute() override;
};
