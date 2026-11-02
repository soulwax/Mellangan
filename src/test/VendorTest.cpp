#undef NDEBUG
#include <cassert>

#include <iostream>

#include "test/Tests.hpp"

#include "Game.hpp"
#include "entity/Merchant.hpp"
#include "entity/Player.hpp"
#include "item/Upgrade.hpp"
#include "item/loot/Loot.hpp"
#include "item/loot/Unique.hpp"
#include "item/loot/Vendor.hpp"
#include "level/Level.hpp"

namespace {

template <class F> void market(F body) {
  RunRules rules;
  rules.loot = true, rules.spawnRate = 0, rules.waves = rules.bosses = false;
  Game game;
  game.level = std::make_unique<Level>(game, rules);
  Level &level = *game.level;
  Player &p = *level.player;
  p.god = true;
  for (auto &up : p.upgrades)
    up->level = 0;
  body(level, p);
  game.level.reset();
}

size_t find(const Vendor &v, WareKind kind) {
  for (size_t i = 0; i < v.wares.size(); i++)
    if (v.wares[i].kind == kind)
      return i;
  assert(false);
  return 0;
}

} // namespace

void testVendor() {
  Rng r;
  r.state = 99;
  Vendor v;
  v.restock(40, r);
  int kinds[4]{};
  for (const Ware &w : v.wares) {
    kinds[int(w.kind)]++;
    assert(w.item.ilvl == 40 && !w.sold);
    bool magic = w.kind == WareKind::Magic;
    assert((w.item.rarity == Rarity::Magic) == magic && (w.item.count > 0) == magic);
    if (w.kind != WareKind::Draught)
      assert(v.price(w) > Vendor::sellPrice(w.item) * 12);
  }
  assert(kinds[0] == 2 && kinds[1] == 3 && kinds[2] >= 2 && kinds[2] <= 3 && kinds[3] == 4);

  const Ware &bet = v.wares[find(v, WareKind::Gamble)];
  int rarity[int(Rarity::Count)]{};
  for (int i = 0; i < 3000; i++) {
    Item g = gamble(bet.item, r);
    rarity[int(g.rarity)]++;
    assert(g.base == bet.item.base && g.tier == bet.item.tier && !g.night());
    assert(v.price(bet) > Vendor::sellPrice(g) * 4);
  }
  assert(!rarity[int(Rarity::Normal)] && !rarity[int(Rarity::Superior)]);
  assert(rarity[int(Rarity::Rare)] > 90 && rarity[int(Rarity::Rare)] < 360);

  Item charm;
  charm.base = findBase(UNIQUES[0].base), charm.ilvl = 99;
  int uniques = 0;
  for (int i = 0; i < 4000; i++)
    if (Item g = gamble(charm, r); g.rarity == Rarity::Unique)
      uniques++, assert(g.base == charm.base && uniqueOf(g));
  assert(uniques > 10);

  int buys = 0;
  for (uint64_t s = 0; s < 8; s++)
    buys += vendorSim(30, s, false);
  assert(buys >= 3 * 8 && buys <= 6 * 8);

  market([&](Level &level, Player &p) {
    Merchant::camp(level, p.x + 18, p.y);
    level.tick();
    Vendor &shop = level.vendor;
    assert(shop.restocks == 1 && level.get<Merchant>(shop.merchant) && Merchant::near(level));

    size_t magic = find(shop, WareKind::Magic);
    Item ware = shop.wares[magic].item;
    p.gold = shop.price(shop.wares[magic]);
    assert(shop.buy(p, magic, r) && p.gold == 0 && p.bag.size() == 1);
    assert(p.bag.back().name() == ware.name() && shop.wares[magic].sold);
    p.gold = 1 << 30;
    assert(!shop.buy(p, magic, r));
    p.gold = 0;
    assert(!shop.buy(p, find(shop, WareKind::Base), r));

    p.gold = 1 << 30;
    size_t bet = find(shop, WareKind::Gamble);
    assert(shop.buy(p, bet, r) && shop.buy(p, bet, r) && !shop.wares[bet].sold);
    assert(p.bag.size() == 3 && int(p.bag.back().rarity) >= int(Rarity::Magic));

    int gold = p.gold, worth = Vendor::sellPrice(p.bag[0]);
    assert(Vendor::sell(p, 0) && p.gold == gold + worth && p.bag.size() == 2);

    while (int(p.bag.size()) < Player::BAG)
      p.bag.push_back(ware);
    assert(!shop.buy(p, find(shop, WareKind::Base), r));
    p.hp = 10;
    assert(shop.buy(p, find(shop, WareKind::Draught), r) && p.hp > 10);

    entt::entity first = shop.merchant;
    p.x += 400;
    assert(!Merchant::near(level));
    Merchant::camp(level, p.x + 10, p.y + 10);
    level.tick();
    assert(!level.get<Merchant>(first) && shop.restocks == 2 && Merchant::near(level));
    assert(shop.wares.size() >= 11 && !shop.wares[magic].sold);

    level.night.dawn = true;
    Merchant::visit(level);
    Merchant::visit(level);
    assert(shop.restocks == 3);
  });
  std::cout << "[TEST] Vendor: stock and prices, buying and selling, gambles roll magic or "
               "better on their base, 30 minutes buy 3-6 magic items, the merchant camps "
               "and restocks at dawn: PASS"
            << std::endl;
}
