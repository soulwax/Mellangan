#include "menu/CodexMenu.hpp"

#include <cctype>
#include <string>
#include <string_view>

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/enemy/Enemy.hpp"
#include "entity/Player.hpp"
#include "gui/Widget.hpp"
#include "item/Upgrade.hpp"
#include "level/Chart.hpp"
#include "menu/PauseMenu.hpp"
#include "menu/TitleMenu.hpp"

namespace {
constexpr int LIST_ROWS = 13, RELIC_ROWS = 7,
              PAGE_COUNT = (Codex::KINDS + LIST_ROWS - 1) / LIST_ROWS;

SDL_FRect pageButton(const Screen &s, int direction) {
  float u = s.u, small = std::max(1.0f, u - 1), size = 22 * small;
  return {s.w * 0.5f + direction * 76 * small - size * 0.5f,
          s.h - 14 * u - 22 * small, size, size};
}

bool inside(const SDL_FRect &r, const Input &in) {
  SDL_FPoint m{in.mouseX, in.mouseY};
  return SDL_PointInRectFloat(&m, &r);
}


struct Grid {
  float u, small, pad, row, top, start, colW, footer;
};

std::string foeTitle(int kind) {
  std::string_view n = Enemy::nameOf(EnemyKind(kind));
  for (std::string_view article : {"a ", "an ", "the "})
    if (n.starts_with(article)) {
      n.remove_prefix(article.size());
      break;
    }
  std::string out(n);
  for (char &c : out)
    c = char(std::toupper(static_cast<unsigned char>(c)));
  return out;
}

void heading(Screen &s, const Grid &g, float x, const char *name, int have, int of) {
  s.text(fmt::format("{} {}/{}", name, have, of), x, g.top, g.small,
         have == of ? col::gold : col::white);
}

void regions(Screen &s, const Grid &g, const Codex &c, float x, int page) {
  heading(s, g, x, "REGIONS", c.count(c.biomes), BIOME_COUNT);
  float y = g.start;
  for (int b = page * LIST_ROWS; b < std::min(int(BIOME_COUNT), (page + 1) * LIST_ROWS);
       b++, y += g.row) {
    bool known = c.biomes >> b & 1;
    if (known)
      s.rect(x, y + 4 * g.small, 8 * g.small, 8 * g.small, Chart::tone(BiomeId(b)));
    s.text(Screen::fit(known ? Chart::title(BiomeId(b)) : "? ? ?", g.colW - 20 * g.small, g.small),
           x + 12 * g.small, y, g.small, known ? col::white : col::muted * 0.6f);
  }
}

void relics(Screen &s, const Grid &g, const Codex &c, float x, int page,
            const std::vector<std::unique_ptr<Upgrade>> &upgrades) {
  int total = 0, found = 0;
  for (auto &up : upgrades)
    if (up->rare)
      found += c.relics >> total++ & 1;
  heading(s, g, x, "RELICS", found, total);
  float y = g.start, step = std::min(g.row * 1.6f, (g.footer - g.start) / RELIC_ROWS);
  int i = 0;
  for (auto &up : upgrades) {
    if (!up->rare)
      continue;
    int index = i++;
    if (index < page * RELIC_ROWS || index >= (page + 1) * RELIC_ROWS)
      continue;
    bool known = c.relics >> index & 1;
    s.color(col::white.alpha(known ? 1 : 0.15f));
    s.sprite(up->icon, x, y - 2 * g.small, 0, 0, g.small, g.small);
    s.color(col::white);
    s.text(Screen::fit(known ? up->name : "? ? ?", g.colW - 28 * g.small, g.small),
           x + 20 * g.small, y, g.small, known ? col::gold : col::muted * 0.6f);
    y += step;
  }
}

template <class Name>
void tally(Screen &s, const Grid &g, float x, const char *title, const int *counts, int n,
           int first, int last, Name &&name) {
  int seen = 0;
  for (int k = 0; k < n; k++)
    seen += counts[k] > 0;
  heading(s, g, x, title, seen, n);
  float y = g.start;
  for (int k = first; k < std::min(n, last); k++, y += g.row) {
    bool known = counts[k] > 0;
    std::string count = known ? std::to_string(counts[k]) : "";
    float width = Screen::textWidth(count, g.small);
    s.text(Screen::fit(known ? name(k) : std::string("? ? ?"), g.colW - 16 * g.small - width,
                       g.small),
           x, y, g.small, known ? col::white : col::muted * 0.6f);
    if (known)
      s.text(count, x + g.colW - 8 * g.small - width, y, g.small, col::muted);
  }
}
} // namespace

CodexMenu::CodexMenu(Game &game, bool fromPause) : Menu(game), fromPause(fromPause) {
  if (!game.level || !game.level->player)
    catalogue = Upgrade::all();
}

CodexMenu::~CodexMenu() = default;

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
  Grid g;
  g.u = s.u, g.small = std::max(1.0f, g.u - 1), g.pad = 14 * g.u;
  s.rect(0, 0, s.w, s.h, {0.02f, 0.02f, 0.05f, 0.92f});
  s.text("CODEX", g.pad, g.pad, 2 * g.u, col::gold);
  s.text(Screen::fit(fmt::format("{} runs   {} dawn{}   best {}   best streak {}   {} slain",
                                 c.runs, c.dawns, c.dawns == 1 ? "" : "s",
                                 clockText(c.bestTime), c.bestStreak, c.kills()),
                     s.w - 2 * g.pad, g.small),
         g.pad, g.pad + 36 * g.u, g.small, col::muted);
  g.top = g.pad + 62 * g.u, g.colW = (s.w - 2 * g.pad) / 4;
  g.start = g.top + 18 * g.small, g.footer = s.h - g.pad - 24 * g.small;
  g.row = std::min(18 * g.small, (g.footer - g.start) / LIST_ROWS);

  float x = g.pad;
  regions(s, g, c, x, page);
  relics(s, g, c, x += g.colW, page,
         catalogue.empty() ? game.level->player->upgrades : catalogue);
  tally(s, g, x += g.colW, "BESTIARY", c.slain, Codex::KINDS, page * LIST_ROWS,
        (page + 1) * LIST_ROWS, foeTitle);
  int siteRows = (Codex::SITES + PAGE_COUNT - 1) / PAGE_COUNT;
  tally(s, g, x += g.colW, "PLACES", c.entered, Codex::SITES, page * siteRows,
        (page + 1) * siteRows, [](int k) {
          return std::string(STRUCTURES[k].name ? STRUCTURES[k].name : "UNKNOWN SITE");
        });

  s.text("C / Esc back", g.pad, s.h - g.pad - 16 * g.small, g.small, col::muted);
  s.textCenter(fmt::format("PAGE {} / {}", page + 1, PAGE_COUNT), s.w * 0.5f,
               s.h - g.pad - 16 * g.small, g.small, col::white);
  for (int direction : {-1, 1}) {
    SDL_FRect button = pageButton(s, direction);
    bool hover = inside(button, game.input);
    gui::TextButton(button, hover ? col::muted * 0.3f : col::panel,
                    hover ? col::gold : col::muted, g.small,
                    direction < 0 ? "<" : ">", button.y + 7 * g.small,
                    g.small, col::gold).render(s);
  }
}
