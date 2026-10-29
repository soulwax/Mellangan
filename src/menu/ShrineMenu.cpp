#include "menu/ShrineMenu.hpp"

#include <algorithm>
#include <cmath>

#include "Atlas.hpp"
#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"

namespace {
struct ShrineOption {
  const char *title;
  const char *desc;
  const char *cost;
  Color color;
  Color costColor;
  int spriteIndex;
  const char *keyBadge;
};

const ShrineOption OPTIONS[4] = {{"FURY",
                                  "+20% damage",
                                  "-10 MAX HP",
                                  {0.96f, 0.32f, 0.24f, 1.0f},
                                  {0.96f, 0.45f, 0.45f, 1.0f},
                                  atlas::ex::ShrineFury,
                                  "1"},
                                 {"ROOTS",
                                  "+30 maximum health",
                                  "-6 SPEED",
                                  {0.28f, 0.88f, 0.44f, 1.0f},
                                  {0.96f, 0.70f, 0.45f, 1.0f},
                                  atlas::ex::ShrineRoots,
                                  "2"},
                                 {"VISION",
                                  "Gain a level",
                                  "-1 ARMOUR",
                                  {0.38f, 0.72f, 1.0f, 1.0f},
                                  {0.96f, 0.65f, 0.50f, 1.0f},
                                  atlas::ex::ShrineVision,
                                  "3"},
                                 {"KEEP THE FLAME",
                                  "Leave the shrine undisturbed",
                                  "NO COST",
                                  {1.0f, 0.82f, 0.30f, 1.0f},
                                  {0.75f, 0.80f, 0.88f, 1.0f},
                                  atlas::ex::ShrineFlame,
                                  "Esc"}};
} // namespace

std::array<SDL_FRect, 4> ShrineMenu::cards() const {
  const Screen &s = game.screen;
  float u = s.u, w = std::min(s.w * 0.90f, 440 * u), h = 46 * u, gap = 6 * u;
  float y = s.h * 0.5f - (4 * h + 3 * gap) * 0.5f + 10 * u;
  std::array<SDL_FRect, 4> r{};
  for (int i = 0; i < 4; i++)
    r[i] = {(s.w - w) * 0.5f, y + i * (h + gap), w, h};
  return r;
}

void ShrineMenu::choose(int choice) {
  auto &n = game.level->night;
  if (!n.offering)
    return;
  Player &p = *game.level->player;
  if (choice == 0 && p.maxHp > 20) {
    p.might += 0.2f;
    p.maxHp -= 10;
    p.hp = std::min(p.hp, p.maxHp);
    game.level->handicaps++;
    Sound::confirm.play(0.6f, 0);
  } else if (choice == 1 && p.speed > 25) {
    p.maxHp += 30;
    p.hp = std::min(p.maxHp, p.hp + 30);
    p.speed -= 6;
    game.level->handicaps++;
    Sound::confirm.play(0.6f, 0);
  } else if (choice == 2) {
    p.pendingLevels++;
    p.armor -= 1;
    game.level->handicaps++;
    Sound::confirm.play(0.6f, 0);
  } else if (choice == 3) {
    Sound::select.play(0.5f, 0);
  } else {
    return;
  }
  n.offering = false;
  game.setMenu(nullptr);
}

void ShrineMenu::tick() {
  Menu::tick();
  Input &in = game.input;
  if (in.escape.clicked)
    return choose(3);
  int prev = selected;
  if (in.up.clicked)
    selected = (selected + 3) % 4;
  if (in.down.clicked)
    selected = (selected + 1) % 4;
  Key *keys[] = {&in.one, &in.two, &in.three, &in.four};
  auto r = cards();
  for (int i = 0; i < 4; i++) {
    if (keys[i]->clicked)
      return choose(i);
    SDL_FPoint m{in.mouseX, in.mouseY};
    if (!SDL_PointInRectFloat(&m, &r[i]))
      continue;
    if (in.mouseMoved && selected != i)
      selected = i, Sound::select.play(0.4f, 0);
    if (in.mouseClicked && t > 0.25f)
      return choose(i);
  }
  if (selected != prev)
    Sound::select.play(0.4f, 0);
  if (in.confirm.clicked && t > 0.25f)
    choose(selected);
}

void ShrineMenu::render(Screen &s) {
  float u = s.u, small = std::max(1.0f, u - 1);
  auto r = cards();

  s.textCenter("THE SHRINE OFFERS A BARGAIN", s.w * 0.5f, r[0].y - 28 * u,
               2 * u, col::gold.alpha(0.88f + 0.12f * std::sin(t * 5.0f)));

  for (int i = 0; i < 4; i++) {
    float in = clampf((t - i * 0.05f) / 0.22f, 0, 1);
    in = 1 - (1 - in) * (1 - in) * (1 - in);
    float x = r[i].x + (1 - in) * 32 * u, y = r[i].y, w = r[i].w, h = r[i].h;
    bool sel = i == selected;
    const auto &opt = OPTIONS[i];
    Color themeCol = opt.color;

    s.rect(x, y, w, h, col::panel.alpha(0.92f * in));
    if (sel)
      s.rect(x, y, w, h, themeCol.alpha(0.12f * in));
    s.frame(x, y, w, h, sel ? 2 * u : u,
            themeCol.alpha((sel ? 1.0f : 0.65f) * in));
    if (sel) {
      float tickLen = 7 * u;
      s.line(x, y, x + tickLen, y, u, col::white.alpha(in));
      s.line(x, y, x, y + tickLen, u, col::white.alpha(in));
      s.line(x + w, y, x + w - tickLen, y, u, col::white.alpha(in));
      s.line(x + w, y, x + w, y + tickLen, u, col::white.alpha(in));
      s.line(x, y + h, x + tickLen, y + h, u, col::white.alpha(in));
      s.line(x, y + h, x, y + h - tickLen, u, col::white.alpha(in));
      s.line(x + w, y + h, x + w - tickLen, y + h, u, col::white.alpha(in));
      s.line(x + w, y + h, x + w, y + h - tickLen, u, col::white.alpha(in));
    }

    float badgeSize = 24 * u;
    float bx = x + 6 * u, by = y + (h - badgeSize) * 0.5f;
    s.rect(bx, by, badgeSize, badgeSize,
           {themeCol.r * 0.16f, themeCol.g * 0.16f, themeCol.b * 0.16f,
            0.9f * in});
    s.frame(bx, by, badgeSize, badgeSize, u, themeCol.alpha(0.8f * in));

    float iconX = bx + (badgeSize - 16 * u) * 0.5f;
    float iconY = by + (badgeSize - 16 * u) * 0.5f;
    s.color(col::white.alpha(in));
    s.sprite(atlas::expansion(opt.spriteIndex), iconX, iconY, 0, 0, u, u);
    s.color(col::white);

    float tx = bx + badgeSize + 8 * u;
    s.text(opt.title, tx, y + 5.0f * u, u,
           (sel ? col::white : themeCol).alpha(in));
    s.text(opt.desc, tx, y + 25.0f * u, small,
           (sel ? col::white * 0.9f : col::muted).alpha(in));

    float kw = std::max(18 * u, Screen::textWidth(opt.keyBadge, small) + 10 * u);
    float kh = 18 * u;
    float kx = x + w - kw - 8 * u, ky = y + (h - kh) * 0.5f;
    s.rect(kx, ky, kw, kh, {0.10f, 0.08f, 0.15f, 0.85f * in});
    s.frame(kx, ky, kw, kh, u, (sel ? themeCol : col::muted * 0.45f).alpha(in));
    s.textCenter(opt.keyBadge, kx + kw * 0.5f,
                 ky + (kh - 16 * small) * 0.5f, small,
                 (sel ? col::white : col::muted).alpha(in));

    float tagRight = kx - 8 * u;
    s.text(opt.cost, tagRight - Screen::textWidth(opt.cost, small),
           y + 5.0f * u, small, opt.costColor.alpha(in));
  }

  const SDL_FRect &last = r[3];
  s.textCenter("1-3 choose  /  Arrows + Enter  /  Esc keep the flame",
               s.w * 0.5f, last.y + last.h + 9 * u, small, col::muted * 0.75f);
}
