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
  auto result = selection.update(in, gui::ChoiceList::layout(game.screen, 4),
                                 4, t > 0.25f);
  if (result.selectionChanged)
    Sound::select.play(0.4f, 0);
  if (result.activated >= 0)
    choose(result.activated);
}

void ShrineMenu::render(Screen &s) {
  float u = s.u, small = std::max(1.0f, u - 1);
  auto r = gui::ChoiceList::layout(s, 4);

  s.textCenter("THE SHRINE OFFERS A BARGAIN", s.w * 0.5f, r[0].y - 28 * u,
               2 * u, col::gold.alpha(0.88f + 0.12f * std::sin(t * 5.0f)));

  for (int i = 0; i < 4; i++) {
    float in = gui::ChoiceList::entrance(t, i);
    float x = r[i].x + (1 - in) * 32 * u, y = r[i].y, w = r[i].w, h = r[i].h;
    bool sel = i == selection.selected;
    const auto &opt = OPTIONS[i];
    Color themeCol = opt.color;

    gui::ChoiceCard({x, y, w, h}, themeCol, u, in, sel).render(s);

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
    gui::KeyBadge({kx, ky, kw, kh}, opt.keyBadge, themeCol, u, small, in, sel).render(s);

    float tagRight = kx - 8 * u;
    s.text(opt.cost, tagRight - Screen::textWidth(opt.cost, small),
           y + 5.0f * u, small, opt.costColor.alpha(in));
  }

  const SDL_FRect &last = r[3];
  s.textCenter("1-3 choose  /  Arrows + Enter  /  Esc keep the flame",
               s.w * 0.5f, last.y + last.h + 9 * u, small, col::muted * 0.75f);
}
