#pragma once

#include "menu/Menu.hpp"

class CodexMenu : public Menu {
public:
  CodexMenu(Game &game, bool fromPause) : Menu(game), fromPause(fromPause) {}
  void tick() override;
  void render(Screen &s) override;
  bool showHud() const override { return false; }

private:
  bool fromPause;
};
