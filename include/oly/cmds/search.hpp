#pragma once

#include "oly/cmds/cmds.hpp"

class Search : public Command {
public:
  Search();
  int execute() override;
};
