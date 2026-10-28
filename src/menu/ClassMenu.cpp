#include "menu/ClassMenu.hpp"

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "item/Upgrade.hpp"
#include "menu/TitleMenu.hpp"

namespace {

struct Stat {
  const char *name;
  float value;
};

void stats(const Hero &h, Stat *out) {
  out[0] = {"HEALTH", 0.5f + h.hp / 80 + h.armor * 0.1f};
  out[1] = {"SPEED", 0.5f + (h.speed - 1) * 4};
  out[2] = {"MIGHT", 0.5f + h.might * 1.6f + h.luck * 3};
  out[3] = {"HASTE", 0.5f - h.haste * 4};
  out[4] = {"REACH", 0.5f + h.magnet / 28};
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
  float u = s.u, w = std::min(s.w * 0.62f, 520 * u), step = w / HERO_COUNT;
  float x = s.w * 0.62f - w / 2 + step * i;
  return {x, s.h * 0.44f, step, s.h * 0.34f};
}

void ClassMenu::choose(int i) {
  i = (i % HERO_COUNT + HERO_COUNT) % HERO_COUNT;
  if (i == pick)
    return;
  pick = i;
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
  s.rect(0, 0, s.w, s.h, {0.01f, 0.01f, 0.03f, 0.7f * in});
  s.textCenter("CHOOSE YOUR CLASS", s.w * 0.62f, s.h * 0.1f, small,
               col::muted.alpha(in));

  const Codex &c = game.codex;
  for (int i = 0; i < HERO_COUNT; i++) {
    const Hero &h = *HEROES[i];
    SDL_FRect r = slot(i);
    bool open = h.met(c), on = i == pick;
    float g = grow[i], cx = r.x + r.w / 2, foot = r.y + r.h * 0.72f;
    float scale = (3 + 2.5f * g) * u * 0.5f + u;
    Color ring = (open ? col::gold : col::muted).alpha((0.15f + 0.55f * g) * in);
    s.color(ring);
    s.center(atlas::Ring, cx, foot, (12 + 10 * g) * u / atlas::Ring.w * 2);
    s.color(col::white);
    float bob = on ? std::sin(t * 2.4f) * u : 0;
    Sprite body = atlas::tile(h.tile);
    float xo = body.w * 0.5f, yo = float(body.h);
    s.color(open ? col::white.alpha((0.55f + 0.45f * g) * in)
                 : Color{0.06f, 0.05f, 0.1f, in});
    s.sprite(body, cx, foot + bob, xo, yo, scale, scale);
    s.color(col::white);
    Color label = on ? col::gold : open ? col::white * 0.75f : col::muted * 0.6f;
    s.textCenter(open || on ? h.name : "???", cx, foot + 6 * u, small,
                 label.alpha(in));
  }

  const Hero &h = *HEROES[pick];
  bool open = h.met(c);
  float x = s.w * 0.06f, y = s.h * 0.2f, w = std::min(s.w * 0.24f, 240 * u);
  s.text(h.name, x, y, 3 * u, (open ? col::gold : col::muted).alpha(in));
  y += 52 * u;
  s.text(h.title, x, y, small, col::muted.alpha(in));
  y += 20 * u;
  s.rect(x, y, w, u, col::gold.alpha(0.4f * in));
  y += 10 * u;
  if (!open) {
    s.text("LOCKED", x, y, u, col::red.alpha(in));
    s.text(fmt::format("to unlock: {}", h.need), x, y + 22 * u, small,
           col::muted.alpha(in));
  } else {
    s.text("SIGNATURE WEAPON", x, y, small, col::muted.alpha(in));
    y += 18 * u;
    for (auto &up : game.level->player->upgrades)
      if (std::string_view(up->name) == h.weapon)
        s.sprite(up->icon, x, y, 0, 0, u, u);
    s.text(h.weapon, x + 22 * u, y + 2 * u, small, col::white.alpha(in));
    y += 26 * u;
    s.text("TRAIT", x, y, small, col::muted.alpha(in));
    y += 18 * u;
    s.text(h.trait, x, y, small, col::white.alpha(in));
    y += 30 * u;
    Stat st[5];
    stats(h, st);
    for (const Stat &k : st) {
      s.text(k.name, x, y, small, col::muted.alpha(in));
      float bx = x + 70 * small, bw = w - 70 * small, v = clampf(k.value, 0.08f, 1);
      s.rect(bx, y + 5 * small, bw, 5 * small, col::panel.alpha(in));
      s.rect(bx, y + 5 * small, bw * v, 5 * small,
             (v > 0.55f ? col::gold : v < 0.45f ? col::red * 0.8f : col::xp)
                 .alpha(in));
      y += 18 * small;
    }
  }

  s.textCenter("Left/Right choose   Enter begin   Esc back", s.w / 2,
               s.h - 30 * small, small, col::muted.alpha(in));
}
