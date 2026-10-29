#pragma once

#include "Math.hpp"

class Game;
class Screen;

class Menu {
public:
  Game &game;
  float t = 0;

  explicit Menu(Game &game) : game(game) {}
  virtual ~Menu() = default;
  virtual void tick() { t += DT; }
  virtual void render(Screen &s) = 0;
  virtual bool showHud() const { return true; }
  virtual bool typing() const { return false; }
  virtual float pauseEffect() const { return 0; }
  virtual float shrineEffect() const { return 0; }
};
