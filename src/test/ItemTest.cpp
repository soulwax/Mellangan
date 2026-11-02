#undef NDEBUG
#include <cassert>

#include <cstring>
#include <iostream>

#include "test/Tests.hpp"

#include "Game.hpp"
#include "entity/Player.hpp"
#include "entity/pickup/ItemDrop.hpp"
#include "item/Upgrade.hpp"
#include "entity/enemy/Enemy.hpp"
#include "item/loot/Loot.hpp"
#include "item/loot/Unique.hpp"
#include "level/Level.hpp"
#include "menu/InventoryMenu.hpp"
#include "hero/Hero.hpp"

namespace {

void checkItem(const Item &item) {
  assert(item.base >= 0 && item.base < int(itemBases().size()));
  if (item.rarity == Rarity::Unique) {
    assert(uniqueOf(item) && item.name() == uniqueOf(item)->name && !item.night());
    return;
  }
  assert(item.prefixes() <= 3 && item.suffixes() <= 3);
  for (int i = 0; i < item.count; i++) {
    const Roll &r = item.rolls[i];
    const AffixDef &d = AFFIXES[r.affix];
    assert(item.night() || d.tier[r.tier].qlvl <= item.ilvl);
    assert(d.slots & item.def().slots);
    assert(!d.classOnly || item.def().klass);
    assert(r.value >= d.tier[r.tier].lo - 0.05f && r.value <= d.tier[r.tier].hi + 0.05f);
    for (int k = i + 1; k < item.count; k++)
      assert(item.rolls[k].affix != r.affix);
  }
  if (item.rarity == Rarity::Normal || item.rarity == Rarity::Superior)
    assert(item.count == 0);
  if (item.rarity == Rarity::Magic)
    assert(item.count >= 1 && item.prefixes() <= 1 && item.suffixes() <= 1);
  if (item.rarity == Rarity::Rare)
    assert(item.count >= 3 || item.ilvl < 12);
  assert(!item.name().empty());
}

} // namespace

void testItems() {
  assert(itemBases().size() > 70);
  for (const ItemBase &b : itemBases())
    assert(findBase(b.key) >= 0 && b.qlvl >= 1);

  Rng a, b;
  a.state = b.state = 1234;
  for (int i = 0; i < 50; i++) {
    Item x = rollItem(30, 100, true, a), y = rollItem(30, 100, true, b);
    assert(x.save() == y.save() && x.name() == y.name());
  }

  Rng rng;
  rng.state = 99;
  int count = 100000, magic = 0, rare = 0, night = 0;
  for (int i = 0; i < count; i++) {
    int ilvl = 1 + i % 60;
    Item item = rollItem(ilvl, 0, true, rng);
    checkItem(item);
    Item back;
    assert(Item::load(item.save(), back) && back.save() == item.save());
    assert(back.name() == item.name());
    if (ilvl == 30)
      magic += item.rarity == Rarity::Magic, rare += item.rarity == Rarity::Rare,
          night += item.night();
  }
  int at30 = count / 60;
  RarityOdds o = rarityOdds(30, 0);
  float wantRare = (o.unique + (1 - o.unique) * o.rare) * at30;
  float wantMagic = (1 - o.unique) * (1 - o.rare) * o.magic * at30;
  assert(std::abs(magic - wantMagic) < wantMagic * 0.1f + 10);
  assert(std::abs(rare - wantRare) < wantRare * 0.25f + 6);

  Rng drops;
  drops.state = 7;
  for (int i = 0; i < 200; i++) {
    auto boss = rollLoot({Source::Boss, 25}, drops);
    int rares = 0;
    for (const Drop &d : boss)
      if (d.kind == DropKind::Item)
        checkItem(d.item), rares += int(d.item.rarity) >= int(Rarity::Rare);
    assert(rares >= 2);
  }
  Item bad;
  assert(!Item::load("No_Such_Base 0 1 0 1 -1", bad));
  assert(!Item::load("", bad));
  std::cout << "[TEST] Items roll from seeds, respect tiers and caps, save and load, drop at "
               "target rates: PASS"
            << std::endl;
}

void testLootDrops() {
  auto arena = [](bool loot, auto body) {
    RunRules rules;
    rules.loot = loot, rules.spawnRate = 0, rules.waves = rules.bosses = false;
    Game game;
    game.level = std::make_unique<Level>(game, rules);
    Level &level = *game.level;
    Player &p = *level.player;
    p.god = true;
    for (auto &up : p.upgrades)
      up->level = 0;
    body(level, p);
    game.level.reset();
  };
  auto kill = [](Level &level, double x, double y, bool boss) {
    auto made = Enemy::create(EnemyKind::Rat, 1, 1);
    Enemy *e = made.get();
    e->x = x, e->y = y, e->boss = boss;
    level.add(std::move(made));
    level.tick();
    e->hurt(1e9f, 0, 0, 0);
    level.tick();
  };
  arena(false, [&](Level &level, Player &p) {
    for (int i = 0; i < 20; i++)
      kill(level, p.x + 200, p.y, false);
    assert(level.count<ItemDrop>() == 0);
  });
  arena(true, [&](Level &level, Player &p) {
    kill(level, p.x + 200, p.y, true);
    int rares = 0;
    level.each<ItemDrop>([&](ItemDrop &d) {
      rares += d.drop.kind == DropKind::Item && int(d.drop.item.rarity) >= int(Rarity::Rare);
    });
    assert(rares >= 2);

    Rng r;
    r.state = 5;
    Item sword = rollItem(10, 0, false, r, Rarity::Magic);
    level.add<ItemDrop>(p.x, p.y, Drop{DropKind::Item, 1, sword}, p.x, p.y);
    level.add<ItemDrop>(p.x + 20, p.y, Drop{DropKind::Gold, 17, {}}, p.x + 20, p.y);
    for (int i = 0; i < 60; i++)
      level.tick();
    assert(p.bag.size() == 1 && p.bag[0].save() == sword.save());
    assert(p.gold >= 17);

    p.bag.resize(Player::BAG);
    level.add<ItemDrop>(p.x, p.y, Drop{DropKind::Item, 1, sword}, p.x, p.y);
    for (int i = 0; i < 60; i++)
      level.tick();
    assert(int(p.bag.size()) == Player::BAG && level.count<ItemDrop>() >= 1);

    for (int i = 0; i < 130; i++)
      level.add<ItemDrop>(p.x + 400, p.y, Drop{DropKind::Gold, 1, {}}, p.x + 400, p.y);
    level.tick();
    size_t before = level.count<ItemDrop>();
    for (int i = 0; i < 10; i++)
      ItemDrop::spill(level, p.x + 300, p.y, Source::Brute, 20);
    level.tick();
    level.each<ItemDrop>([&](ItemDrop &d) {
      if (d.x > p.x + 250 && d.x < p.x + 350)
        assert(d.drop.kind == DropKind::Item && int(d.drop.item.rarity) >= int(Rarity::Rare));
    });
    assert(level.count<ItemDrop>() >= before);
  });
  std::cout << "[TEST] Loot drops only when the rules allow, bosses drop rares, walking picks up "
               "items, gold flies in, full bags refuse, crowded ground keeps only rares: PASS"
            << std::endl;
}

void testInventory() {
  Game game;
  game.level = std::make_unique<Level>(game);
  Player &p = *game.level->player;
  auto base = [&](unsigned slot, bool classOnly, const char *notFor) {
    const auto &bases = itemBases();
    for (size_t i = 0; i < bases.size(); i++)
      if ((bases[i].slots & slot) && bool(bases[i].klass) == classOnly &&
          (!notFor || !bases[i].klass || std::strcmp(bases[i].klass, notFor) != 0)) {
        Item item;
        item.base = int(i);
        return item;
      }
    assert(false);
    return Item{};
  };
  Item sword = base(slot::Weapon, false, nullptr), axe = sword;
  axe.ilvl = 9;
  Item charm = base(slot::Charm, false, nullptr), alien = base(slot::Weapon, true, p.hero->name);
  p.bag = {sword, axe, charm, charm, alien};
  InventoryMenu menu(game);
  menu.selected = GEAR;
  menu.activate();
  assert(p.worn[GearWeapon] && p.gear[GearWeapon].ilvl == sword.ilvl && p.bag.size() == 4);
  menu.selected = GEAR;
  menu.activate();
  assert(p.gear[GearWeapon].ilvl == 9 && p.bag.size() == 4 && p.bag[0].ilvl == sword.ilvl);
  menu.selected = GEAR + 1;
  menu.activate();
  menu.selected = GEAR + 1;
  menu.activate();
  assert(p.worn[GearCharm] && p.worn[GearCharm2] && p.bag.size() == 2);
  menu.selected = GEAR + 1;
  menu.activate();
  assert(p.bag.size() == 2 && !p.worn[GearOffhand]);
  menu.selected = GearWeapon;
  menu.activate();
  assert(!p.worn[GearWeapon] && p.bag.size() == 3);
  menu.selected = GEAR;
  menu.dropSelected();
  game.level->tick();
  assert(p.bag.size() == 2 && game.level->count<ItemDrop>() == 1);
  game.level.reset();
  std::cout << "[TEST] Inventory equips, swaps, fills both charms, refuses other classes' gear, "
               "unequips and drops: PASS"
            << std::endl;
}

void testEquipStats() {
  Game game;
  game.level = std::make_unique<Level>(game);
  Level &level = *game.level;
  Player &p = *level.player;
  auto affix = [](Stat stat) {
    for (int i = 0; i < AFFIX_COUNT; i++)
      if (AFFIXES[i].stat == stat)
        return i;
    assert(false);
    return 0;
  };
  auto ring = [&](std::initializer_list<std::pair<Stat, float>> stats) {
    Item item;
    for (size_t i = 0; i < itemBases().size(); i++)
      if (itemBases()[i].slots & slot::Charm) {
        item.base = int(i);
        break;
      }
    for (auto [stat, v] : stats)
      item.rolls[item.count++] = Roll{affix(stat), 0, v};
    item.rarity = Rarity::Rare;
    return item;
  };

  float might = p.might, maxHp = p.maxHp, speed = p.speed, armor = p.armor;
  p.bag = {ring({{Stat::Damage, 20}, {Stat::Life, 30}, {Stat::Speed, 10}, {Stat::Armour, 3},
                 {Stat::ResistBurn, 50}})};
  assert(p.equip(0));
  assert(std::abs(p.might - might - 0.2f) < 1e-3f && std::abs(p.maxHp - maxHp - 30) < 1e-3f);
  assert(p.speed > speed && p.armor == armor + 3);
  p.status.apply(StatusKind::Burn, 2, 0.4f);
  assert(p.status.power(StatusKind::Burn) < 0.21f);
  float hp = p.hp;
  p.invuln = 0, p.bonus.block = 100;
  p.hurt(10);
  assert(p.hp == hp);
  p.bonus.block = 0;
  assert(p.unequip(GearCharm));
  assert(std::abs(p.might - might) < 1e-3f && std::abs(p.maxHp - maxHp) < 1e-3f);
  assert(std::abs(p.speed - speed) < 1e-3f && p.armor == armor);
  assert(std::abs(p.status.resist[int(StatusKind::Burn)]) < 1e-3f);

  bool granted = false;
  for (size_t i = 0; i < itemBases().size() && !granted; i++) {
    Item item;
    item.base = int(i);
    if (!item.def().grants || !p.fits(item))
      continue;
    Upgrade *up = p.upgrade(item.def().grants);
    if (!up || up->level != 0)
      continue;
    p.bag = {item};
    assert(p.equip(0) && up->level == item.tier + 1);
    assert(p.unequip(item.gearSlot()) && up->level == 0);
    granted = true;
  }
  assert(granted);

  p.bag = {ring({{Stat::Thorns, 5}})};
  assert(p.equip(0) && p.bonus.thorns == 5);
  p.maxHp = p.hp = 1e6f;
  auto made = Enemy::create(EnemyKind::Rat, 4, 1);
  Enemy *e = made.get();
  e->x = p.x, e->y = p.y;
  level.add(std::move(made));
  float full = e->hp;
  for (int i = 0; i < 120 && e->hp >= full && !e->removed; i++) {
    e->x = p.x, e->y = p.y, p.invuln = 0;
    level.tick();
  }
  assert(e->removed || e->hp < full);
  game.level.reset();
  std::cout << "[TEST] Equipped items move stats both ways, resist, grant levels, block and "
               "thorns: PASS" << std::endl;
}
