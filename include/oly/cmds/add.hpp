#include "oly/cmds/cmds.hpp"

class Add : public Command {
public:
  Add();
  int execute() override;
};
