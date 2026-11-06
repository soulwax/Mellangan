#pragma once

#include "menu/Menu.hpp"

class ControlsMenu : public Menu {
public:
  ControlsMenu(Game &game, bool fromPause) : Menu(game), fromPause(fromPause) {}
  void tick() override;
  void render(Screen &s) override;
  bool showHud() const override { return false; }
  float pauseEffect() const override { return fromPause ? 1.0f : 0.0f; }

private:
  bool fromPause, waiting = false;
  int row = 0;
};
