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

} // namespace

ClassMenu::ClassMenu(Game &game) : Menu(game) {
  pick = game.codex.start;
  if (pick < 0 || pick >= HERO_COUNT || !HEROES[pick]->met(game.codex))
    pick = 0;
  grow[pick] = 1;
}

SDL_FRect ClassMenu::slot(int i) const {
  const Screen &s = game.screen;
  float u = s.u, w = std::min(s.w * 0.54f, 640 * u), step = w / HERO_COUNT;
  float x = s.w * 0.69f - w / 2 + step * i;
  return {x, s.h * 0.44f, step, s.h * 0.34f};
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
    Sprite body = atlas::tile(k.tile);

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

  float x = s.w * 0.05f, y = s.h * 0.14f, w = std::min(s.w * 0.3f, 300 * u);
  std::vector<Chip> traits = chips(h);
  int rows = 1;
  for (float cx = x; const Chip &k : traits) {
    float cw = Screen::textWidth(k.text, small) + 8 * u;
    if (cx + cw > x + w)
      cx = x, rows++;
    cx += cw + 4 * u;
  }
  float pad = 10 * u,
        tall = openPick ? 190 * u + rows * (16 * small + 4 * u) : 135 * u;
  s.rect(x - pad, y - pad, w + 2 * pad, tall, {0.03f, 0.025f, 0.06f, 0.94f * in});
  s.bracket(x - pad, y - pad, w + 2 * pad, tall, u);
  Color tint = openPick ? h.tint : col::muted;
  float big = std::min(3 * u, std::floor(w / Screen::textWidth(h.name, 1)));
  s.text(h.name, x, y, big, tint.alpha(in));
  y += 16 * big + 2 * u;
  s.text(h.title, x, y, small, col::muted.alpha(in));
  y += 18 * u;
  s.rect(x, y, w, u, tint.alpha(0.5f * in));
  y += 9 * u;
  if (!openPick) {
    s.text("LOCKED", x, y, u, col::red.alpha(in));
    s.text(fmt::format("to unlock: {}", h.need), x, y + 22 * u, small,
           col::muted.alpha(in));
  } else {
    float row = 16 * small + 3 * u, line = 18 * u;
    for (auto &up : game.level->player->upgrades)
      if (std::string_view(up->name) == h.weapon)
        s.sprite(up->icon, x, y - 2 * u, 0, 0, u, u);
    s.text(h.weapon, x + 20 * u, y, small, col::white.alpha(in));
    y += line;
    s.text(h.trait, x, y, small, col::muted.alpha(in));
    y += row * 1.6f;
    s.text(h.skill, x, y, small, tint.alpha(in));
    s.text("SPACE", x + w - Screen::textWidth("SPACE", small), y, small,
           col::muted.alpha(in));
    y += row;
    s.text(h.skillDesc, x, y, small, col::white.alpha(0.85f * in));
    y += row * 1.6f;
    float kx = x;
    for (auto &up : game.level->player->upgrades)
      if (up->owner && std::string_view(up->owner) == h.name) {
        s.sprite(up->icon, kx, y - 2 * u, 0, 0, u, u);
        s.text(up->name, kx + 19 * u, y, small, tint.alpha(in));
        kx += 19 * u + Screen::textWidth(up->name, small) + 10 * u;
      }
    y += line;
    s.text(fmt::format("favours {}, {}, {}", h.affinity[0], h.affinity[1],
                       h.affinity[2]),
           x, y, small, col::muted.alpha(0.8f * in));
    y += row * 1.6f;
    float cx = x;
    for (const Chip &k : traits) {
      float cw = Screen::textWidth(k.text, small) + 8 * u;
      if (cx + cw > x + w)
        cx = x, y += 16 * small + 4 * u;
      Color ink = k.good ? tint : col::red * 0.9f;
      s.rect(cx, y, cw, 16 * small, ink.alpha(0.14f * in));
      s.frame(cx, y, cw, 16 * small, 1, ink.alpha(0.5f * in));
      s.text(k.text, cx + 4 * u, y, small, ink.alpha(in));
      cx += cw + 4 * u;
    }
  }

  s.textCenter("Left/Right choose   Enter begin   Esc back", s.w * 0.69f,
               s.h - 30 * small, small, col::muted.alpha(in));
}
