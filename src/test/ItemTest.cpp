#undef NDEBUG
#include <cassert>

#include <iostream>

#include "test/Tests.hpp"

#include "item/loot/Loot.hpp"

namespace {

void checkItem(const Item &item) {
  assert(item.base >= 0 && item.base < int(itemBases().size()));
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
