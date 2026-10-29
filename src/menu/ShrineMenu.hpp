#pragma once

#include <SDL3/SDL.h>

#include "gui/Widget.hpp"
#include "menu/Menu.hpp"

class ShrineMenu : public Menu {
public:
  explicit ShrineMenu(Game &game) : Menu(game) {}
  void tick() override;
  void render(Screen &s) override;
  void choose(int choice);
  float shrineEffect() const override { return 1.0f; }

private:
  gui::ChoiceList selection{3};
};
