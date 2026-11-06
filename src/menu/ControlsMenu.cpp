#include "menu/ControlsMenu.hpp"

#include <algorithm>

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "menu/SettingsMenu.hpp"

namespace {
constexpr int ROWS = Input::BINDING_COUNT + 1;
} // namespace

void ControlsMenu::tick() {
  Menu::tick();
  Input &in = game.input;
  if (waiting) {
    if (in.listening >= 0)
      return;
    waiting = false;
    game.saveKeys();
    return Sound::confirm.play(0.5f, 0);
  }
  if (in.escape.clicked) {
    Sound::select.play(0.5f, 0);
    return game.setMenu(std::make_unique<SettingsMenu>(game, fromPause));
  }
  if (in.up.clicked)
    row = (row + ROWS - 1) % ROWS, Sound::select.play(0.3f, 0);
  else if (in.down.clicked)
    row = (row + 1) % ROWS, Sound::select.play(0.3f, 0);
  else if (in.confirm.clicked && row == Input::BINDING_COUNT)
    in.resetKeys(), game.saveKeys(), Sound::confirm.play(0.5f, 0);
  else if (in.confirm.clicked)
    in.listening = row, waiting = true, Sound::select.play(0.5f, 0);
}

void ControlsMenu::render(Screen &s) {
  float u = s.u, small = std::max(1.0f, u - 1), w = std::min(s.w - 32 * u, 300 * u),
        x = (s.w - w) / 2, line = 15 * small + 4 * u, top = s.h * 0.5f - line * ROWS / 2;
  s.rect(0, 0, s.w, s.h, {0.02f, 0.02f, 0.05f, fromPause ? 0.75f : 0.88f});
  s.textCenter("CONTROLS", s.w / 2, top - 44 * u, 2 * u, col::gold);
  const Input &in = game.input;
  for (int i = 0; i < ROWS; i++) {
    float y = top + i * line;
    bool on = i == row;
    if (on)
      s.rect(x - 6 * u, y - 2 * u, w + 12 * u, line - u, col::panel.alpha(0.9f));
    Color ink = on ? col::gold : col::white;
    if (i == Input::BINDING_COUNT) {
      s.text("RESET TO DEFAULTS", x, y, small, ink);
      continue;
    }
    s.text(Input::BINDINGS[i].name, x, y, small, ink);
    bool changed = in.keymap[i] != Input::BINDINGS[i].fallback;
    std::string key = waiting && on ? std::string("PRESS A KEY") : Input::keyName(in.keymap[i]);
    s.text(key, x + w - Screen::textWidth(key, small), y, small,
           waiting && on ? col::gold : changed ? col::xp : ink);
  }
  s.textCenter(waiting ? "press the new key   Esc cancel"
                       : "Up/Down choose   Enter rebind   Esc back   arrows, Enter, Esc, 1-4, Q stay fixed",
               s.w / 2, top + ROWS * line + 16 * u, small, col::muted);
}
