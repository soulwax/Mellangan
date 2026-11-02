#undef NDEBUG
#include <cassert>

#include <iostream>
#include <sstream>

#include "test/Tests.hpp"

#include "Game.hpp"
#include "campaign/Campaign.hpp"
#include "campaign/Stash.hpp"
#include "entity/Merchant.hpp"
#include "entity/Player.hpp"
#include "entity/Workbench.hpp"
#include "entity/enemy/Enemy.hpp"
#include "item/Upgrade.hpp"
#include "item/loot/Loot.hpp"
#include "level/Edits.hpp"
#include "level/Level.hpp"
#include "level/Prop.hpp"
#include "level/World.hpp"

namespace {

bool sameProp(int x, int y, Edits *a, Edits *b) {
  Prop pa, pb;
  world::edits = a;
  bool ha = world::propAt(x, y, pa);
  world::edits = b;
  bool hb = world::propAt(x, y, pb);
  world::edits = nullptr;
  if (ha != hb)
    return false;
  if (!ha)
    return true;
  return pa.count == pb.count && pa.solid == pb.solid &&
         (pa.count == 0 || pa.parts[0].sprite.x == pb.parts[0].sprite.x);
}

void play(Game &game, Campaign &c) {
  game.level = std::make_unique<Level>(game, c.rules());
  c.enter(*game.level);
  game.level->tick();
}

} // namespace

void testCampaign() {
  {
    Edits e;
    e.place(3, -4, Piece::Torch);
    e.harvest(-5, 6, 12.5f);
    Edits back;
    std::istringstream lines(e.save());
    for (std::string line; std::getline(lines, line);)
      assert(back.load(line));
    assert(back.changes.size() == 2 && back.at(3, -4)->placed &&
           back.at(3, -4)->piece == Piece::Torch && !back.at(-5, 6)->placed &&
           back.at(-5, 6)->regrow == 12.5f && !back.at(0, 0));
    assert(!back.load("edit q 1 2 3") && !back.load("nonsense"));
  }
  {
    int tx = 0, ty = 0;
    Prop p;
    bool found = false;
    for (int r = 0; r < 60 && !found; r++)
      for (int i = -r; i <= r && !found; i++)
        if (world::propAt(i, r, p) && p.count > 0)
          tx = i, ty = r, found = true;
    assert(found);
    Edits e;
    world::edits = &e;
    e.harvest(tx, ty);
    assert(!world::propAt(tx, ty, p));
    e.place(tx, ty, Piece::Campfire);
    assert(world::propAt(tx, ty, p) && p.count == 1 && p.light.r > 0 && p.solid > 0);
    e.clear(tx, ty);
    assert(world::propAt(tx, ty, p) && p.count > 0);
    world::edits = nullptr;
  }

  Game game;
  Campaign first;
  first.world = "TESTWRLD", first.hero = 0;
  play(game, first);
  Level &level = *game.level;
  Player &p = *level.player;
  assert(first.founded && world::edits == &first.edits && !level.rules.endsAtDawn);
  assert(level.rules.safeAt(first.xTown, first.yTown) && level.night.dawn);
  int cx = int(std::floor(first.xTown / 16)), cy = int(std::floor(first.yTown / 16));
  Prop prop;
  assert(world::propAt(cx, cy + 2, prop) && prop.light.r > 0);
  assert(!world::propAt(cx + 1, cy, prop));
  assert(level.rules.threat(first.xTown, first.yTown) < 1 &&
         level.rules.threat(first.xTown + 1600, first.yTown) > 150);
  Merchant *trader = level.get<Merchant>(level.vendor.merchant);
  assert(trader && level.get<Workbench>(level.vendor.bench) && level.vendor.restocks == 1);
  assert(level.rules.safeAt(trader->x, trader->y) && level.rules.dayRate > 0);
  double tx = trader->x;
  level.night.lit = 1;
  Merchant::visit(level);
  assert(level.get<Merchant>(level.vendor.merchant)->x == tx);
  level.night.lit = 0;
  p.god = true;
  for (int i = 0; i < 900 && level.count<Enemy>() == 0; i++)
    level.tick();
  assert(level.night.dawn && level.count<Enemy>() > 0);
  p.god = false;

  p.gold = 500, p.materials[int(Mat::Rune)] = 4;
  Rng r;
  r.state = 3;
  for (int i = 0; i < 3; i++)
    p.bag.push_back(rollItem(30, 0, false, r, Rarity::Magic));
  Item armour;
  for (int i = 0; i < 200 && armour.gearSlot() != GearArmour; i++)
    armour = rollItem(20, 0, false, r, Rarity::Rare);
  assert(armour.gearSlot() == GearArmour);
  p.bag.push_back(armour);
  assert(p.equip(p.bag.size() - 1) && p.worn[GearArmour]);
  Upgrade *learned = nullptr;
  for (auto &up : p.upgrades)
    if (up->level == 0 && !up->skill && !up->maxed()) {
      learned = up.get();
      break;
    }
  assert(learned);
  learned->levelUp(p), learned->levelUp(p);
  first.stash.push_back(rollItem(10, 0, false, r, Rarity::Rare));
  first.edits.place(cx + 20, cy + 20, Piece::Torch);
  first.edits.harvest(cx - 20, cy - 20, 30);
  p.x += 37, p.y -= 11;
  first.capture(level);
  std::string text = first.save();

  Campaign second;
  assert(Campaign::load(text, second));
  assert(second.world == "TESTWRLD" && second.gold == 500 && second.founded &&
         second.materials[int(Mat::Rune)] == 4 && second.bag.size() == 3 &&
         second.stash.size() == 1 && second.worn[GearArmour] &&
         second.gear[GearArmour].name() == armour.name());
  assert(second.edits.changes.size() == first.edits.changes.size());
  for (const auto &[k, c] : first.edits.changes) {
    const Edits::Change &d = second.edits.changes.at(k);
    assert(d.placed == c.placed && d.piece == c.piece && std::abs(d.regrow - c.regrow) < 0.06f);
  }
  for (int y = cy - 22; y <= cy + 22; y++)
    for (int x = cx - 22; x <= cx + 22; x++)
      assert(sameProp(x, y, &first.edits, &second.edits));
  std::string name = learned->name;
  int levels = learned->level;
  first.leave();

  Game again;
  play(again, second);
  Player &q = *again.level->player;
  assert(q.gold == 500 && q.materials[int(Mat::Rune)] == 4 && q.bag.size() == 3);
  assert(q.worn[GearArmour] && q.gear[GearArmour].name() == armour.name());
  assert(q.upgrade(name)->level == levels);
  assert(std::abs(q.x - p.x) < 0.2 && std::abs(q.y - p.y) < 0.2);
  assert(world::edits == &second.edits);
  assert(Campaign::load(second.save(), first) && first.edits.changes.size() == second.edits.changes.size());

  q.x = second.xTown + 40, q.y = second.yTown;
  again.campaign = std::make_unique<Campaign>(second);
  again.campaign->chest = second.chest;
  assert(Stash::near(*again.level));
  q.hp = 0;
  second.fall(*again.level);
  assert(q.gold == 450 && q.hp == q.maxHp && std::abs(q.x - second.xTown) < 1);
  again.campaign.reset();
  second.leave();
  assert(!world::edits);
  std::cout << "[TEST] Campaign: edits save, load and reshape props; a founded town has its "
               "fence, fire and safe zone; hero, gear, bag, stash, upgrades and every edited "
               "prop survive a save and reload; falling wakes you by the waypoint for a tenth "
               "of your gold: PASS"
            << std::endl;
}
