#include "menu/CodexMenu.hpp"

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "gui/Widget.hpp"
#include "item/Upgrade.hpp"
#include "level/Chart.hpp"
#include "menu/PauseMenu.hpp"
#include "menu/TitleMenu.hpp"

namespace {
constexpr int PAGE_COUNT = 2, LIST_ROWS = 13, RELIC_ROWS = 7;

SDL_FRect pageButton(const Screen &s, int direction) {
  float u = s.u, small = std::max(1.0f, u - 1), size = 22 * small;
  return {s.w * 0.5f + direction * 76 * small - size * 0.5f,
          s.h - 14 * u - 22 * small, size, size};
}

bool inside(const SDL_FRect &r, const Input &in) {
  SDL_FPoint m{in.mouseX, in.mouseY};
  return SDL_PointInRectFloat(&m, &r);
}


const char *const FOES[Codex::KINDS] = {
    "BAT",        "RAT",        "SLIME",      "SCORPION",
    "SPIDER",     "GHOST",      "CULTIST",    "DEMON",
    "KNIGHT",     "CYCLOPS",    "WOLF",       "ALPHA WOLF",
    "BURROWER",   "NECROMANCER","SKELETON",   "TREASURE THIEF",
    "MIRE LEECH","BOG LANTERN","GLASS MANTIS","DUST VULTURE","FROST WISP",
    "ICEBACK","SPORELING","MYCELIAL BRUTE","EMBER IMP","CINDER GOLEM"
};
} // namespace

void CodexMenu::turn(int direction) {
  page = (page + direction + PAGE_COUNT) % PAGE_COUNT;
  Sound::select.play(0.35f, 0);
}

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
    return;
  }
  if (in.left.clicked)
    turn(-1);
  else if (in.right.clicked)
    turn(1);
  else if (in.mouseClicked) {
    if (inside(pageButton(game.screen, -1), in))
      turn(-1);
    else if (inside(pageButton(game.screen, 1), in))
      turn(1);
  }
}

void CodexMenu::render(Screen &s) {
  const Codex &c = game.codex;
  float u = s.u, small = std::max(1.0f, u - 1), pad = 14 * u, row = 18 * small;
  s.rect(0, 0, s.w, s.h, {0.02f, 0.02f, 0.05f, 0.92f});
  s.text("CODEX", pad, pad, 2 * u, col::gold);
  s.text(Screen::fit(fmt::format("{} runs   {} dawn{}   best {}   best streak {}   {} slain",
                     c.runs, c.dawns, c.dawns == 1 ? "" : "s",
                     clockText(c.bestTime), c.bestStreak, c.kills()), s.w - 2 * pad, small),
         pad, pad + 36 * u, small, col::muted);

  float top = pad + 62 * u, colW = (s.w - 2 * pad) / 4;
  float start = top + 18 * small, footer = s.h - pad - 24 * small;
  row = std::min(row, (footer - start) / LIST_ROWS);
  auto header = [&](float x, const char *name, int have, int of) {
    s.text(fmt::format("{} {}/{}", name, have, of), x, top, small,
           have == of ? col::gold : col::white);
  };

  float x = pad, y = start;
  header(x, "REGIONS", c.count(c.biomes), BIOME_COUNT);
  for (int b = page * LIST_ROWS; b < std::min(int(BIOME_COUNT), (page + 1) * LIST_ROWS); b++, y += row) {
    bool known = c.biomes >> b & 1;
    if (known)
      s.rect(x, y + 4 * small, 8 * small, 8 * small, Chart::tone(BiomeId(b)));
    s.text(Screen::fit(known ? Chart::title(BiomeId(b)) : "? ? ?", colW - 20 * small, small), x + 12 * small, y, small,
           known ? col::white : col::muted * 0.6f);
  }

  x += colW, y = start;
  int relics = 0, found = 0;
  auto fallback = game.level && game.level->player ? std::vector<std::unique_ptr<Upgrade>>{} : Upgrade::all();
  auto &upgrades = fallback.empty() ? game.level->player->upgrades : fallback;
  for (auto &up : upgrades)
    if (up->rare)
      found += c.relics >> relics++ & 1;
  header(x, "RELICS", found, relics);
  int i = 0;
  float relicRow = std::min(row * 1.6f, (footer - start) / RELIC_ROWS);
  for (auto &up : upgrades) {
    if (!up->rare)
      continue;
    int index = i++;
    if (index < page * RELIC_ROWS || index >= (page + 1) * RELIC_ROWS)
      continue;
    bool known = c.relics >> index & 1;
    s.color(col::white.alpha(known ? 1 : 0.15f));
    s.sprite(up->icon, x, y - 2 * small, 0, 0, small, small);
    s.color(col::white);
    s.text(Screen::fit(known ? up->name : "? ? ?", colW - 28 * small, small), x + 20 * small, y, small,
           known ? col::gold : col::muted * 0.6f);
    y += relicRow;
  }

  x += colW, y = start;
  int seen = 0;
  for (int k : c.slain)
    seen += k > 0;
  header(x, "BESTIARY", seen, Codex::KINDS);
  for (int k = page * LIST_ROWS; k < std::min(Codex::KINDS, (page + 1) * LIST_ROWS); k++, y += row) {
    bool known = c.slain[k] > 0;
    std::string n = known ? std::to_string(c.slain[k]) : "";
    float countWidth = Screen::textWidth(n, small);
    s.text(Screen::fit(known ? FOES[k] : "? ? ?", colW - 16 * small - countWidth, small), x, y, small,
           known ? col::white : col::muted * 0.6f);
    if (known) {
      s.text(n, x + colW - 8 * small - countWidth, y, small,
             col::muted);
    }
  }

  x += colW, y = start;
  int visited = 0;
  for (int k : c.entered)
    visited += k > 0;
  header(x, "PLACES", visited, Codex::SITES);
  int siteRows = (Codex::SITES + PAGE_COUNT - 1) / PAGE_COUNT;
  for (int k = page * siteRows; k < std::min(Codex::SITES, (page + 1) * siteRows); k++, y += row) {
    bool known = c.entered[k] > 0;
    std::string n = known ? std::to_string(c.entered[k]) : "";
    float countWidth = Screen::textWidth(n, small);
    const char *name = STRUCTURES[k].name ? STRUCTURES[k].name : "UNKNOWN SITE";
    s.text(Screen::fit(known ? name : "? ? ?", colW - 16 * small - countWidth, small), x, y, small,
           known ? col::white : col::muted * 0.6f);
    if (known) {
      s.text(n, x + colW - 8 * small - countWidth, y, small,
             col::muted);
    }
  }

  s.text("C / Esc back", pad, s.h - pad - 16 * small, small, col::muted);
  s.textCenter(fmt::format("PAGE {} / {}", page + 1, PAGE_COUNT), s.w * 0.5f,
               s.h - pad - 16 * small, small, col::white);
  for (int direction : {-1, 1}) {
    SDL_FRect button = pageButton(s, direction);
    bool hover = inside(button, game.input);
    gui::TextButton(button, hover ? col::muted * 0.3f : col::panel,
                    hover ? col::gold : col::muted, small,
                    direction < 0 ? "<" : ">", button.y + 7 * small,
                    small, col::gold).render(s);
  }
}
