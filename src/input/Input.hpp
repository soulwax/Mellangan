#pragma once

#include <SDL3/SDL.h>

#include <array>
#include <string>

#include "input/Key.hpp"

class Input {
public:
  Key up, down, left, right, confirm, escape, pause, one, two, three, four, restart,
      quit, map, codex, erase, older, newer, prior, next, fps, fullscreen, mute, devLevel, devTime, devGod, devWorld, devWarp;
  float mouseX = 0, mouseY = 0;
  bool mouseClicked = false, mouseMoved = false;
  std::string typed;

  void handle(const SDL_Event &e, float density);
  void tick();

private:
  int clicks = 0, moves = 0;
  std::string pending;

  std::array<Key *, 28> keys();
  Key *bind(SDL_Scancode sc);
};
