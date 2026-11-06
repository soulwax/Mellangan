#pragma once

#include <SDL3/SDL.h>

#include <array>
#include <string>
#include <string_view>

#include "input/Key.hpp"

class Input {
public:
  struct Binding {
    const char *name, *word;
    Key Input::*key;
    SDL_Scancode fallback;
  };
  static const Binding BINDINGS[];
  static constexpr int BINDING_COUNT = 17;

  struct PadBinding {
    const char *name;
    Key Input::*first, Input::*second;
    int fallback;
    const char *words;
  };
  static constexpr int LT = SDL_GAMEPAD_BUTTON_COUNT, RT = LT + 1, PAD_COUNT = 8;
  static const PadBinding PAD_BINDINGS[];

  Key up, down, left, right, confirm, escape, pause, one, two, three, four, restart,
      quit, map, codex, erase, older, newer, prior, next, fps, fullscreen, mute, particles, devLevel, devTime, devGod, devWorld, devWarp, devSky, loot, inventory, campaign, gather, build, options;
  float mouseX = 0, mouseY = 0, stickX = 0, stickY = 0;
  bool mouseClicked = false, mouseMoved = false, pad = false;
  std::string typed;

  SDL_Scancode keymap[BINDING_COUNT];
  int padmap[PAD_COUNT];
  int listening = -1, padListening = -1;

  Input();
  void handle(const SDL_Event &e, float density);
  bool rebind(int binding, SDL_Scancode sc);
  bool rebindPad(int binding, int button);
  static const char *padName(int button);
  void resetKeys();
  std::string saveKeys() const;
  void parseKeys(std::string_view text);
  static std::string keyName(SDL_Scancode sc);
  std::string prompt(std::string_view text) const;
  void tick();

private:
  int clicks = 0, moves = 0;
  std::string pending;

  float axis[4]{};
  bool held[6]{};

  std::array<Key *, 36> keys();
  Key *bind(SDL_Scancode sc);
  Key *fixed(SDL_Scancode sc);
  int bind(int button, Key **out);
  void press(int button, bool pressed);
  void steer();
};
