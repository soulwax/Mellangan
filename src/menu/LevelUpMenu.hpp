#pragma once

#include <SDL3/SDL.h>

#include <array>
#include <memory>
#include <vector>

#include "menu/Menu.hpp"
#include "gui/Widget.hpp"

class Upgrade;

class LevelUpMenu : public Menu {
public:
  explicit LevelUpMenu(Game &game);
  static std::unique_ptr<Menu> open(Game &game);
  void tick() override;
  void render(Screen &s) override;
  float pauseEffect() const override { return 1.0f; }

private:
  std::vector<Upgrade *>
      choices; // nullptr: every upgrade is maxed, offer a feast
  gui::ChoiceList selection;

  bool treasure = false;
  int evolveSlot = -1;

  void apply(int i);
  void choose(int i);
};
