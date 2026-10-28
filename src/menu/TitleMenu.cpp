#include "menu/TitleMenu.hpp"

#include <cctype>

#include "Game.hpp"
#include "Version.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "menu/ClassMenu.hpp"
#include "menu/CodexMenu.hpp"
#include "item/Upgrade.hpp"
#include "menu/Menu.hpp"

namespace {
const char *const MODES[4] = {"CLASSIC", "RANDOM", "DAILY", "CUSTOM"};
const char *const BLURBS[4] = {"the same land every night",
                               "a new land each run",
                               "one land for everyone today",
                               "type a code to share"};

bool inside(const SDL_FRect &r, const Input &in) {
  SDL_FPoint m{in.mouseX, in.mouseY};
  return SDL_PointInRectFloat(&m, &r);
}
} // namespace

TitleMenu::TitleMenu(Game &game) : Menu(game) {
  if (typing())
    SDL_StartTextInput(game.window);
}

TitleMenu::~TitleMenu() { SDL_StopTextInput(game.window); }

bool TitleMenu::typing() const {
  return game.worldMode == WorldMode::Custom;
}

TitleMenu::Layout TitleMenu::layout() const {
  const Screen &s = game.screen;
  float u = s.u, small = std::max(1.0f, u - 1), pad = 8 * u;
  float w = std::max(200 * u, Screen::textWidth("CLASSIC", small) * 4 + 52 * u);
  Layout l{};
  float x = s.w - w - 16 * u, y = s.h * 0.12f + 100 * u, in = x + pad,
        iw = w - 2 * pad;
  float chip = (iw - 3 * 3 * u) / 4;
  for (int i = 0; i < 4; i++)
    l.chips[i] = {in + i * (chip + 3 * u), y + pad + 14 * u, chip, 16 * u};
  l.seed = {in, y + pad + 56 * u, iw, 18 * u};
  float ry = l.seed.y + l.seed.h + 30 * u;
  int n = std::min(int(game.history.size()), 6);
  for (int i = 0; i < 6; i++)
    l.rows[i] = {in, ry + i * 13 * u, iw, 12 * u};
  float bottom = n ? ry + n * 13 * u : l.seed.y + l.seed.h;
  l.panel = {x, y, w, bottom + pad - y};
  return l;
}

void TitleMenu::choose(int mode) {
  if (int(game.worldMode) == mode)
    return;
  Sound::select.play(0.5f, 0);
  bool wasTyping = typing();
  game.worldMode = WorldMode(mode);
  if (typing() && !wasTyping)
    SDL_StartTextInput(game.window);
  else if (!typing() && wasTyping)
    SDL_StopTextInput(game.window);
  if (game.worldMode != WorldMode::Classic)
    game.seedCode.clear();
  pick = -1;
  game.reseed();
}

void TitleMenu::recall(int i) {
  if (i < 0 || i >= int(game.history.size()))
    return;
  pick = i;
  if (!typing())
    SDL_StartTextInput(game.window);
  const std::string &code = game.history[i].code;
  game.worldMode = code == "CLASSIC" ? WorldMode::Classic : WorldMode::Custom;
  game.seedCode = code == "CLASSIC" ? "" : code;
  if (!typing())
    SDL_StopTextInput(game.window);
  Sound::select.play(0.5f, 0);
  game.reseed();
}

void TitleMenu::tick() {
  Menu::tick();
  game.level->player->animT += DT;
  Input &in = game.input;
  if (in.confirm.clicked) {
    Sound::confirm.play(0.6f, 0);
    return game.setMenu(std::make_unique<ClassMenu>(game));
  }
  if (in.escape.clicked) {
    if (typing())
      return choose(int(WorldMode::Classic));
    game.running = false;
    return;
  }
  if (in.map.clicked)
    return choose((int(game.worldMode) + 1) % 4);
  if (!typing() && in.codex.clicked)
    return game.setMenu(std::make_unique<CodexMenu>(game, false));
  int n = int(game.history.size());
  if (n && in.older.clicked)
    return recall(pick < 0 ? 0 : std::max(0, pick - 1));
  if (n && in.newer.clicked)
    return recall(pick < 0 ? 0 : std::min(n - 1, pick + 1));

  Layout l = layout();
  if (in.mouseClicked) {
    for (int i = 0; i < 4; i++)
      if (inside(l.chips[i], in))
        return choose(i);
    for (int i = 0; i < std::min(n, 6); i++)
      if (inside(l.rows[i], in))
        return recall(i);
    if (game.worldMode == WorldMode::Random && inside(l.seed, in))
      return game.seedCode.clear(), game.reseed();
  }
  if (game.worldMode == WorldMode::Random && in.restart.clicked)
    return game.seedCode.clear(), game.reseed();
  if (!typing())
    return;
  std::string code = game.seedCode;
  for (char c : in.typed)
    if (std::isalnum(uint8_t(c)) && int(code.size()) < MAX_CODE)
      code += char(std::toupper(uint8_t(c)));
  if (in.erase.clicked && !code.empty())
    code.pop_back();
  if (code != game.seedCode)
    game.seedCode = code, pick = -1, game.reseed();
}

void TitleMenu::render(Screen &s) {
  float u = s.u, small = std::max(1.0f, u - 1), y = s.h * 0.12f;
  s.rect(0, 0, s.w, s.h, {0.02f, 0.01f, 0.05f, 0.35f});
  s.textCenter("MELLANGAN", s.w / 2, y, 4 * u,
               col::gold.alpha(0.9f + 0.1f * std::sin(t * 2)));
  s.textCenter("kindle the shrines, outlast the night", s.w / 2, y + 70 * u, u,
               col::muted);
  if (std::fmod(t, 1.0f) < 0.65f)
    s.textCenter("PRESS ENTER", s.w / 2, s.h * 0.62f, u, col::white);
  s.textCenter("WASD/Arrows move   Tab map   C codex   Esc pause   M mute   F11 fullscreen",
               s.w / 2, s.h - 30 * small, small, col::muted);
  std::string_view version = "v" MELLANGAN_VERSION;
  s.text(version, s.w - Screen::textWidth(version, small) - 6 * small,
         s.h - 22 * small, small, col::muted * 0.7f);

  Layout l = layout();
  const SDL_FRect &p = l.panel;
  int mode = int(game.worldMode);
  float pad = 8 * u;
  s.rect(p.x, p.y, p.w, p.h, {0.04f, 0.04f, 0.07f, 0.8f});
  s.bracket(p.x, p.y, p.w, p.h, u);
  s.text("WORLD", p.x + pad, p.y + pad, small, col::muted);
  s.text("Tab", p.x + p.w - pad - Screen::textWidth("Tab", small), p.y + pad,
         small, col::muted * 0.7f);
  for (int i = 0; i < 4; i++) {
    const SDL_FRect &c = l.chips[i];
    bool on = i == mode;
    s.rect(c.x, c.y, c.w, c.h, on ? col::gold.alpha(0.9f) : col::panel);
    if (!on)
      s.frame(c.x, c.y, c.w, c.h, u, col::muted.alpha(0.3f));
    s.textCenter(MODES[i], c.x + c.w / 2, c.y + (c.h - 16 * small) / 2, small,
                 on ? col::black : col::muted);
  }
  s.text(BLURBS[mode], p.x + pad, l.chips[0].y + l.chips[0].h + 6 * u, small,
         col::muted);

  const SDL_FRect &f = l.seed;
  bool custom = mode == int(WorldMode::Custom);
  s.rect(f.x, f.y, f.w, f.h, {0.02f, 0.02f, 0.04f, 0.9f});
  s.frame(f.x, f.y, f.w, f.h, u,
          (custom ? col::gold : col::muted).alpha(custom ? 0.6f : 0.25f));
  std::string code = game.worldName();
  if (custom && game.seedCode.empty())
    code.clear();
  Color ink = mode == int(WorldMode::Classic) ? col::muted : col::gold;
  float tx = f.x + 6 * u, ty = f.y + (f.h - 16 * small) / 2;
  s.text(code, tx, ty, small, ink);
  if (custom && std::fmod(t, 1.0f) < 0.55f)
    s.rect(tx + Screen::textWidth(code, small) + u, ty + 2 * u, small * 6,
           12 * small, col::gold);
  if (custom && game.seedCode.empty())
    s.text("type a code", tx + 10 * small, ty, small, col::muted * 0.6f);
  std::string_view hint = mode == int(WorldMode::Random)   ? "R reroll"
                          : custom                           ? "Esc back"
                                                             : "";
  s.text(hint, f.x + f.w - Screen::textWidth(hint, small) - 6 * u, ty, small,
         col::muted * 0.7f);

  int n = std::min(int(game.history.size()), 6);
  if (!n)
    return;
  float hy = l.rows[0].y - 16 * u;
  s.text("PAST WORLDS", p.x + pad, hy, small, col::muted);
  s.text("Up/Down", p.x + p.w - pad - Screen::textWidth("Up/Down", small), hy,
         small, col::muted * 0.7f);
  for (int i = 0; i < n; i++) {
    const PastWorld &w = game.history[i];
    const SDL_FRect &r = l.rows[i];
    bool on = w.code == game.worldName();
    if (on)
      s.rect(r.x - 2 * u, r.y - u, r.w + 4 * u, r.h, col::gold.alpha(0.12f));
    s.text(w.code, r.x, r.y, small, on ? col::gold : col::white * 0.85f);
    std::string stat = fmt::format("{}{}  x{}", w.dawn ? "* " : "",
                                   clockText(w.best), w.runs);
    s.text(stat, r.x + r.w - Screen::textWidth(stat, small), r.y, small,
           w.dawn ? col::gold : col::muted);
  }
}
