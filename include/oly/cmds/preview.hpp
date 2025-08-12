#include "oly/commands.hpp"

class Preview : public Command {
public:
  Preview();
  int execute() override;
};
