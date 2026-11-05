#pragma once

#include "gfx/Screen.hpp"

class Game;
class Hero;
class Upgrade;

class HeroCard {
public:
  HeroCard(Game &game, const Screen &s, int pick, float t, float in);
  void render(Screen &s);

private:
  Game &game;
  const Hero &h;
  int pick;
  float t, in, u, small, x, top, w, pad, line, body, gut;
  bool open, dense = false;
  Color tint;

  const Upgrade *named(const char *name) const;
  float banner(Screen &s, bool draw) const;
  float label(Screen &s, const char *text, float y, bool draw) const;
  float arsenal(Screen &s, float y, bool draw) const;
  float nature(Screen &s, float y, bool draw) const;
  float layout(Screen &s, bool draw) const;
};
