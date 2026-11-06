#include "menu/ControlsMenu.hpp"

#include <algorithm>

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "menu/SettingsMenu.hpp"

void ControlsMenu::tick() {
  Menu::tick();
  Input &in = game.input;
  if (waiting) {
    if (in.listening >= 0 || in.padListening >= 0)
      return;
    waiting = false;
    game.saveKeys();
    return Sound::confirm.play(0.5f, 0);
  }
  int count = (in.pad ? Input::PAD_COUNT : Input::BINDING_COUNT), rows = count + 1;
  row = std::min(row, count);
  if (in.escape.clicked) {
    Sound::select.play(0.5f, 0);
    return game.setMenu(std::make_unique<SettingsMenu>(game, fromPause));
  }
  if (in.up.clicked)
    row = (row + rows - 1) % rows, Sound::select.play(0.3f, 0);
  else if (in.down.clicked)
    row = (row + 1) % rows, Sound::select.play(0.3f, 0);
  else if (in.confirm.clicked && row == count)
    in.resetKeys(), game.saveKeys(), Sound::confirm.play(0.5f, 0);
  else if (in.confirm.clicked) {
    (in.pad ? in.padListening : in.listening) = row;
    waiting = true, Sound::select.play(0.5f, 0);
  }
}

void ControlsMenu::render(Screen &s) {
  const Input &in = game.input;
  bool pad = in.pad;
  int count = pad ? Input::PAD_COUNT : Input::BINDING_COUNT, rows = count + 1;
  float u = s.u, small = std::max(1.0f, u - 1), w = std::min(s.w - 32 * u, 300 * u),
        x = (s.w - w) / 2, line = 15 * small + 4 * u, top = s.h * 0.5f - line * rows / 2;
  s.rect(0, 0, s.w, s.h, {0.02f, 0.02f, 0.05f, fromPause ? 0.75f : 0.88f});
  s.textCenter(pad ? "GAMEPAD" : "CONTROLS", s.w / 2, top - 44 * u, 2 * u, col::gold);
  for (int i = 0; i < rows; i++) {
    float y = top + i * line;
    bool on = i == std::min(row, count);
    if (on)
      s.rect(x - 6 * u, y - 2 * u, w + 12 * u, line - u, col::panel.alpha(0.9f));
    Color ink = on ? col::gold : col::white;
    if (i == count) {
      s.text("RESET TO DEFAULTS", x, y, small, ink);
      continue;
    }
    bool changed;
    std::string key;
    if (pad) {
      s.text(Input::PAD_BINDINGS[i].name, x, y, small, ink);
      changed = in.padmap[i] != Input::PAD_BINDINGS[i].fallback;
      key = waiting && on ? "PRESS A BUTTON" : Input::padName(in.padmap[i]);
    } else {
      s.text(Input::BINDINGS[i].name, x, y, small, ink);
      changed = in.keymap[i] != Input::BINDINGS[i].fallback;
      key = waiting && on ? std::string("PRESS A KEY") : Input::keyName(in.keymap[i]);
    }
    s.text(key, x + w - Screen::textWidth(key, small), y, small,
           waiting && on ? col::gold : changed ? col::xp : ink);
  }
  const char *hint =
      pad ? (waiting ? "press the new button   B cancel"
                     : "D-pad choose   A rebind   B back   A, B, Start and the D-pad stay fixed")
          : (waiting ? "press the new key   Esc cancel"
                     : "Up/Down choose   Enter rebind   Esc back   arrows, Enter, Esc, 1-4, Q stay fixed");
  s.textCenter(hint, s.w / 2, top + rows * line + 16 * u, small, col::muted);
}
