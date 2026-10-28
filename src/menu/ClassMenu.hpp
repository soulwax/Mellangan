#pragma once

#include <SDL3/SDL.h>

#include "hero/Hero.hpp"
#include "menu/Menu.hpp"

class ClassMenu : public Menu {
public:
  explicit ClassMenu(Game &game);
  void tick() override;
  void render(Screen &s) override;
  bool showHud() const override { return false; }

private:
  int pick = 0;
  float grow[HERO_COUNT]{};

  SDL_FRect slot(int i) const;
  void choose(int i);
  void begin();
};
