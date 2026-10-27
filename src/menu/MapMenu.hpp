#pragma once

#include "menu/Menu.hpp"

class MapMenu : public Menu {
public:
  using Menu::Menu;
  void tick() override;
  void render(Screen &s) override;
  bool showHud() const override { return false; }

private:
  double xPan = 0, yPan = 0;
  int zoom = 1;
};
