#include "oly/cmds/cmds.hpp"

class Default: public Command {
public:
  Default();
  int execute() override;
};
