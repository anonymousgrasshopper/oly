#include "oly/commands.hpp"

class Add : public Command {
public:
  AddOptions();
  int execute() override;
};
