#pragma once

#include <SDL3/SDL.h>

#include "menu/Menu.hpp"

class TitleMenu : public Menu {
public:
  explicit TitleMenu(Game &game);
  ~TitleMenu() override;
  void tick() override;
  void render(Screen &s) override;
  bool showHud() const override { return false; }
  bool typing() const override;

private:
  static constexpr int MAX_CODE = 12;

  struct Layout {
    SDL_FRect panel, chips[4], seed, rows[6];
  };
  int pick = -1;

  Layout layout() const;
  void choose(int mode);
  void recall(int i);
};
