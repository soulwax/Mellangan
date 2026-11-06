#pragma once

#include <SDL3/SDL.h>

#include <array>
#include <string>
#include <string_view>

#include "input/Key.hpp"

class Input {
public:
  Key up, down, left, right, confirm, escape, pause, one, two, three, four, restart,
      quit, map, codex, erase, older, newer, prior, next, fps, fullscreen, mute, particles, devLevel, devTime, devGod, devWorld, devWarp, devSky, loot, inventory, campaign, gather, build, options;
  float mouseX = 0, mouseY = 0, stickX = 0, stickY = 0;
  bool mouseClicked = false, mouseMoved = false, pad = false;
  std::string typed;

  void handle(const SDL_Event &e, float density);
  std::string prompt(std::string_view text) const;
  void tick();

private:
  int clicks = 0, moves = 0;
  std::string pending;

  float axis[4]{};
  bool held[6]{};

  std::array<Key *, 36> keys();
  Key *bind(SDL_Scancode sc);
  int bind(SDL_GamepadButton button, Key **out);
  void steer();
};
