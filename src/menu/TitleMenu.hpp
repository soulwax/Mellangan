#pragma once

#include <SDL3/SDL.h>

#include <string_view>

#include "menu/Menu.hpp"

class TitleMenu : public Menu {
public:
  explicit TitleMenu(Game &game);
  ~TitleMenu() override;
  void tick() override;
  void render(Screen &s) override;
  bool showHud() const override { return false; }
  bool typing() const override;

  float veil = 0;

private:
  static constexpr int MAX_CODE = 12;

  struct Layout {
    SDL_FRect panel, chips[4], seed, diff, rows[6];
  };
  int pick = -1, saved = -1;
  std::string summary;

  Layout layout() const;
  void choose(int mode);
  void recall(int i);
  void stepDifficulty(int dir);
  void heading(Screen &s, const Layout &l, float y, const char *label,
               std::string_view hint) const;
  void renderModes(Screen &s, const Layout &l) const;
  void renderSeed(Screen &s, const Layout &l) const;
  void renderDifficulty(Screen &s, const Layout &l) const;
  void renderHistory(Screen &s, const Layout &l) const;
};
