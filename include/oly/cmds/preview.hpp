#include "oly/commands.hpp"

class Preview : public Command {
public:
  AddOptions();
  int execute() override;
};
