#include "ExpansionTest.hpp"

#include <cassert>
#include <iostream>
#include <set>
#include <filesystem>

#include "Game.hpp"
#include "entity/Player.hpp"
#include "hero/Hero.hpp"
#include "item/relic/ExpansionRelics.hpp"
#include "item/weapon/ExpansionWeapons.hpp"
#include "level/Noise.h"
#include "menu/ShrineMenu.hpp"

namespace {
void biomes() {
  uint32_t salt=noise_salt;
  noise_reseed(0);
  std::array<bool,BIOME_COUNT> seen{};
  std::array<int,BIOME_COUNT> xs{},ys{};
  for (int y=-4000;y<=4000;y+=41)
    for (int x=-4000;x<=4000;x+=41) {
      BiomeId id=world::biomeAt(x,y);
      assert(id>=0 && id<BIOME_COUNT);
      seen[id]=true;
      xs[id]=x;
      ys[id]=y;
    }
  for (int i=0;i<BIOME_COUNT;i++) {
    if (!seen[i])
      std::cout << "[TEST] Missing biome " << i << std::endl;
    assert(seen[i]);
    assert(BIOMES[i] && BIOMES[i]->name);
    assert(Chart::title(BiomeId(i)));
    assert(world::biomeAt(xs[i],ys[i])==i);
    if (i>=BIOME_SALT) {
      assert(BIOMES[i]->spawns[0].feature);
      assert(BIOMES[i]->spawns[1].feature);
      assert(BIOMES[i]->spawns[2].feature);
    }
  }
  noise_reseed(salt);
  std::cout << "[TEST] All 26 biomes reachable in seeded survey: PASS" << std::endl;
}

void codex() {
  auto path=std::filesystem::temp_directory_path()/"mellangan-expansion-codex-test.txt";
  std::string old="7 5 4 1 30 7 120.0\n";
  for (int i=0;i<16;i++)
    old+=std::to_string(i+1)+" ";
  old+="\n1 2 3 4 5 6 7 8 \n12\n";
  SDL_SaveFile(path.string().c_str(),old.data(),old.size());
  Codex c;
  c.load(path.string());
  assert(c.slain[15]==16 && c.slain[16]==0 && c.entered[7]==8);
  assert(c.shrines==12 && c.start==7 && c.relics==5);
  c.biomes|=1u<<25;
  c.relics|=1u<<13;
  c.slain[25]=42;
  c.start=11;
  c.save(path.string());
  Codex copy;
  copy.load(path.string());
  assert(copy.biomes==c.biomes && copy.relics==c.relics && copy.start==11);
  assert(copy.slain[25]==42 && copy.shrines==12);
  std::filesystem::remove(path);
}

void weapons() {
  Game game;
  game.level=std::make_unique<Level>(game);
  auto &l=*game.level;
  auto &p=*l.player;
  p.god=true;
  std::set<std::string> names;
  int rare=0;
  for (auto &u:p.upgrades) {
    assert(names.insert(u->name).second);
    rare+=u->rare;
  }
  assert(rare==14);
  for (int k=0;k<8;k++) {
    ExpansionWeapon w(k);
    for (int n=0;n<5;n++) {
      w.levelUp(p);
      assert(w.desc[n] && w.desc[n][0]);
      w.cd=0;
      w.tick(p);
    }
    assert(w.maxed());
    ExpansionAttack attack(k,p.x,p.bodyY(),1,0,10,60,2);
    attack.level=&l;
    for (int n=0;n<200 && !attack.removed;n++)
      attack.tick();
    assert(attack.removed);
  }
  ExpansionRelic bastion(1),mercy(7),mirror(4);
  p.hp=20;
  assert(bastion.guard(p,20)==10);
  assert(bastion.guard(p,20)==20);
  mercy.guard(p,30);
  for (int n=0;n<190;n++)
    mercy.tick(p);
  assert(p.hp>20 && p.hp<=32.01f);
  mirror.cast(p);
  assert(mirror.cd>0);
}

void heroesAndShrines() {
  for (int i=8;i<12;i++) {
    Game game;
    game.codex.biomes=(1u<<26)-1;
    game.codex.slain[0]=1000;
    game.codex.shrines=12;
    game.codex.bestStreak=100;
    game.codex.start=i;
    assert(HEROES[i]->met(game.codex));
    game.level=std::make_unique<Level>(game);
    auto &l=*game.level;
    auto &p=*l.player;
    assert(p.hero==HEROES[i]);
    bool equipped=false;
    for (auto &u:p.upgrades)
      if (u->level && std::string(u->name)==p.hero->weapon)
        equipped=true;
    assert(equipped);
    p.god=true;
    p.cast();
    assert(p.skillTime>0);
    for (int n=0;n<400;n++)
      l.tick();
    assert(!p.dead());
    l.night.offering=true;
    ShrineMenu menu(game);
    float might=p.might;
    menu.choose(0);
    assert(p.might>might && !l.night.offering);
    l.night.offering=true;
    menu.choose(1);
    l.night.offering=true;
    int levels=p.pendingLevels;
    menu.choose(2);
    assert(p.pendingLevels==levels+1);
    l.night.offering=true;
    menu.choose(3);
    assert(!l.night.offering);
  }
}
}

void testExpansionContent() {
  biomes();
  codex();
  weapons();
  heroesAndShrines();
  std::cout << "[TEST] Expansion registries, weapons, relics, heroes, shrine choices and save compatibility: PASS" << std::endl;
}
