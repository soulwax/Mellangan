#undef NDEBUG
#include <cassert>

#include <iostream>
#include <string>

#include "test/Tests.hpp"

#include "Game.hpp"
#include "entity/Merchant.hpp"
#include "entity/Player.hpp"
#include "entity/Workbench.hpp"
#include "item/Upgrade.hpp"
#include "item/craft/Recipes.hpp"
#include "item/loot/Loot.hpp"
#include "level/Level.hpp"

namespace {

const Formula &formula(std::string_view name) {
  for (int i = 0; i < FORMULA_COUNT; i++)
    if (name == FORMULAS[i].name)
      return FORMULAS[i];
  assert(false);
  return FORMULAS[0];
}

Item plain(int slot, int ilvl) {
  const auto &bases = itemBases();
  for (size_t b = 0; b < bases.size(); b++) {
    Item item;
    item.base = int(b), item.ilvl = ilvl;
    if (item.gearSlot() == slot && !bases[b].klass)
      return item;
  }
  assert(false);
  return {};
}

Item rolled(Rarity r, bool night, Rng &rng) {
  for (int i = 0; i < 4000; i++) {
    Item item = rollItem(40, 0, night, rng, r);
    if (item.rarity == r && item.night() == night && item.count > 0)
      return item;
  }
  assert(false);
  return {};
}

bool fixedRoll(const Item &item, const char *key) {
  for (int i = 0; i < item.count; i++)
    if (item.rolls[i].affix == findAffix(key))
      return true;
  return false;
}

} // namespace

void testCrafting() {
  Rng r;
  r.state = 7;
  int kinds[MATS]{};
  for (int i = 0; i < 4000; i++)
    for (const Drop &d : rollLoot({Source::Champion, 30}, r))
      if (d.kind == DropKind::Material)
        kinds[d.material]++;
  for (int k : kinds)
    assert(k > 0);

  RunRules rules;
  rules.loot = true, rules.spawnRate = 0, rules.waves = rules.bosses = false;
  Game game;
  game.level = std::make_unique<Level>(game, rules);
  Level &level = *game.level;
  Player &p = *level.player;
  p.god = true;
  auto stock = [&](int n) {
    p.bag.clear();
    for (int &m : p.materials)
      m = n;
  };

  stock(3);
  Item magic = rolled(Rarity::Magic, false, r);
  p.bag.push_back(magic);
  const Formula &reroll = formula("Reroll");
  assert(0 <= craft(p, reroll, {0}, r) && p.bag.size() == 1 && p.materials[int(Mat::Shard)] == 0);
  assert(p.bag[0].count == magic.count && p.bag[0].name() == magic.name());
  for (int i = 0; i < magic.count; i++)
    assert(p.bag[0].rolls[i].affix == magic.rolls[i].affix && p.bag[0].quality(i) >= 0 &&
           p.bag[0].quality(i) <= 1);
  assert(0 > craft(p, reroll, {0}, r) && missing(p, reroll, {0}).rfind("needs", 0) == 0);

  stock(10);
  Item rare = rolled(Rarity::Rare, false, r);
  p.bag.push_back(rare);
  assert(0 <= craft(p, formula("Reforge"), {0}, r));
  assert(p.bag[0].rarity == Rarity::Rare && p.bag[0].name() == rare.name() &&
         p.bag[0].count >= 3 && p.bag[0].count <= 5);
  assert(p.materials[int(Mat::Shard)] == 5 && p.materials[int(Mat::Ore)] == 8);

  stock(10);
  Item worn = plain(GearArmour, 20);
  p.bag.push_back(worn);
  const Formula &upgrade = formula("Upgrade");
  assert(0 <= craft(p, upgrade, {0}, r) && p.bag[0].tier == 1 && p.bag[0].baseName() != worn.baseName());
  p.materials[int(Mat::Ore)] = 10, p.materials[int(Mat::Rune)] = 10;
  assert(0 <= craft(p, upgrade, {0}, r) && p.bag[0].tier == 2);
  p.materials[int(Mat::Ore)] = 10, p.materials[int(Mat::Rune)] = 10;
  assert(0 > craft(p, upgrade, {0}, r) && p.bag[0].tier == 2);

  stock(0);
  const Formula &combine = formula("Combine");
  int lowest = 99;
  for (int i = 0; p.bag.size() < 3 && i < 4000; i++) {
    Item m = rollItem(20 + i % 30, 0, false, r, Rarity::Magic);
    if (m.rarity == Rarity::Magic && m.gearSlot() == GearArmour)
      p.bag.push_back(m), lowest = std::min(lowest, m.ilvl);
  }
  assert(p.bag.size() == 3 && !missing(p, combine, {0, 1}).empty());
  assert(0 > craft(p, combine, {0, 0, 1}, r));
  assert(0 <= craft(p, combine, {0, 1, 2}, r) && p.bag.size() == 1);
  assert(p.bag[0].rarity == Rarity::Rare && p.bag[0].ilvl == lowest &&
         p.bag[0].gearSlot() == GearArmour);
  for (int i = 0; p.bag.size() < 3 && i < 4000; i++) {
    Item m = rollItem(30, 0, false, r, Rarity::Magic);
    if (m.rarity == Rarity::Magic && m.gearSlot() == (p.bag.size() == 1 ? GearWeapon : GearArmour))
      p.bag.push_back(m);
  }
  p.bag.push_back(p.bag.back());
  assert(missing(p, combine, {1, 2, 3}) == "the items must share a slot");

  stock(1);
  Item cursed = rolled(Rarity::Magic, true, r);
  int top = 0;
  for (int i = 0; i < cursed.count; i++)
    top = std::max(top, cursed.rolls[i].tier);
  p.bag.push_back(cursed);
  assert(0 <= craft(p, formula("Cleanse"), {0}, r) && !p.bag[0].night());
  int after = 0;
  for (int i = 0; i < p.bag[0].count; i++)
    after = std::max(after, p.bag[0].rolls[i].tier);
  assert(after <= top && (top == 0 || after < top || p.bag[0].count > 1));
  p.materials[int(Mat::Shard)] = p.materials[int(Mat::Herb)] = 1;
  assert(0 > craft(p, formula("Cleanse"), {0}, r));

  int forged = 0;
  for (int i = 0; i < FORMULA_COUNT; i++) {
    const Formula &f = FORMULAS[i];
    if (f.slot < 0)
      continue;
    stock(10);
    p.bag.push_back(plain(f.slot, 40));
    p.bag.push_back(plain((f.slot + 1) % GearCharm2, 40));
    assert(missing(p, f, {1}) == "that item does not fit this recipe");
    assert(0 <= craft(p, f, {0}, r) && p.bag.size() == 2);
    const Item &made = p.bag[0];
    assert(made.rarity == Rarity::Crafted && made.gearSlot() == f.slot);
    assert(fixedRoll(made, f.fixed[0]) && fixedRoll(made, f.fixed[1]));
    assert(made.count >= 3 && made.count <= 5 && made.prefixes() <= 3 && made.suffixes() <= 3);
    forged++;
  }
  assert(forged == 8);

  stock(0);
  Merchant::camp(level, p.x + 18, p.y);
  level.tick();
  Workbench *bench = level.get<Workbench>(level.vendor.bench);
  assert(bench && Merchant::near(level) && !Workbench::near(level));
  p.x = bench->x, p.y = bench->y + 4;
  assert(Workbench::near(level) && !Merchant::near(level));
  entt::entity first = level.vendor.bench;
  Merchant::camp(level, p.x + 300, p.y);
  level.tick();
  assert(!level.get<Workbench>(first) && level.get<Workbench>(level.vendor.bench));
  game.level.reset();
  std::cout << "[TEST] Crafting: drops carry all six materials; reroll, reforge, upgrade, "
               "combine, cleanse and eight crafted recipes each work and refuse wrong "
               "inputs; the workbench camps beside the merchant: PASS"
            << std::endl;
}

void testLootTies() {
  auto tally = [](Lean lean, Source source, auto count) {
    Rng r;
    r.state = 11;
    int n = 0;
    for (int i = 0; i < 6000; i++)
      for (const Drop &d : rollLoot({source, 30, 0, 0, false, lean}, r))
        n += count(d);
    return n;
  };
  auto gold = [](const Drop &d) { return d.kind == DropKind::Gold; };
  auto ore = [](const Drop &d) { return d.kind == DropKind::Material && d.material == int(Mat::Ore); };
  auto pelts = [](const Drop &d) {
    return d.kind == DropKind::Material &&
           (d.material == int(Mat::Hide) || d.material == int(Mat::Bone));
  };
  auto share = [&](Lean lean, unsigned slots) {
    int items = tally(lean, Source::Elite, [](const Drop &d) { return d.kind == DropKind::Item; });
    int hits = tally(lean, Source::Elite, [&](const Drop &d) {
      return d.kind == DropKind::Item && (d.item.def().slots & slots) != 0;
    });
    return float(hits) / std::max(1, items);
  };
  assert(tally(Lean::Gold, Source::Line, gold) > tally(Lean::None, Source::Line, gold) * 1.8f);
  assert(tally(Lean::Ore, Source::Line, ore) > tally(Lean::None, Source::Line, ore) * 3);
  assert(tally(Lean::Hide, Source::Line, pelts) > tally(Lean::None, Source::Line, pelts) * 4);
  assert(share(Lean::Ore, slot::Armour) > share(Lean::None, slot::Armour) * 1.5f);
  assert(share(Lean::Arcane, slot::Offhand) > share(Lean::None, slot::Offhand) * 1.5f);
  assert(share(Lean::Charm, slot::Charm) > share(Lean::None, slot::Charm) * 1.5f);
  int plain = tally(Lean::Riches, Source::Elite, [](const Drop &d) {
    return d.kind == DropKind::Item && int(d.item.rarity) < int(Rarity::Magic);
  });
  assert(plain == 0);
  std::cout << "[TEST] Loot ties: goblins lean gold, golems ore and armour, casters off-hands, "
               "wraiths charms, beasts hide and bone, mimics and robbers magic or better: PASS"
            << std::endl;
}
