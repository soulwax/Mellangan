#pragma once

#include <SDL3/SDL.h>

#include <string>
#include <vector>

#include "gfx/Screen.hpp"

class Player;
class Upgrade;

class UpgradeCard {
public:
  UpgradeCard(const Upgrade *up, const Player &p, bool evolving, int index);
  void render(Screen &s, SDL_FRect r, float in, bool selected, float t) const;
  std::string synergy(bool &ready) const;
  std::vector<std::string> numbers() const;

private:
  const Upgrade *up;
  const Player &p;
  bool evolving, classy = false;
  int index, symbol;
  Color tint;

  void renderThread(Screen &s, SDL_FRect r, float in, float t) const;
  void renderTag(Screen &s, float right, float y, float in) const;
};
