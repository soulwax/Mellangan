#include "menu/LevelUpMenu.hpp"

#include <spdlog/spdlog.h>

#include <cstring>

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "hero/Hero.hpp"
#include "item/Upgrade.hpp"
#include "item/skill/Skill.hpp"
#include "menu/Menu.hpp"
#include "menu/UpgradeCard.hpp"

LevelUpMenu::LevelUpMenu(Game &game) : Menu(game) {
  Player &p = *game.level->player;
  auto &all = p.upgrades;
  if (p.pendingTreasures > 0)
    p.pendingTreasures--, treasure = true;
  int owned[2] = {0, 0};
  for (auto &u : all)
    if (u->level > 0 && !u->rare && !u->skill)
      owned[u->weapon]++;
  std::vector<Upgrade *> relics;
  for (auto &u : all)
    if (u->hidden)
      continue;
    else if (u->rare && !u->maxed())
      relics.push_back(u.get());
    else if (!u->rare && !u->skill && !u->maxed() &&
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
  std::vector<Upgrade *> skills;
  for (auto &u : all)
    if (u->skill && u.get() != p.skill && p.hero &&
        (!u->owner || std::strcmp(u->owner, p.hero->name) == 0))
      skills.push_back(u.get());
  if (!skills.empty() && choices.front() && p.lvl >= 4 && rng.chance(0.25f)) {
    choices.push_back(skills[rng.irange(0, int(skills.size()) - 1)]);
  } else if (treasure && !ready.empty())
    evolveSlot = int(choices.size()),
    choices.push_back(ready[rng.irange(0, int(ready.size()) - 1)]);
  else if (!relics.empty() && (treasure || rng.chance(0.03f)))
    choices.push_back(relics[rng.irange(0, int(relics.size()) - 1)]);
  for (int i = 0; i < int(choices.size()); i++) {
    UpgradeCard card(choices[i], p, i == evolveSlot, i);
    bool paired = false;
    lines = std::max(lines, 1 + !card.numbers().empty() + !card.synergy(paired).empty());
  }
  Sound::levelUp.play(0.6f, 0);
}

float LevelUpMenu::cardHeight(const Screen &s) const {
  return 24 * s.u + lines * 15 * std::max(1.0f, s.u - 1);
}

std::unique_ptr<Menu> LevelUpMenu::open(Game &game) {
  Player &p = *game.level->player;
  while (p.pendingLevels > 0) {
    auto menu = std::make_unique<LevelUpMenu>(game);
    if (menu->choices.size() > 1)
      return menu;
    menu->apply(0);
  }
  return nullptr;
}


void LevelUpMenu::tick() {
  Menu::tick();
  Input &in = game.input;
  auto result = selection.update(in, gui::ChoiceList::layout(game.screen, int(choices.size()), cardHeight(game.screen)),
                                 int(choices.size()), t > 0.3f);
  if (result.selectionChanged)
    Sound::select.play(0.4f, 0);
  if (result.activated >= 0)
    choose(result.activated);
}

void LevelUpMenu::choose(int i) {
  apply(i);
  game.setMenu(open(game));
}

void LevelUpMenu::apply(int i) {
  Player &p = *game.level->player;
  Sound::confirm.play(0.6f, 0);
  Upgrade *choice = choices[i];
  if (choice && i == evolveSlot) {
    choice->evolve();
    spdlog::info("Level {}: evolved into {}", p.lvl, choice->name);
  } else if (choice) {
    choice->levelUp(p);
    game.level->trace.picks.push_back(choice->name);
    int relic = 0;
    for (auto &up : p.upgrades)
      if (up->rare && up.get() != choice)
        relic++;
      else if (up.get() == choice)
        break;
    if (choice->rare)
      game.codex.relics |= 1u << relic;
    spdlog::info("Level {}: {} -> {}", p.lvl, choice->name, choice->level);
  } else {
    p.heal(40);
  }
  --p.pendingLevels;
}

void LevelUpMenu::render(Screen &s) {
  const Player &p = *game.level->player;
  float u = s.u, small = std::max(1.0f, u - 1);
  auto r = gui::ChoiceList::layout(s, int(choices.size()), cardHeight(s));
  s.textCenter(treasure ? "TREASURE!" : "LEVEL UP!", s.w / 2, r[0].y - 34 * u,
               2 * u, col::gold.alpha(0.88f + 0.12f * std::sin(t * 5)));

  for (int i = 0; i < int(choices.size()); i++) {
    float in = gui::ChoiceList::entrance(t, i);
    SDL_FRect card = r[i];
    card.x += (1 - in) * 32 * u;
    UpgradeCard(choices[i], p, i == evolveSlot, i)
        .render(s, card, in, i == selection.selected, t);
  }
  const SDL_FRect &last = r[choices.size() - 1];
  s.textCenter(game.input.prompt(fmt::format("1-{} / Arrows + Enter / Click", choices.size())),
               s.w / 2, last.y + last.h + 9 * u, small, col::muted * 0.75f);
}
