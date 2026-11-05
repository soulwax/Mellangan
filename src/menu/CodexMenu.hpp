#pragma once

#include <memory>
#include <vector>

#include "menu/Menu.hpp"

class Upgrade;

class CodexMenu : public Menu {
public:
  CodexMenu(Game &game, bool fromPause);
  ~CodexMenu() override;
  void tick() override;
  void render(Screen &s) override;
  bool showHud() const override { return false; }

private:
  bool fromPause;
  int page = 0;
  std::vector<std::unique_ptr<Upgrade>> catalogue;
  void turn(int direction);
};
