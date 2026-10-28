#include "menu/ClassMenu.hpp"

#include <string>
#include <vector>

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "item/Upgrade.hpp"
#include "menu/TitleMenu.hpp"

namespace {

struct Chip {
  std::string text;
  bool good;
};

std::vector<Chip> chips(const Hero &h) {
  std::vector<Chip> out;
  auto pct = [&](float v, const char *name, bool lowGood) {
    int n = int(std::lround(v * 100));
    if (n)
      out.push_back({fmt::format("{} {:+}%", name, n), lowGood ? n < 0 : n > 0});
  };
  if (h.hp)
    out.push_back({fmt::format("{:+} HP", int(h.hp)), h.hp > 0});
  if (h.armor)
    out.push_back({fmt::format("ARMOUR +{}", int(h.armor)), true});
  pct(h.speed - 1, "SPEED", false);
  pct(h.might, "DAMAGE", false);
  pct(h.haste, "COOLDOWN", true);
  pct(h.luck, "CRIT", false);
  if (h.regen)
    out.push_back({fmt::format("REGEN {:.1f}/S", h.regen), true});
  if (h.magnet)
    out.push_back({fmt::format("REACH +{}", int(h.magnet)), true});
  return out;
}

float ease(float x) {
  x = clampf(x, 0, 1);
  return x * x * (3 - 2 * x);
}

float fract(float x) { return x - std::floor(x); }

const char *const SIGILS[12][13] = {
    {"......#......", "......#...*..", ".....#+#.***.", ".....#+#..*..", "....#+++#....", "..##+++++##..", "##+++++++++##", "..##+++++##..", "....#+++#....", ".....#+#.....", ".....#+#.....", "......#......", "......#......"},
    {".............", ".............", "........#....", "........##...", "#..#....#+#..", "##.##...#++#.", "+#++#####+++#", "##.##...#++#.", "#..#....#+#..", "........##...", "........#....", ".............", "............."},
    {"#############", "#-----+-----#", "#-----+-----#", "#-----+-----#", "#++++++++++*#", "#-----+-----#", "#-----+-----#", ".#----+----#.", ".#----+----#.", "..#---+---#..", "...#--+--#...", "....#-+-#....", "......#......"},
    {"....#####....", ".....#.#.....", ".....#.#.....", ".....#*#.....", "....#...#....", "...#.....#...", "..#.......#..", ".#+++++++++#.", "#+++-+++*+++#", "#+++++++-+++#", "#+*+++++++++#", ".#+++++++++#.", "..#########.."},
    {".......####..", "......#++#...", ".....#++#....", "....#++#.....", "...#++######.", "..#+++++++#..", ".######++#...", "......#+#....", ".....#+#.....", "....#*#......", "...##........", "..#..........", "............."},
    {".....######..", "...##++++++#.", "..#++####++#.", ".#+##....#+#.", "#+#......#+#.", "#*.......#+#.", "#........#+#.", ".........#+#.", ".........#+#.", ".........#+#.", ".........#+#.", "........##+#.", ".........###."},
    {"......*......", "..*...#...*..", "...#.....#...", ".....###.....", "....#+++#....", "...#+++++#...", "*#.#++-++#.#*", "...#+++++#...", "....#+++#....", ".....###.....", "...#.....#...", "..*...#...*..", "......*......"},
    {"#...........#", "+#.........#+", ".+#.......#+.", "..+#.....#+..", "...+#...#+...", "....+#.#+....", "......*......", "....+#.#+....", "...+#...#+...", "..##.....##..", ".####...####.", "..#.......#..", ".#.........#."},
    {"......*......", ".....#+#.....", "....#+++#....", "...#+++++#...", "..#+++++++#..", ".#+++++++++#.", "..#+++++++#..", "...#++-++#...", "....#---#....", ".....#-#.....", "....#-#-#....", "...#..#..#...", "..*...*...*.."},
    {"....##.##....", "...#++#++#...", "..#+#...#+#..", "..##..*..##..", ".#+...#...+#.", "##...#+#...##", ".#..#+++#..#.", "##...#+#...##", ".#+...#...+#.", "..##..*..##..", "..#+#...#+#..", "...#++#++#...", "....##.##...."},
    {"......*......", "....*###.....", "...#++++#....", "..#++++..*...", ".#++++...*...", ".#+++...#+#..", ".#+++..#+++#.", ".#+++...#+#..", ".#++++...*...", "..#++++..*...", "...#++++#....", "....*###.....", "......*......"},
    {"......#......", "....#####....", "...#..#..#...", "......#......", ".#....#....#.", "..#...#...#..", "...#..#..#...", "....#.#.#....", ".##..###..##.", "####..#..####", ".##...#...##.", "......#......", ".....###....."}
};

void sigil(Screen &s, int hero, float x, float y, float r, Color c, float t) {
  const char *const *art = SIGILS[hero % 12];
  float cell = std::max(1.0f, std::floor(r * 2 / 13));
  float x0 = std::floor(x - cell * 6.5f), y0 = std::floor(y - cell * 6.5f);
  Color light{lerpf(c.r, 1, 0.55f), lerpf(c.g, 1, 0.55f), lerpf(c.b, 1, 0.55f), c.a};
  Color dim = c * 0.55f, shadow{0.01f, 0.01f, 0.03f, 0.7f * c.a};
  for (int pass = 0; pass < 2; pass++)
    for (int row = 0; row < 13; row++)
      for (int col = 0; col < 13; col++) {
        char k = art[row][col];
        if (k == '.')
          continue;
        float px = x0 + col * cell, py = y0 + row * cell;
        if (pass == 0) {
          if (cell > 1)
            s.rect(px + cell, py + cell, cell, cell, shadow);
          continue;
        }
        Color ink = k == '+' ? light : k == '-' ? dim : c;
        if (k == '*')
          ink = light.alpha(c.a * (0.35f + 0.65f * std::abs(std::sin(
                                                    t * 2.2f + row * 0.7f + col))));
        s.rect(px, py, cell, cell, ink);
      }
  if (hero == 3)
    for (int i = 0; i < 3; i++) {
      float b = fract(t * 0.6f + i * 0.33f);
      s.rect(x0 + (5 + i) * cell, y0 + (2 - b * 4) * cell, cell, cell,
             light.alpha(c.a * (1 - b)));
    }
}

float wrap(Screen &s, std::string_view text, float x, float y, float w,
           float size, float line, Color c, bool draw) {
  std::string row;
  size_t at = 0;
  while (at <= text.size()) {
    size_t end = text.find(' ', at);
    std::string_view word = text.substr(at, end == std::string_view::npos
                                                ? std::string_view::npos
                                                : end - at);
    std::string next = row.empty() ? std::string(word)
                                   : row + " " + std::string(word);
    if (!row.empty() && Screen::textWidth(next, size) > w) {
      if (draw)
        s.text(row, x, y, size, c);
      y += line, row = std::string(word);
    } else {
      row = next;
    }
    if (end == std::string_view::npos)
      break;
    at = end + 1;
  }
  if (!row.empty()) {
    if (draw)
      s.text(row, x, y, size, c);
    y += line;
  }
  return y;
}

} // namespace

ClassMenu::ClassMenu(Game &game) : Menu(game) {
  pick = game.codex.start;
  if (pick < 0 || pick >= HERO_COUNT || !HEROES[pick]->met(game.codex))
    pick = 0;
  grow[pick] = 1;
}

SDL_FRect ClassMenu::slot(int i) const {
  const Screen &s = game.screen;
  float u = s.u, w = std::min(s.w * 0.54f, 640 * u), step = w / 6;
  float x = s.w * 0.69f - w / 2 + step * (i % 6);
  return {x, s.h * (i < 6 ? 0.25f : 0.53f), step, s.h * 0.25f};
}

void ClassMenu::choose(int i) {
  i = (i % HERO_COUNT + HERO_COUNT) % HERO_COUNT;
  if (i == pick)
    return;
  pick = i, since = 0;
  Sound::select.play(0.5f, 0);
  if (HEROES[pick]->met(game.codex)) {
    game.codex.start = pick;
    game.level->player->tile = HEROES[pick]->tile;
  }
}

void ClassMenu::begin() {
  if (!HEROES[pick]->met(game.codex))
    return Sound::hurt.play(0.4f, 0);
  game.codex.start = pick;
  Sound::confirm.play(0.6f, 0);
  game.newGame(false);
}

void ClassMenu::tick() {
  Menu::tick();
  since += DT;
  game.level->player->animT += DT;
  for (int i = 0; i < HERO_COUNT; i++)
    grow[i] += ((i == pick ? 1.0f : 0.0f) - grow[i]) * damp(12, DT);
  Input &in = game.input;
  if (in.escape.clicked) {
    Sound::select.play(0.5f, 0);
    return game.setMenu(std::make_unique<TitleMenu>(game));
  }
  if (in.left.clicked)
    return choose(pick - 1);
  if (in.right.clicked)
    return choose(pick + 1);
  if (in.confirm.clicked && t > 0.2f)
    return begin();
  SDL_FPoint m{in.mouseX, in.mouseY};
  for (int i = 0; i < HERO_COUNT; i++) {
    SDL_FRect r = slot(i);
    if (!in.mouseClicked || !SDL_PointInRectFloat(&m, &r))
      continue;
    if (i == pick)
      return begin();
    return choose(i);
  }
}

void ClassMenu::render(Screen &s) {
  float u = s.u, small = std::max(1.0f, u - 1), in = std::min(1.0f, t * 3);
  s.rect(0, 0, s.w, s.h, {0.015f, 0.012f, 0.03f, 0.9f * in});
  float edge = std::min(s.w, s.h) * 0.025f;
  for (int i = 0; i < 10; i++)
    s.frame(i * edge, i * edge, s.w - 2 * i * edge, s.h - 2 * i * edge, edge,
            {0, 0, 0, 0.42f * (1 - i / 10.0f) * in});
  s.textCenter("CHOOSE YOUR CLASS", s.w * 0.69f, s.h * 0.1f, small,
               col::muted.alpha(in));

  const Codex &c = game.codex;
  const Hero &h = *HEROES[pick];
  bool openPick = h.met(c);
  float cycle = fract(since / 2.2f);
  float wind = ease(cycle / 0.5f), strike = ease((cycle - 0.5f) / 0.12f),
        recover = ease((cycle - 0.62f) / 0.38f);
  float lunge = strike * (1 - recover);

  for (int i = 0; i < HERO_COUNT; i++) {
    const Hero &k = *HEROES[i];
    SDL_FRect r = slot(i);
    bool open = k.met(c), on = i == pick;
    float g = grow[i], cx = r.x + r.w / 2, foot = r.y + r.h * 0.72f;
    float scale = (3 + 2.5f * g) * u * 0.5f + u;
    Sprite body = k.sprite();

    if (on && open) {
      float pulse = 0.75f + 0.25f * std::sin(t * 3);
      s.color(k.tint.alpha(0.22f * pulse * in));
      s.center(atlas::Glow, cx, foot - body.h * scale * 0.5f,
               body.h * scale * 2.6f / atlas::Glow.w);
      s.color(k.tint.alpha((0.25f + 0.5f * wind * (1 - recover)) * in));
      s.center(atlas::Ring, cx, foot,
               (16 + 14 * wind) * u * 2 / atlas::Ring.w, t * 2.5f);
      s.center(atlas::Ring, cx, foot,
               (10 + 8 * wind) * u * 2 / atlas::Ring.w, -t * 3.5f);
      for (int m = 0; m < 16; m++) {
        float p = fract(t * 0.35f + m * 0.137f);
        float mx = cx + std::sin(m * 2.3f + t * 1.3f) * (14 + 6 * (m % 3)) * u;
        float my = foot - p * body.h * scale * 1.3f;
        float size = (m % 2 ? 1.5f : 1) * u;
        s.rect(mx, my, size, size, k.tint.alpha((1 - p) * 0.8f * in));
      }
    } else {
      s.color((open ? col::gold : col::muted).alpha((0.12f + 0.3f * g) * in));
      s.center(atlas::Ring, cx, foot, 12 * u * 2 / atlas::Ring.w);
    }
    s.color(col::white);

    float bob = std::sin(t * 2 + i) * u * (on ? 1 : 0.4f);
    float lean = on && open ? -0.12f * wind * (1 - strike) + 0.1f * lunge : 0;
    float push = on && open ? (lunge * 8 - wind * (1 - strike) * 2) * u : 0;
    float swell = 1 + (on && open ? 0.07f * lunge : 0);
    float xo = body.w * 0.5f, yo = float(body.h);
    s.color(open ? col::white.alpha((0.5f + 0.5f * g) * in)
                 : Color{0.05f, 0.04f, 0.09f, in});
    s.sprite(body, cx + push, foot + bob, xo, yo, scale * swell, scale * swell,
             lean);
    s.color(col::white);

    if (on && open) {
      for (auto &up : game.level->player->upgrades)
        if (std::string_view(up->name) == k.weapon) {
          float swing = -1.1f * wind * (1 - strike) + 1.5f * lunge;
          float hx = cx + push + body.w * scale * 0.42f,
                hy = foot + bob - body.h * scale * 0.5f;
          s.sprite(up->icon, hx, hy, up->icon.w * 0.5f, float(up->icon.h),
                   scale * 0.85f, scale * 0.85f, swing);
          if (cycle > 0.5f && cycle < 0.85f) {
            float burst = ease((cycle - 0.5f) / 0.35f);
            for (int m = 0; m < 12; m++) {
              float a = m * TAU / 12 + t;
              float d = burst * 34 * u;
              s.rect(hx + std::cos(a) * d, hy - 6 * u + std::sin(a) * d,
                     1.5f * u, 1.5f * u, k.tint.alpha((1 - burst) * in));
            }
            s.color(k.tint.alpha(0.5f * (1 - burst) * in));
            s.center(atlas::Glow, hx, hy - 6 * u,
                     (10 + 30 * burst) * u / atlas::Glow.w);
            s.color(col::white);
          }
        }
    }
    Color label = on ? (open ? k.tint : col::muted)
                     : open ? col::white * 0.75f
                            : col::muted * 0.6f;
    s.textCenter(open || on ? k.name : "???", cx, foot + 7 * u, small,
                 label.alpha(in));
  }

  float x = s.w * 0.045f, top = s.h * 0.07f, w = std::min(s.w * 0.34f, 340 * u);
  float pad = 12 * u, line = 13 * u, body = u, gut = 22 * u;
  Color tint = openPick ? h.tint : col::muted;
  const std::vector<Chip> traits = chips(h);
  auto card = [&](bool draw) {
    float y = top;
    float big = std::min(2 * u, std::floor((w - 46 * u) / Screen::textWidth(h.name, 1)));
    float band = 8 * big + 12 * u;
    if (draw) {
      s.rect(x, y, w, band, tint.alpha(0.12f * in));
      for (int k = 0; k < 6; k++)
        sigil(s, pick, x + w - 18 * u - k * 34 * u, y + band / 2, 11 * u,
              tint.alpha(0.07f * in), t);
      sigil(s, pick, x + 21 * u, y + band / 2, 14 * u,
            (openPick ? tint : col::muted * 0.6f).alpha(in), t);
      s.text(h.name, x + 42 * u, y + band / 2 - 8 * big, big, tint.alpha(in));
      s.rect(x, y + band, w, u, tint.alpha(0.7f * in));
    }
    y += band + 8 * u;
    float cx = x + pad, cw = w - 2 * pad;
    y = wrap(s, h.title, cx, y, cw, body, line, tint.alpha(0.75f * in), draw) +
        6 * u;
    if (!openPick) {
      if (draw)
        s.text("LOCKED", cx, y, body, col::red.alpha(in));
      y += line + 4 * u;
      y = wrap(s, fmt::format("Unlock: {}", h.need), cx, y, cw, body, line,
               col::white.alpha(0.85f * in), draw);
      return y + pad - top;
    }
    auto label = [&](const char *text) {
      if (draw) {
        s.text(text, cx, y, small, tint.alpha(0.85f * in));
        s.rect(cx + Screen::textWidth(text, small) + 6 * u, y + 8 * small,
               cw - Screen::textWidth(text, small) - 6 * u, 1,
               tint.alpha(0.25f * in));
      }
      y += 12 * u;
    };
    auto icon = [&](std::string_view name, float iy) {
      for (auto &up : game.level->player->upgrades)
        if (std::string_view(up->name) == name && draw)
          s.sprite(up->icon, cx, iy - u, 0, 0, u, u);
    };

    label("WEAPON");
    icon(h.weapon, y);
    if (draw)
      s.text(h.weapon, cx + gut, y, body, col::white.alpha(in));
    y += line + 6 * u;
    label("TRAIT");
    if (draw)
      sigil(s, pick, cx + 8 * u, y + 7 * u, 7 * u, tint.alpha(in), t);
    y = wrap(s, h.trait, cx + gut, y, cw - gut, body, line,
             col::white.alpha(0.85f * in), draw) + 6 * u;

    label("SKILL");
    float key = Screen::textWidth("SPACE", small) + 8 * u;
    if (draw) {
      s.rect(cx + cw - key, y - u, key, 14 * u, tint.alpha(0.15f * in));
      s.frame(cx + cw - key, y - u, key, 14 * u, 1, tint.alpha(0.7f * in));
      s.text("SPACE", cx + cw - key + 4 * u, y + u, small, tint.alpha(in));
      s.text(h.skill, cx, y, body, tint.alpha(in));
    }
    y += line + 2 * u;
    y = wrap(s, h.skillDesc, cx, y, cw, body, line,
             col::white.alpha(0.85f * in), draw) + 6 * u;

    label("ARSENAL");
    for (auto &up : game.level->player->upgrades)
      if (up->owner && std::string_view(up->owner) == h.name) {
        if (draw) {
          s.sprite(up->icon, cx, y - u, 0, 0, u, u);
          s.text(up->name, cx + gut, y, body,
                 (up->weapon ? col::white : tint).alpha(in));
          if (!up->weapon)
            s.text("talent", cx + cw - Screen::textWidth("talent", small),
                   y + 3 * u, small, col::muted.alpha(in));
        }
        y += line;
      }
    y += 6 * u;

    label("NATURE");
    float chipX = cx;
    for (const Chip &k : traits) {
      float kw = Screen::textWidth(k.text, small) + 10 * u;
      if (chipX + kw > cx + cw)
        chipX = cx, y += 15 * u;
      Color ink = k.good ? tint : col::red * 0.9f;
      if (draw) {
        s.rect(chipX, y, kw, 12 * u, ink.alpha(0.18f * in));
        s.rect(chipX, y, 2 * u, 12 * u, ink.alpha(in));
        s.text(k.text, chipX + 6 * u, y + 2 * u, small, ink.alpha(in));
      }
      chipX += kw + 5 * u;
    }
    y += 15 * u;
    float fx = cx;
    if (draw)
      s.text("favours", fx, y + 3 * u, small, col::muted.alpha(in));
    fx += Screen::textWidth("favours ", small) + 4 * u;
    for (int k = 0; k < 3 && draw; k++)
      for (auto &up : game.level->player->upgrades)
        if (std::string_view(up->name) == h.affinity[k])
          s.sprite(up->icon, fx + k * 20 * u, y, 0, 0, u, u);
    y += 18 * u;
    return y + pad - top;
  };

  float tall = card(false);
  s.rect(x, top, w, tall, {0.03f, 0.025f, 0.06f, 0.95f * in});
  s.frame(x, top, w, tall, u, tint.alpha(0.35f * in));
  for (int k = 2; k < 4; k++)
    sigil(s, pick, k & 1 ? x + w - 9 * u : x + 9 * u,
          k & 2 ? top + tall - 9 * u : top + 9 * u, 7 * u,
          tint.alpha(0.8f * in), t);
  card(true);

  s.textCenter("Left/Right choose   Enter begin   Esc back", s.w * 0.69f,
               s.h - 22 * u, u, col::muted.alpha(in));
}
