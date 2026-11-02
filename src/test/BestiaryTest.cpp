#undef NDEBUG
#include <cassert>

#include <iostream>

#include "test/Tests.hpp"

#include "Game.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Hazard.hpp"
#include "entity/enemy/WildFoe.hpp"
#include "entity/projectile/Arrow.hpp"
#include "entity/projectile/Orb.hpp"
#include "item/Upgrade.hpp"
#include "level/Level.hpp"
#include "level/World.hpp"

namespace {

struct Arena {
  Game game;
  Level &level;
  Player &p;

  Arena() : level(*(game.level = std::make_unique<Level>(game))), p(*level.player) {
    p.god = true;
    for (auto &up : p.upgrades)
      up->level = 0;
  }
  ~Arena() { game.level.reset(); }

  Enemy *any(EnemyKind kind, double dx, double dy, Trait trait) {
    auto made = Enemy::create(kind, 4, 1);
    Enemy *e = made.get();
    e->x = p.x + dx, e->y = p.y + dy;
    e->traits.add(trait);
    level.add(std::move(made));
    return e;
  }

  int orbs() {
    int n = 0;
    for (auto [id, o] : level.reg.view<Owned>().each())
      n += dynamic_cast<Orb *>(o.self.get()) && !o.self->removed;
    return n;
  }

  WildFoe *add(EnemyKind kind, double dx, double dy) {
    auto made = Enemy::create(kind, 4, 1);
    auto *e = dynamic_cast<WildFoe *>(made.get());
    assert(e);
    e->x = p.x + dx, e->y = p.y + dy;
    level.add(std::move(made));
    return e;
  }

  void run(int ticks) {
    for (int i = 0; i < ticks; i++)
      level.tick();
  }

  template <class T> int particles() {
    int n = 0;
    for (auto &q : level.particles)
      n += !q->removed && dynamic_cast<T *>(q.get());
    return n;
  }

  int count(EnemyKind kind) {
    int n = 0;
    for (auto [id, o] : level.reg.view<Owned>().each())
      if (auto *e = dynamic_cast<Enemy *>(o.self.get()); e && !e->removed && e->is(kind))
        n++;
    return n;
  }

  int hostileArrows() {
    int n = 0;
    for (auto [id, o] : level.reg.view<Owned>().each())
      if (auto *a = dynamic_cast<Arrow *>(o.self.get()); a && !a->removed && a->hostile)
        n++;
    return n;
  }
};

} // namespace

void testGreenwoodFoes() {
  for (int k = int(WildFoe::FIRST); k <= int(WildFoe::LAST); k++) {
    Arena a;
    WildFoe *e = a.add(EnemyKind(k), 70, 20);
    assert(e->sprite().w == 16 && e->name() && e->plan);
    a.run(240);
  }
  {
    Arena a;
    WildFoe *shaman = a.add(EnemyKind::GoblinShaman, 120, 0);
    WildFoe *archer = a.add(EnemyKind::GoblinArcher, 150, 0);
    a.run(2);
    archer->hp = archer->maxHp * 0.4f;
    float hp = archer->hp;
    a.run(150);
    assert(!shaman->removed && archer->hp > hp);
  }
  {
    Arena a;
    WildFoe *sapling = a.add(EnemyKind::Sapling, 90, 0);
    a.run(2);
    sapling->life = sapling->wild.grow - DT * 0.5f;
    a.run(3);
    assert(sapling->removed && a.level.count<Enemy>() >= 1);
    bool treant = false;
    a.level.each<Enemy>([&](Enemy &e) { treant |= e.is(EnemyKind::Treant); });
    assert(treant);
  }
  {
    Arena a;
    a.add(EnemyKind::MothWisp, 0, 0);
    a.run(3);
    assert(a.p.dim > 0);
  }
  {
    Arena a;
    WildFoe *knife = a.add(EnemyKind::Cutthroat, -20, 0);
    a.p.facing = 1;
    assert(knife->contactBonus(a.p) == 1.5f);
    knife->x = a.p.x + 20;
    assert(knife->contactBonus(a.p) == 1.0f);
  }
  {
    Arena a;
    WildFoe *thorn = a.add(EnemyKind::Thornling, 50, 0);
    a.run(2);
    thorn->hurt(1e6f, 0, 0, 0);
    a.run(1);
    assert(a.particles<Bramble>() == 1);
  }
  {
    Arena a;
    a.add(EnemyKind::GoblinArcher, 140, 0);
    int seen = 0;
    for (int i = 0; i < 400 && !seen; i++)
      a.run(1), seen = a.hostileArrows();
    assert(seen >= 1);
  }
  std::cout << "[TEST] Greenwood foes: shamans mend, saplings grow, moths dim, cutthroats "
               "backstab, thornlings leave brambles, archers loose volleys: PASS"
            << std::endl;
}

void testChampionTraits() {
  auto contact = [](Trait trait, auto check) {
    Arena a;
    a.p.god = false, a.p.hp = a.p.maxHp = 1e6f, a.p.invuln = 0;
    a.any(EnemyKind::Knight, 3, 0, trait);
    float skill = a.p.skillTime;
    for (int i = 0; i < 30 && a.p.hp >= a.p.maxHp; i++)
      a.run(1);
    assert(a.p.hp < a.p.maxHp);
    check(a.p, skill);
  };
  contact(Trait::Cursed, [](Player &p, float) { assert(p.status.power(StatusKind::Weaken) > 0); });
  contact(Trait::Silencing, [](Player &p, float skill) { assert(p.skillTime >= skill + 1.9f); });
  contact(Trait::FireEnchanted, [](Player &p, float) { assert(p.status.power(StatusKind::Burn) > 0); });
  {
    Arena a;
    a.p.god = false, a.p.hp = a.p.maxHp = 1e6f;
    Enemy *e = a.any(EnemyKind::Rat, 30, 0, Trait::FireEnchanted);
    a.run(2);
    a.p.status.clear();
    e->hurt(1e6f, 0, 0, 0);
    assert(a.p.status.power(StatusKind::Burn) > 0);
  }
  {
    Arena a;
    Enemy *e = a.any(EnemyKind::Knight, 200, 0, Trait::StoneSkin);
    a.run(2);
    assert(std::abs(e->traits.incoming(*e, 100) - 65) < 1e-3f);
    assert(e->traits.speed() < 1);
  }
  {
    Arena a;
    a.any(EnemyKind::Cultist, 120, 0, Trait::Multishot);
    int most = 0;
    for (int i = 0; i < 600 && most < 3; i++)
      a.run(1), most = std::max(most, a.orbs());
    assert(most >= 3);
  }
  {
    Arena a;
    a.any(EnemyKind::Knight, 150, 0, Trait::AuraBearer);
    Enemy *ally = a.any(EnemyKind::Rat, 170, 10, Trait::Hasted);
    a.run(3);
    assert(ally->frenzy > 0);
  }
  std::cout << "[TEST] Cursed, silencing, enchanted, stone skin, multishot and aura bearer "
               "traits: PASS"
            << std::endl;
}

void testMarchesFoes() {
  auto water = [](Player &p, double &wx, double &wy) {
    int px = int(p.x / 16), py = int(p.y / 16);
    for (int r = 0; r < 400; r++)
      for (int i = 0; i < 8 * std::max(1, r); i++) {
        double a = TAU * i / (8 * std::max(1, r));
        int tx = px + int(std::cos(a) * r), ty = py + int(std::sin(a) * r);
        if (world::isWater(tx, ty) && world::isWater(tx + 1, ty) && world::isWater(tx, ty + 1)) {
          wx = tx * 16 + 8, wy = ty * 16 + 8;
          return true;
        }
      }
    return false;
  };
  {
    Arena a;
    WildFoe *toad = a.add(EnemyKind::BogToad, 100, 0);
    a.run(2);
    double start = toad->x - a.p.x;
    bool flew = false, chilled = false;
    for (int i = 0; i < 240 && !chilled; i++) {
      a.run(1);
      flew |= toad->z > 4;
      chilled |= a.p.status.power(StatusKind::Chill) > 0.5f;
    }
    assert(flew && chilled && std::abs(toad->x - a.p.x) < start);

    WildFoe *tongue = a.add(EnemyKind::BogToad, 80, 0);
    a.run(1);
    double before = tongue->x - a.p.x;
    tongue->casts->fn(*tongue);
    assert(tongue->x - a.p.x < before - 10);
  }
  {
    Arena a;
    a.p.luck = -1;
    WildFoe *beetle = a.add(EnemyKind::AmberBeetle, 90, 0);
    a.run(20);
    assert(beetle->xMark < -0.9);
    float hp = beetle->hp;
    beetle->hurt(10, 1, 0, 0);
    float front = hp - beetle->hp;
    hp = beetle->hp;
    beetle->hurt(10, -1, 0, 0);
    float back = hp - beetle->hp;
    assert(std::abs(front - 3) < 0.01f && std::abs(back - 13) < 0.01f);
  }
  {
    Arena a;
    WildFoe *vine = a.add(EnemyKind::StranglerVine, 60, 0);
    a.run(2);
    double home = vine->x;
    a.run(150);
    assert(std::abs(vine->x - home) < 0.01);
    assert(a.p.status.power(StatusKind::Chill) > 0 || vine->x - a.p.x < 60);
  }
  {
    Arena a;
    WildFoe *puffer = a.add(EnemyKind::SporePuffer, 10, 0);
    entt::entity id = puffer->id;
    a.run(10);
    assert(!a.level.get<Enemy>(id));
  }
  {
    Arena a;
    double wx, wy;
    WildFoe *dry = a.add(EnemyKind::Drowned, 300, 300);
    entt::entity dryId = dry->id;
    assert(water(a.p, wx, wy));
    {
      a.run(2);
      bool landlocked = true;
      for (int dy = -8; dy <= 8 && landlocked; dy++)
        for (int dx = -8; dx <= 8; dx++)
          landlocked &= !world::isWater(int((a.p.x + 300) / 16) + dx, int((a.p.y + 300) / 16) + dy);
      if (landlocked)
        assert(!a.level.get<Enemy>(dryId));
      a.p.x = wx - 200, a.p.y = wy;
      WildFoe *snap = a.add(EnemyKind::Snapjaw, 200, 0);
      a.run(2);
      assert(a.level.get<Enemy>(snap->id) && snap->buried);
      a.p.x = snap->x - 40, a.p.y = snap->y;
      for (int i = 0; i < 30 && snap->buried; i++)
        a.run(1);
      assert(!snap->buried);
      WildFoe *priest = a.add(EnemyKind::SunkenPriest, 200, 30);
      a.run(1);
      assert(priest->casts && priest->castCount == 1);
    }
  }
  std::cout << "[TEST] Marches foes: toads leap, stun and pull, beetles shrug off the front, "
               "vines hold their ground and root, puffers burst, the drowned keep to water, "
               "snapjaws wait beneath it: PASS"
            << std::endl;
}

void testFrostFoes() {
  {
    Arena a;
    WildFoe *yeti = a.add(EnemyKind::Yeti, 120, 0);
    a.run(1);
    int orbs = a.orbs();
    yeti->casts->fn(*yeti);
    a.run(1);
    assert(a.orbs() == orbs + 1);
    bool staggered = false;
    for (int i = 0; i < 150 && !staggered; i++)
      a.run(1), staggered = a.p.status.power(StatusKind::Chill) > 0.6f;
    assert(staggered);
    yeti->x = a.p.x + 30, yeti->y = a.p.y;
    orbs = a.orbs();
    yeti->casts->fn(*yeti);
    a.run(1);
    assert(a.orbs() == orbs);
  }
  {
    Arena a;
    WildFoe *wraith = a.add(EnemyKind::IceWraith, 20, 0);
    a.run(5);
    assert(wraith->type.floats && a.p.status.power(StatusKind::Chill) > 0);
  }
  {
    Arena a;
    a.p.god = false;
    WildFoe *wight = a.add(EnemyKind::BarrowWight, 70, 0);
    a.run(1);
    wight->hp = wight->maxHp / 2;
    float hp = a.p.hp;
    for (int i = 0; i < 240 && wight->drainTime <= 0; i++)
      a.run(1);
    assert(wight->drainTime > 0);
    a.run(30);
    assert(a.p.hp < hp && wight->hp > wight->maxHp / 2);
    a.p.x = wight->x - 200, a.p.y = wight->y;
    a.run(2);
    assert(wight->drainTime <= 0);
  }
  {
    Arena a;
    a.p.luck = -1;
    WildFoe *golem = a.add(EnemyKind::CrystalGolem, 90, 0);
    a.run(20);
    assert(golem->xMark < -0.9);
    int orbs = a.orbs();
    float hp = golem->hp;
    golem->hurt(10, 1, 0, 0);
    a.run(1);
    assert(std::abs(hp - golem->hp - 3) < 0.01f && a.orbs() > orbs);
    golem->hurt(golem->maxHp * 10, 1, 0, 0);
    a.run(2);
    assert(a.count(EnemyKind::CrystalGolem) == 0 && a.count(EnemyKind::CrystalShard) == 3);
  }
  {
    Arena a;
    a.level.spawner.spawn(EnemyKind::MoonHound, a.p.x + 150, a.p.y);
    a.run(1);
    assert(a.count(EnemyKind::MoonHound) == 4);
    WildFoe *hound = a.add(EnemyKind::MoonHound, -120, 0);
    a.run(1);
    bool blinked = false;
    for (int i = 0; i < 400 && !blinked; i++) {
      double x = hound->x, y = hound->y;
      a.run(1);
      blinked = (hound->x - x) * (hound->x - x) + (hound->y - y) * (hound->y - y) > 20 * 20;
    }
    assert(blinked);
    a.level.night.dawn = true;
    a.run(2);
    assert(a.count(EnemyKind::MoonHound) == 0);
  }
  {
    Arena a;
    WildFoe *lynx = a.add(EnemyKind::Lynx, 250, 0);
    a.run(2);
    assert(lynx->buried);
    a.p.x = lynx->x - 70, a.p.y = lynx->y;
    for (int i = 0; i < 30 && lynx->buried; i++)
      a.run(1);
    assert(!lynx->buried);
    bool flew = false;
    for (int i = 0; i < 120 && !flew; i++)
      a.run(1), flew = lynx->z > 4;
    assert(flew);
  }
  {
    Arena a;
    WildFoe *archer = a.add(EnemyKind::BarrowArcher, 100, 0);
    a.run(1);
    int arrows = a.hostileArrows();
    archer->casts->fn(*archer);
    a.run(1);
    assert(a.hostileArrows() == arrows + 1);
    bool chilled = false;
    for (int i = 0; i < 90 && !chilled; i++)
      a.run(1), chilled = a.p.status.power(StatusKind::Chill) > 0.3f;
    assert(chilled);
  }
  std::cout << "[TEST] Frost foes: yetis throw staggering snowballs from range, wraiths chill "
               "close by, wights drain until you step away, golems throw frontal hits back "
               "and shatter into three shards, moon hounds hunt in fours, blink and fade at "
               "dawn, lynxes wait hidden and pounce, barrow arrows chill: PASS"
            << std::endl;
}
