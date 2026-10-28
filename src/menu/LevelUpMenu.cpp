#include "menu/LevelUpMenu.hpp"

#include <spdlog/spdlog.h>

#include <cstring>

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "hero/Hero.hpp"
#include "item/Upgrade.hpp"
#include "menu/Menu.hpp"

LevelUpMenu::LevelUpMenu(Game &game) : Menu(game) {
  Player &p = *game.level->player;
  auto &all = p.upgrades;
  if (p.pendingTreasures > 0)
    p.pendingTreasures--, treasure = true;
  int owned[2] = {0, 0};
  for (auto &u : all)
    if (u->level > 0 && !u->rare)
      owned[u->weapon]++;
  std::vector<Upgrade *> relics;
  for (auto &u : all)
    if (u->rare && !u->maxed())
      relics.push_back(u.get());
    else if (!u->rare && !u->maxed() &&
             (u->level > 0 || owned[u->weapon] < Upgrade::SLOTS) &&
             (!u->owner || (p.hero && std::strcmp(u->owner, p.hero->name) == 0)))
      choices.push_back(u.get());
  std::vector<Upgrade *> pool;
  pool.swap(choices);
  auto weight = [&](const Upgrade *u) {
    return u->owner ? 3 : p.hero && p.hero->favours(u->name) ? 2 : 1;
  };
  while (!pool.empty() && choices.size() < 3) {
    int total = 0;
    for (Upgrade *u : pool)
      total += weight(u);
    int r = rng.irange(0, total - 1);
    size_t k = 0;
    while ((r -= weight(pool[k])) >= 0)
      k++;
    choices.push_back(pool[k]);
    pool.erase(pool.begin() + long(k));
  }
  if (choices.empty())
    choices.push_back(nullptr);
  std::vector<Upgrade *> ready;
  for (auto &u : all)
    if (u->canEvolve(p))
      ready.push_back(u.get());
  if (treasure && !ready.empty())
    evolveSlot = int(choices.size()),
    choices.push_back(ready[rng.irange(0, int(ready.size()) - 1)]);
  else if (!relics.empty() && (treasure || rng.chance(0.03f)))
    choices.push_back(relics[rng.irange(0, int(relics.size()) - 1)]);
  Sound::levelUp.play(0.6f, 0);
}

std::array<SDL_FRect, 4> LevelUpMenu::cards() const {
  const Screen &s = game.screen;
  int n = int(choices.size());
  float u = s.u, w = std::min(s.w * 0.9f, 340 * u), h = 36 * u, gap = 6 * u;
  float y = s.h * 0.5f - (n * h + (n - 1) * gap) * 0.5f + 12 * u;
  std::array<SDL_FRect, 4> r{};
  for (int i = 0; i < 4; i++)
    r[i] = {(s.w - w) * 0.5f, y + i * (h + gap), w, h};
  return r;
}

void LevelUpMenu::tick() {
  Menu::tick();
  Input &in = game.input;
  int n = int(choices.size());
  int prev = selected;
  if (in.up.clicked)
    selected = (selected + n - 1) % n;
  if (in.down.clicked)
    selected = (selected + 1) % n;
  Key *keys[] = {&in.one, &in.two, &in.three, &in.four};
  auto r = cards();
  for (int i = 0; i < n; i++) {
    if (keys[i]->clicked)
      return choose(i);
    SDL_FPoint m{in.mouseX, in.mouseY};
    if (!SDL_PointInRectFloat(&m, &r[i]))
      continue;
    if (in.mouseMoved && selected != i)
      selected = i, Sound::select.play(0.4f, 0);
    if (in.mouseClicked && t > 0.3f)
      return choose(i);
  }
  if (selected != prev)
    Sound::select.play(0.4f, 0);
  if (in.confirm.clicked && t > 0.3f)
    choose(selected);
}

void LevelUpMenu::choose(int i) {
  Player &p = *game.level->player;
  Sound::confirm.play(0.6f, 0);
  if (Upgrade *u = choices[i]; u && i == evolveSlot) {
    u->evolve();
    spdlog::info("Level {}: evolved into {}", p.lvl, u->name);
  } else if (Upgrade *u = choices[i]) {
    u->levelUp(p);
    int relic = 0;
    for (auto &up : p.upgrades)
      if (up->rare && up.get() != u)
        relic++;
      else if (up.get() == u)
        break;
    if (u->rare)
      game.codex.relics |= 1u << relic;
    spdlog::info("Level {}: {} -> {}", p.lvl, u->name, u->level);
  } else {
    p.heal(40);
  }
  game.setMenu(--p.pendingLevels > 0 ? std::make_unique<LevelUpMenu>(game)
                                     : nullptr);
}

void LevelUpMenu::render(Screen &s) {
  const Player &p = *game.level->player;
  float u = s.u, small = std::max(1.0f, u - 1);
  auto r = cards();
  s.rect(0, 0, s.w, s.h, {0, 0, 0, 0.55f});
  s.textCenter(treasure ? "TREASURE!" : "LEVEL UP!", s.w / 2, r[0].y - 34 * u,
               2 * u, col::gold.alpha(0.85f + 0.15f * std::sin(t * 6)));

  for (int i = 0; i < int(choices.size()); i++) {
    float in = clampf((t - i * 0.06f) / 0.25f, 0, 1);
    in = 1 - (1 - in) * (1 - in) * (1 - in);
    float x = r[i].x + (1 - in) * 40 * u, y = r[i].y, w = r[i].w, h = r[i].h;
    bool sel = i == selected;
    Upgrade *up = choices[i];
    bool evolving = i == evolveSlot;
    bool kin = up && up->owner && p.hero;
    Color tint = kin ? p.hero->tint : col::gold;
    bool rare = up && (up->rare || evolving);
    float shine = 0.5f + 0.5f * std::sin(t * 4);

    s.rect(x, y, w, h, col::panel.alpha(0.92f * in));
    if (sel || rare)
      s.rect(x, y, w, h, col::gold.alpha((rare ? 0.06f + 0.06f * shine : 0.08f) * in));
    s.frame(x, y, w, h, rare ? 2 * u : u,
            (sel || rare ? col::gold : Color{0.3f, 0.28f, 0.4f, 1})
                .alpha(rare && !sel ? (0.55f + 0.45f * shine) * in : in));

    float box = 24 * u, bx = x + 6 * u, by = y + (h - box) / 2;
    s.rect(bx, by, box, box, {0.16f, 0.14f, 0.22f, in});
    s.color(col::white.alpha(in));
    s.sprite(up ? up->icon : atlas::tile(atlas::t::PotionRed), bx + 4 * u,
             by + 4 * u, 0, 0, u, u);
    s.color(col::white);

    float tx = bx + box + 6 * u;
    std::string desc = evolving ? fmt::format("{} evolves: double damage, faster",
                                              up->name)
                       : up        ? up->desc[std::min(up->level, Upgrade::MAX_LEVEL - 1)]
                                   : "Restore 40 health";
    s.text(evolving ? up->evolvedName : up ? up->name : "Roast Feast", tx,
           y + 5 * u, u,
           (evolving ? col::xp
            : rare   ? col::gold
            : kin    ? tint
                     : col::white)
               .alpha(in));
    s.text(desc, tx, y + 20 * u, small, col::muted.alpha(in));
    if (up) {
      std::string tag = evolving        ? "EVOLVE"
                        : rare            ? "RELIC"
                        : kin && up->level == 0 ? std::string(p.hero->name)
                        : up->level == 0 ? "NEW!"
                                         : fmt::format("Lv {}", up->level + 1);
      s.text(tag, x + w - Screen::textWidth(tag, small) - 6 * u, y + 5 * u,
             small,
             (kin ? tint : rare || up->level == 0 ? col::gold : col::muted)
                 .alpha(in));
    }
  }
  const SDL_FRect &last = r[choices.size() - 1];
  s.textCenter(fmt::format("1-{} / Arrows + Enter / Click", choices.size()),
               s.w / 2, last.y + last.h + 8 * u, small, col::muted);
}
