#include "menu/CodexMenu.hpp"

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "item/Upgrade.hpp"
#include "level/Chart.hpp"
#include "menu/PauseMenu.hpp"
#include "menu/TitleMenu.hpp"

namespace {
const char *const FOES[Codex::KINDS] = {
    "BAT",        "RAT",        "SLIME",      "SCORPION",
    "SPIDER",     "GHOST",      "CULTIST",    "DEMON",
    "KNIGHT",     "CYCLOPS",    "WOLF",       "ALPHA WOLF",
    "BURROWER",   "NECROMANCER","SKELETON",   "TREASURE THIEF"
};
} // namespace

void CodexMenu::tick() {
  Menu::tick();
  Input &in = game.input;
  if (in.escape.clicked || in.codex.clicked || in.confirm.clicked ||
      (fromPause && in.pause.clicked)) {
    Sound::select.play(0.5f, 0);
    if (fromPause)
      game.setMenu(std::make_unique<PauseMenu>(game));
    else
      game.setMenu(std::make_unique<TitleMenu>(game));
  }
}

void CodexMenu::render(Screen &s) {
  const Codex &c = game.codex;
  float u = s.u, small = std::max(1.0f, u - 1), pad = 14 * u, row = 18 * small;
  s.rect(0, 0, s.w, s.h, {0.02f, 0.02f, 0.05f, 0.92f});
  s.text("CODEX", pad, pad, 2 * u, col::gold);
  s.text(fmt::format("{} runs   {} dawn{}   best {}   best streak {}   {} slain",
                     c.runs, c.dawns, c.dawns == 1 ? "" : "s",
                     clockText(c.bestTime), c.bestStreak, c.kills()),
         pad, pad + 36 * u, small, col::muted);

  float top = pad + 62 * u, colW = (s.w - 2 * pad) / 4;
  auto header = [&](float x, const char *name, int have, int of) {
    s.text(fmt::format("{} {}/{}", name, have, of), x, top, small,
           have == of ? col::gold : col::white);
  };

  float x = pad, y = top + row * 1.5f;
  header(x, "REGIONS", c.count(c.biomes), BIOME_COUNT);
  for (int b = 0; b < BIOME_COUNT; b++, y += row) {
    bool known = c.biomes >> b & 1;
    if (known)
      s.rect(x, y + 4 * small, 8 * small, 8 * small, Chart::tone(BiomeId(b)));
    s.text(known ? Chart::title(BiomeId(b)) : "? ? ?", x + 12 * small, y, small,
           known ? col::white : col::muted * 0.6f);
  }

  x += colW, y = top + row * 1.5f;
  int relics = 0, found = 0;
  for (auto &up : game.level->player->upgrades)
    if (up->rare)
      found += c.relics >> relics++ & 1;
  header(x, "RELICS", found, relics);
  int i = 0;
  for (auto &up : game.level->player->upgrades) {
    if (!up->rare)
      continue;
    bool known = c.relics >> i++ & 1;
    s.color(col::white.alpha(known ? 1 : 0.15f));
    s.sprite(up->icon, x, y - 2 * small, 0, 0, small, small);
    s.color(col::white);
    s.text(known ? up->name : "? ? ?", x + 20 * small, y, small,
           known ? col::gold : col::muted * 0.6f);
    y += row * 1.4f;
  }

  x += colW, y = top + row * 1.5f;
  int seen = 0;
  for (int k : c.slain)
    seen += k > 0;
  header(x, "BESTIARY", seen, Codex::KINDS);
  for (int k = 0; k < Codex::KINDS; k++, y += row) {
    bool known = c.slain[k] > 0;
    s.text(known ? FOES[k] : "? ? ?", x, y, small,
           known ? col::white : col::muted * 0.6f);
    if (known) {
      std::string n = std::to_string(c.slain[k]);
      s.text(n, x + colW - pad - Screen::textWidth(n, small), y, small,
             col::muted);
    }
  }

  x += colW, y = top + row * 1.5f;
  int visited = 0;
  for (int k : c.entered)
    visited += k > 0;
  header(x, "PLACES", visited, Codex::SITES);
  for (int k = 0; k < Codex::SITES; k++, y += row) {
    bool known = c.entered[k] > 0;
    s.text(known ? STRUCTURES[k].name : "? ? ?", x, y, small,
           known ? col::white : col::muted * 0.6f);
    if (known) {
      std::string n = std::to_string(c.entered[k]);
      s.text(n, x + colW - pad - Screen::textWidth(n, small), y, small,
             col::muted);
    }
  }

  s.text("C / Esc back", pad, s.h - pad - 16 * small, small, col::muted);
}
