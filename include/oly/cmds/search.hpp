#include "oly/commands.hpp"

class Search : public Command {
public:
  Search();
  int execute() override;
};
