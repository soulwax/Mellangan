#include "menu/SettingsMenu.hpp"

#include <spdlog/fmt/fmt.h>

#include <algorithm>

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "menu/ControlsMenu.hpp"
#include "menu/PauseMenu.hpp"
#include "menu/TitleMenu.hpp"

namespace {
struct Row {
  const char *name;
  int Settings::*value;
  int step, hi;
  const char *off, *on;
};

const Row ROWS[] = {
    {"MASTER VOLUME", &Settings::master, 10, 100, nullptr, nullptr},
    {"MUSIC", &Settings::music, 10, 100, nullptr, nullptr},
    {"EFFECTS", &Settings::effects, 10, 100, nullptr, nullptr},
    {"AMBIENCE", &Settings::ambience, 10, 100, nullptr, nullptr},
    {"SCREEN SHAKE", &Settings::shake, 25, 100, nullptr, nullptr},
    {"FLASHES", &Settings::flash, 25, 100, nullptr, nullptr},
    {"PARTICLES", &Settings::reduced, 1, 1, "FULL", "REDUCED"},
    {"DAMAGE NUMBERS", &Settings::numbers, 1, 1, "OFF", "ON"},
    {"COLOURS", &Settings::colours, 1, 1, "STANDARD", "COLOURBLIND"},
    {"FULLSCREEN", &Settings::fullscreen, 1, 1, "OFF", "ON"},
    {"SCALING", &Settings::pixel, 1, 1, "FILL", "PIXEL PERFECT"},
    {"VSYNC", &Settings::vsync, 1, 1, "OFF", "ON"},
    {"CONTROLS", nullptr, 0, 0, nullptr, nullptr}};
constexpr int ROW_COUNT = int(sizeof ROWS / sizeof *ROWS);
} // namespace

void SettingsMenu::change(int direction) {
  const Row &r = ROWS[row];
  if (!r.value) {
    if (direction < 0)
      return;
    Sound::select.play(0.5f, 0);
    return game.setMenu(std::make_unique<ControlsMenu>(game, fromPause));
  }
  int &v = game.settings.*r.value;
  int was = v;
  v = r.hi == 1 ? 1 - v : std::clamp(v + direction * r.step, 0, r.hi);
  if (v == was)
    return;
  game.applySettings();
  game.saveSettings();
  Sound::select.play(0.5f, 0);
}

void SettingsMenu::tick() {
  Menu::tick();
  Input &in = game.input;
  if (in.escape.clicked || in.options.clicked || (fromPause && in.pause.clicked)) {
    Sound::select.play(0.5f, 0);
    if (fromPause)
      return game.setMenu(std::make_unique<PauseMenu>(game));
    return game.setMenu(std::make_unique<TitleMenu>(game));
  }
  if (in.up.clicked)
    row = (row + ROW_COUNT - 1) % ROW_COUNT, Sound::select.play(0.3f, 0);
  else if (in.down.clicked)
    row = (row + 1) % ROW_COUNT, Sound::select.play(0.3f, 0);
  else if (in.left.clicked)
    change(-1);
  else if (in.right.clicked || (in.confirm.clicked && ROWS[row].hi <= 1))
    change(1);
}

void SettingsMenu::render(Screen &s) {
  float u = s.u, small = std::max(1.0f, u - 1), w = std::min(s.w - 32 * u, 300 * u),
        x = (s.w - w) / 2, line = 22 * u, top = s.h * 0.5f - line * ROW_COUNT / 2;
  s.rect(0, 0, s.w, s.h, {0.02f, 0.02f, 0.05f, fromPause ? 0.75f : 0.88f});
  s.textCenter("SETTINGS", s.w / 2, top - 48 * u, 2 * u, col::gold);
  for (int i = 0; i < ROW_COUNT; i++) {
    const Row &r = ROWS[i];
    float y = top + i * line;
    bool on = i == row;
    if (on)
      s.rect(x - 6 * u, y - 3 * u, w + 12 * u, line - 2 * u, col::panel.alpha(0.9f));
    Color ink = on ? col::gold : col::white;
    s.text(r.name, x, y, u, ink);
    if (!r.value) {
      std::string open = game.input.prompt("Enter");
      s.text(open, x + w - Screen::textWidth(open, u), y, u, ink);
      continue;
    }
    int v = game.settings.*r.value;
    float bw = 90 * u, bx = x + w - bw;
    if (r.hi == 1) {
      std::string label = v ? r.on : r.off;
      s.text(label, x + w - Screen::textWidth(label, u), y, u, ink);
      continue;
    }
    s.rect(bx - 40 * u, y + 5 * u, bw, 6 * u, {0.07f, 0.06f, 0.11f, 0.9f});
    s.rect(bx - 40 * u, y + 5 * u, bw * v / r.hi, 6 * u, on ? col::gold : col::muted);
    std::string pct = fmt::format("{}%", v);
    s.text(pct, x + w - Screen::textWidth(pct, u), y, u, ink);
  }
  s.textCenter(game.input.pad ? "D-pad choose and change   B back"
                               : "Up/Down choose   Left/Right change   Esc back",
               s.w / 2,
               top + ROW_COUNT * line + 24 * u, small, col::muted);
}
