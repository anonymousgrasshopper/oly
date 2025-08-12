#include "oly/commands.hpp"

class Add : public Command {
public:
  Add();
  int execute() override;
};
