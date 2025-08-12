#include "oly/commands.hpp"

class Default: public Command {
public:
  Default();
  int execute() override;
};
