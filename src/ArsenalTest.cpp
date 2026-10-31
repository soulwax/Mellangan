#include "ArsenalTest.hpp"

#include <cassert>
#include <cstring>
#include <iostream>

#include "Game.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "hero/Hero.hpp"
#include "item/skill/Skill.hpp"
#include "item/weapon/Recipe.hpp"
#include "level/Level.hpp"

namespace {
void ring(Level &l, Player &p, int n) {
  for (int i = 0; i < n; i++) {
    auto e = Enemy::create(EnemyKind::Knight, 30, 1);
    float a = TAU * i / n, d = 30 + 20 * (i % 4);
    e->x = p.x + std::cos(a) * d, e->y = p.y + std::sin(a) * d;
    l.add(std::move(e));
  }
}

RecipeWeapon *weapon(Player &p, const Recipe &r) {
  for (auto &u : p.upgrades)
    if (auto *w = dynamic_cast<RecipeWeapon *>(u.get()); w && &w->r == &r)
      return w;
  return nullptr;
}

void recipes() {
  for (const Recipe &r : RECIPES) {
    Game game;
    game.level = std::make_unique<Level>(game);
    Level &l = *game.level;
    Player &p = *l.player;
    p.god = true;
    RecipeWeapon *w = weapon(p, r);
    assert(w && w->weapon && !w->rare && !w->skill);
    assert(std::strcmp(w->desc[0], r.pitch) == 0);
    for (int i = 0; i < Upgrade::MAX_LEVEL; i++) {
      assert(w->desc[i] && w->desc[i][0]);
      w->levelUp(p);
    }
    assert(w->maxed() && w->count() >= r.count && w->reach() >= r.reach);
    ring(l, p, 8);
    for (int i = 0; i < 360 && w->dealt <= 0; i++)
      l.tick();
    if (w->dealt <= 0)
      std::cout << "[TEST] Recipe dealt no damage: " << r.name << std::endl;
    assert(w->dealt > 0);
    RecipeShot shot(r, p.x, p.bodyY(), 1, 0, 1, r.reach);
    shot.level = &l;
    for (int i = 0; i < 400 && !shot.removed; i++)
      shot.tick();
    assert(shot.removed);
  }
}

void skills() {
  for (const Hero *h : HEROES) {
    int found = 0;
    Game game;
    game.level = std::make_unique<Level>(game);
    for (auto &u : game.level->player->upgrades)
      if (u->skill && std::strcmp(u->name, h->skill) == 0 &&
          std::strcmp(u->owner, h->name) == 0)
        found++, assert(!u->rare && !u->weapon);
    assert(found == 1);
  }
  Game game;
  game.codex.start = 0;
  game.level = std::make_unique<Level>(game);
  Level &l = *game.level;
  Player &p = *l.player;
  p.god = true;
  assert(p.hero == HEROES[0] && p.skill &&
         std::strcmp(p.skill->name, p.hero->skill) == 0);
  ring(l, p, 6);
  float haste = p.haste;
  Skill *surge = nullptr, *warp = nullptr;
  int mage = 0;
  for (auto &u : p.upgrades) {
    auto *s = dynamic_cast<Skill *>(u.get());
    if (!s || std::strcmp(s->owner, "MAGE") != 0)
      continue;
    mage++;
    s->levelUp(p);
    assert(p.skill == s && p.skillTime == 0 && s->level == 0);
    p.cast();
    assert(std::abs(p.skillTime - s->cooldown * haste) < 0.001f);
    for (int i = 0; i < 360; i++)
      l.tick();
    assert(std::abs(p.haste - haste) < 0.001f);
    if (std::strcmp(s->name, "MANA SURGE") == 0)
      surge = s;
    if (std::strcmp(s->name, "TIME WARP") == 0)
      warp = s;
  }
  assert(mage == 5 && surge && warp);
  surge->levelUp(p);
  p.cast();
  assert(p.haste < haste);
  warp->levelUp(p);
  assert(std::abs(p.haste - haste) < 0.001f && p.skill == warp);
}
} // namespace

void testArsenal() {
  recipes();
  std::cout << "[TEST] " << RECIPES.size()
            << " recipe weapons level, fire, deal damage and expire: PASS"
            << std::endl;
  skills();
  std::cout << "[TEST] Class skills, Mage skill drafting and surge restore: PASS"
            << std::endl;
}

void viewArsenal(SDL_Window *window, const std::string &base) {
  Game game;
  game.window = window;
  game.codex.start = 0;
  if (!game.screen.load(base))
    return;
  SDL_RaiseWindow(window);
  game.level = std::make_unique<Level>(game);
  Level &level = *game.level;
  Player &p = *level.player;
  p.god = true;
  level.night.dawn = true;
  std::vector<Skill *> skills;
  for (auto &u : p.upgrades) {
    if (auto *w = dynamic_cast<RecipeWeapon *>(u.get()))
      while (!w->maxed())
        w->levelUp(p);
    if (auto *s = dynamic_cast<Skill *>(u.get());
        s && std::strcmp(s->owner, p.hero->name) == 0)
      skills.push_back(s);
  }
  size_t pick = 0;
  bool running = true;
  uint64_t end = SDL_GetTicks() + 90000;
  for (int frame = 0; running && SDL_GetTicks() < end; frame++) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_EVENT_QUIT ||
          (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE))
        running = false;
      if (e.type == SDL_EVENT_KEY_DOWN && e.key.key >= SDLK_1 &&
          e.key.key < SDLK_1 + int(skills.size()))
        pick = size_t(e.key.key - SDLK_1), skills[pick]->levelUp(p), p.cast();
    }
    if (frame % 240 == 0)
      ring(level, p, 10);
    if (frame % 200 == 100 && !skills.empty()) {
      skills[pick]->levelUp(p);
      p.cast();
      pick = (pick + 1) % skills.size();
    }
    level.tick();
    Screen &s = game.screen;
    s.fit(window);
    s.xOffset = int(std::floor(p.x - s.vw / 2.0));
    s.yOffset = int(std::floor(p.y - s.vh / 2.0));
    s.beginLight({0.3f, 0.3f, 0.3f, 1});
    level.renderLight(s);
    s.beginWorld({0.04f, 0.03f, 0.06f, 1});
    level.render(s);
    s.beginGui(window, {0, 0, 0, 0, 0});
    s.textCenter("ARSENAL QA: 1-5 CAST A SKILL, ESC QUITS", s.w / 2, 8 * s.u,
                 s.u, col::white);
    if (p.skill)
      s.textCenter(p.skill->name, s.w / 2, 24 * s.u, s.u, col::gold);
    vk2dRendererPresent();
    SDL_Delay(16);
  }
  vk2dRendererWait();
  game.level.reset();
  game.screen.unload();
}
