#undef NDEBUG
#include <cassert>

#include <cmath>
#include <iostream>

#include "test/Tests.hpp"

#include "Game.hpp"
#include "campaign/Campaign.hpp"
#include "campaign/Townsfolk.hpp"
#include "entity/Ally.hpp"
#include "entity/Player.hpp"
#include "item/loot/Loot.hpp"
#include "item/loot/Vendor.hpp"
#include "level/Level.hpp"

namespace {

Townsfolk *straying(Level &level) {
  Townsfolk *out = nullptr;
  level.each<Townsfolk>([&](Townsfolk &f) {
    if (f.mode != Townsfolk::Mode::Home)
      out = &f;
  });
  return out;
}

} // namespace

void testRescue() {
  Game game;
  Campaign c;
  c.world = "RESCUE01";
  game.level = std::make_unique<Level>(game, c.rules());
  Level &level = *game.level;
  c.enter(level);
  level.tick();
  Player &p = *level.player;
  p.god = true;
  level.each<Enemy>([](Enemy &e) { e.remove(); });

  Ware ware{WareKind::Base, rollItem(20, 0, false, rng)};
  int full = level.vendor.price(ware);

  c.lostTime = 0;
  c.tick(level);
  level.tick();
  Townsfolk *f = straying(level);
  assert(f && f->mode == Townsfolk::Mode::Lost && !level.rules.safeAt(f->x, f->y));
  c.lostTime = 0;
  c.tick(level);
  level.tick();
  int strays = 0;
  level.each<Townsfolk>([&](Townsfolk &o) { strays += o.mode != Townsfolk::Mode::Home; });
  assert(strays == 1);

  p.x = f->x + 10, p.y = f->y;
  level.tick();
  assert(f->mode == Townsfolk::Mode::Following);
  double fx = f->x;
  p.x = f->x + 120;
  for (int i = 0; i < 60; i++)
    level.tick();
  assert(f->x > fx + 40);
  p.x = f->x + Townsfolk::LEASH + 40;
  level.tick();
  assert(f->mode == Townsfolk::Mode::Lost);

  p.x = f->x + 10, p.y = f->y;
  level.tick();
  assert(f->mode == Townsfolk::Mode::Following);
  int mats = 0;
  for (int m : p.materials)
    mats += m;
  f->x = c.xTown + 40, f->y = c.yTown;
  p.x = f->x + 10, p.y = f->y;
  c.tick(level);
  assert(f->mode == Townsfolk::Mode::Home && c.rescued == 1);
  int after = 0;
  for (int m : p.materials)
    after += m;
  assert(after == mats + 3);
  assert(level.vendor.price(ware) < full && level.vendor.discount == 0.05f);

  assert(!c.hound);
  c.rescued = 2;
  c.lostTime = 0;
  c.tick(level);
  level.tick();
  Townsfolk *third = straying(level);
  assert(third);
  p.x = third->x + 10, p.y = third->y;
  level.tick();
  third->x = c.xTown + 40, third->y = c.yTown;
  p.x = third->x + 10, p.y = third->y;
  c.tick(level);
  assert(c.rescued == 3 && c.hound && !c.houndDown);
  level.tick();
  Ally *dog = level.get<Ally>(c.companion);
  assert(dog && dog->hp > dog->def.hp && dog->damage == 6 + 2.0f * p.lvl);
  dog->x = p.x + 600;
  c.tick(level);
  assert(std::hypot(dog->x - p.x, dog->y - p.y) < 40);
  dog->retire();
  level.tick();
  c.tick(level);
  assert(c.houndDown && c.companion == entt::null);
  for (int i = 0; i < 30; i++)
    level.tick(), c.tick(level);
  assert(!level.get<Ally>(c.companion));
  level.night.dawn = false;
  c.tick(level);
  level.night.dawn = true;
  c.tick(level);
  assert(!c.houndDown);
  level.tick();
  assert(level.get<Ally>(c.companion));

  c.rescued = 9;
  c.tick(level);
  assert(level.vendor.discount == 0.25f);
  c.capture(level);
  Campaign back;
  assert(Campaign::load(c.save(), back) && back.rescued == 9 && back.hound);

  c.lostTime = 0;
  c.tick(level);
  level.tick();
  Townsfolk *g = straying(level);
  assert(g);
  p.x = g->x + 10, p.y = g->y;
  level.tick();
  entt::entity id = g->id;
  g->hp = -1;
  level.tick();
  assert(!level.get<Townsfolk>(id));
  c.leave();
  std::cout << "[TEST] Rescue: a lost sanctuary seeker appears outside town, follows when called, waits "
               "when left behind, joins a town for materials and a merchant discount capped at "
               "25%, three rescues earn a hound that falls back home and returns at dawn, all of it "
               "survives a save, and a sanctuary seeker can be lost to the dark: PASS"
            << std::endl;
}
