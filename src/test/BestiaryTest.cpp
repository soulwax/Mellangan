#undef NDEBUG
#include <cassert>

#include <iostream>

#include "test/Tests.hpp"

#include "Game.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Hazard.hpp"
#include "entity/enemy/Named.hpp"
#include "entity/enemy/WildFoe.hpp"
#include "entity/pickup/ItemDrop.hpp"
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

  int drops(DropKind kind) {
    int n = 0;
    for (auto [id, o] : level.reg.view<Owned>().each())
      if (auto *d = dynamic_cast<ItemDrop *>(o.self.get()); d && !d->removed && d->drop.kind == kind)
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

void testScorchedFoes() {
  {
    Arena a;
    a.p.luck = -1;
    WildFoe *wyrm = a.add(EnemyKind::SandWyrm, 60, 0);
    a.run(2);
    assert(wyrm->buried);
    float hp = wyrm->hp;
    wyrm->hurt(10, 1, 0, 0);
    assert(wyrm->hp == hp);
    int blasts = 0;
    for (int i = 0; i < 600 && wyrm->buried; i++) {
      a.run(1);
      blasts = std::max(blasts, wyrm->eruptions);
    }
    assert(!wyrm->buried && blasts == 3);
    assert(std::abs(wyrm->x - a.p.x) < 4 && std::abs(wyrm->y - a.p.y) < 4);
    hp = wyrm->hp;
    wyrm->hurt(10, 1, 0, 0);
    assert(std::abs(hp - wyrm->hp - 15) < 0.01f);
    a.level.spawner.spawn(EnemyKind::SandWyrm, a.p.x + 200, a.p.y);
    a.level.spawner.spawn(EnemyKind::SandWyrm, a.p.x - 200, a.p.y);
    a.run(1);
    assert(a.count(EnemyKind::SandWyrm) == 2 && a.count(EnemyKind::DuneRaider) == 1);
  }
  {
    Arena a;
    a.level.spawner.spawn(EnemyKind::Hyena, a.p.x + 150, a.p.y);
    a.run(1);
    assert(a.count(EnemyKind::Hyena) == 6);
    WildFoe *hyena = a.add(EnemyKind::Hyena, -100, 0);
    a.run(1);
    hyena->hp = hyena->maxHp * 0.25f;
    a.run(1);
    assert(hyena->fled && hyena->status.afraid());
  }
  {
    Arena a;
    a.add(EnemyKind::SaltGolem, 120, 0);
    a.run(120);
    assert(a.particles<FrostPatch>() > 0);
  }
  {
    Arena a;
    WildFoe *spitter = a.add(EnemyKind::GlassSpitter, 100, 0);
    a.run(1);
    int orbs = a.orbs();
    spitter->casts->fn(*spitter);
    a.run(1);
    assert(a.orbs() == orbs + 3);
    Screen &view = a.game.screen;
    view.xOffset = int(a.p.x) - view.vw / 2, view.yOffset = int(a.p.y) - view.vh / 2;
    a.level.scatter(view);
    auto solid = [&](double x, double y) {
      double cx = x, cy = y;
      a.level.collide(cx, cy, 2);
      return cx != x || cy != y;
    };
    bool wall = false;
    int px = int(a.p.x / 16), py = int(a.p.y / 16);
    for (int ty = py - 16; ty <= py + 16 && !wall; ty++)
      for (int tx = px - 28; tx <= px + 28 && !wall; tx++)
        if (double xc = tx * 16 + 8.0, yc = ty * 16 + 13.0;
            solid(xc, yc) && !solid(xc - 30, yc) && !solid(xc - 16, yc)) {
          Orb *o = a.level.add<Orb>(xc - 30, yc - 7, 60, 0, 1);
          o->bounces = 1;
          a.run(40);
          assert(o->xa < 0 && o->bounces == 0);
          wall = true;
        }
    assert(wall);
  }
  {
    Arena a;
    WildFoe *kite = a.add(EnemyKind::StormKite, 100, 0);
    a.run(1);
    int blasts = a.particles<Blast>();
    kite->casts->fn(*kite);
    a.run(1);
    assert(a.particles<Blast>() == blasts + 1);
  }
  {
    Arena a;
    WildFoe *raider = a.add(EnemyKind::DuneRaider, 120, 0);
    a.run(1);
    int arrows = a.hostileArrows();
    raider->casts->fn(*raider);
    a.run(1);
    assert(a.hostileArrows() == arrows + 1 && raider->castCount == 0);
  }
  {
    Arena a;
    WildFoe *shaman = a.add(EnemyKind::TotemShaman, 160, 0);
    a.run(1);
    for (int i = 0; i < 3; i++) {
      shaman->casts->fn(*shaman);
      a.run(2);
    }
    assert(a.count(EnemyKind::WarTotem) == 2);
    WildFoe *ally = a.add(EnemyKind::StranglerVine, 160, 20);
    bool hasted = false, warded = false;
    for (int i = 0; i < 300; i++) {
      a.run(1);
      hasted |= ally->frenzy > 0;
      warded |= ally->traits.ward > 0;
    }
    assert(hasted && warded);
  }
  std::cout << "[TEST] Scorched foes: the wyrm erupts three times under you, then surfaces "
               "soft and only one hunts at a time; hyenas run in sixes and flee when hurt; salt "
               "golems leave crust; glass ricochets off walls; kites call lightning; raiders "
               "throw one spear; shamans plant two totems that haste and ward: PASS"
            << std::endl;
}

void testBurningFoes() {
  {
    Arena a;
    a.add(EnemyKind::MagmaSlug, 120, 0);
    a.run(120);
    int burning = 0;
    for (auto &q : a.level.particles)
      if (auto *f = dynamic_cast<FrostPatch *>(q.get()); f && !f->removed)
        burning += f->kind == StatusKind::Burn && f->maxLife == 4;
    assert(burning > 0);
  }
  {
    Arena a;
    a.p.luck = -1;
    WildFoe *wraith = a.add(EnemyKind::AshWraith, 150, 0);
    a.run(1);
    assert(wraith->veiled());
    float hp = wraith->hp;
    wraith->hurt(10, 1, 0, 0);
    assert(std::abs(hp - wraith->hp - 5) < 0.01f);
    a.p.x = wraith->x - 20, a.p.y = wraith->y;
    assert(!wraith->veiled());
    wraith->bit(a.p);
    assert(a.p.status.power(StatusKind::Burn) > 0);
  }
  {
    Arena a;
    WildFoe *horror = a.add(EnemyKind::CaveHorror, 100, 0);
    WildFoe *far = a.add(EnemyKind::CaveHorror, -300, 0);
    a.run(120);
    assert(horror->buried && far->buried);
    WildFoe *vine = a.add(EnemyKind::StranglerVine, 100, 30);
    a.run(1);
    vine->hurt(1, 1, 0, 0);
    a.run(2);
    assert(!horror->buried && far->buried && horror->frenzy > 0);
  }
  {
    Arena a;
    WildFoe *star = a.add(EnemyKind::StarSpawn, 100, 0);
    a.run(1);
    star->hurt(star->maxHp * 10, 1, 0, 0);
    a.run(2);
    assert(a.count(EnemyKind::StarSpawn) == 0 && a.count(EnemyKind::Starlet) == 3);
  }
  {
    Arena a;
    WildFoe *cultist = a.add(EnemyKind::FlameCultist, 120, 0);
    a.run(1);
    int blasts = a.particles<Blast>();
    cultist->casts->fn(*cultist);
    a.run(1);
    assert(a.particles<Blast>() == blasts + 1);
  }
  {
    Arena a;
    WildFoe *sentinel = a.add(EnemyKind::HollowSentinel, 140, 0);
    a.run(60);
    assert(sentinel->xMark > 0);
    a.run(200);
    assert(sentinel->xMark < -0.9);
  }
  {
    Arena a;
    a.p.luck = -1;
    WildFoe *drake = a.add(EnemyKind::EmberDrake, 90, 0);
    a.run(1);
    int breath = 0;
    for (int i = 0; i < 700 && drake->restTime <= 0; i++) {
      a.run(1);
      breath = std::max(breath, a.orbs());
    }
    assert(drake->restTime > 0 && breath >= 7);
    double x = drake->x, y = drake->y;
    a.run(60);
    assert(drake->x == x && drake->y == y && drake->z == 0);
    float hp = drake->hp;
    drake->hurt(10, 1, 0, 0);
    assert(std::abs(hp - drake->hp - 15) < 0.01f);
    a.run(200);
    assert(drake->restTime <= 0);
    a.level.spawner.spawn(EnemyKind::EmberDrake, a.p.x + 200, a.p.y);
    a.level.spawner.spawn(EnemyKind::EmberDrake, a.p.x - 200, a.p.y);
    a.run(1);
    assert(a.count(EnemyKind::EmberDrake) == 2 && a.count(EnemyKind::AshWraith) == 1);
  }
  std::cout << "[TEST] Burning foes: slugs leave burning trails, ash wraiths hide until close and "
               "burn, cave horrors wake only to nearby fighting, star spawn split in three, "
               "cultists call meteors, sentinels turn slowly, the drake breathes fire and lands "
               "soft, one mini-boss at a time: PASS"
            << std::endl;
}

void testWanderingFoes() {
  {
    Arena a;
    WildFoe *stalker = a.add(EnemyKind::NightStalker, 120, 0);
    a.run(30);
    assert(a.level.get<Enemy>(stalker->id));
    a.level.night.dawn = true;
    a.run(2);
    assert(a.count(EnemyKind::NightStalker) == 0);
  }
  {
    Arena a;
    a.level.rules.loot = true;
    WildFoe *robber = a.add(EnemyKind::GraveRobber, 150, 0);
    a.run(1);
    int bones = a.count(EnemyKind::Skeleton);
    robber->casts->fn(*robber);
    a.run(1);
    assert(a.count(EnemyKind::Skeleton) == bones + 1);
    int gold = a.drops(DropKind::Gold);
    robber->hurt(robber->maxHp * 10, 1, 0, 0);
    a.run(2);
    assert(a.drops(DropKind::Gold) > gold);
  }
  {
    Arena a;
    a.level.spawner.spawn(EnemyKind::CaveBat, a.p.x + 150, a.p.y);
    a.run(1);
    assert(a.count(EnemyKind::CaveBat) == 12);
  }
  {
    Arena a;
    WildFoe *crawler = a.add(EnemyKind::RockCrawler, 200, 0);
    a.run(30);
    assert(crawler->buried);
    a.p.x = crawler->x - 45, a.p.y = crawler->y;
    bool flew = false;
    for (int i = 0; i < 120 && !flew; i++)
      a.run(1), flew = crawler->z > 4;
    assert(!crawler->buried && flew);
  }
  {
    Arena a;
    a.level.rules.loot = true;
    WildFoe *golem = a.add(EnemyKind::OreGolem, 150, 0);
    a.run(1);
    golem->hurt(golem->maxHp * 10, 1, 0, 0);
    a.run(2);
    assert(a.drops(DropKind::Material) >= 2);
  }
  {
    Arena a;
    a.level.rules.loot = true;
    a.p.materials[int(Mat::Ore)] = 3;
    WildFoe *kobold = a.add(EnemyKind::KoboldMiner, 150, 0);
    a.run(1);
    int arrows = a.hostileArrows();
    kobold->casts->fn(*kobold);
    a.run(1);
    assert(a.hostileArrows() == arrows + 1);
    kobold->bit(a.p);
    kobold->bit(a.p);
    assert(a.p.materials[int(Mat::Ore)] == 2 && kobold->stolen == 1 && kobold->status.afraid());
    int ore = a.drops(DropKind::Material);
    kobold->hurt(kobold->maxHp * 10, 1, 0, 0);
    a.run(2);
    assert(a.drops(DropKind::Material) >= ore + 1);
  }
  std::cout << "[TEST] Wandering foes: night stalkers fade at dawn, grave robbers dig up "
               "skeletons and drop gold, cave bats swarm in twelves, rock crawlers lie still "
               "then drop on you, ore golems drop ore, kobolds steal ore, run and give it back "
               "when slain: PASS"
            << std::endl;
}

void testNamedPacks() {
  for (int i = 0; i < NAMED_COUNT; i++) {
    Arena a;
    a.level.rules.loot = true;
    a.level.time = 600;
    const NamedPack &n = NAMED[i];
    int before = a.count(n.minion);
    Enemy *leader = named::raise(a.level, i, a.p.x + 160, a.p.y);
    assert(leader && a.level.namedRaised >> i & 1);
    a.run(1);
    uint32_t want = 1u << int(n.traits[0]) | 1u << int(n.traits[1]);
    assert(leader->title == n.name && leader->pack == &n && leader->elite);
    auto &ids = a.level.bossIds;
    assert(std::find(ids.begin(), ids.end(), leader->id) != ids.end());
    assert(a.level.get<Enemy>(leader->id) == leader);
    assert(leader->traits.bits == want);
    assert(a.count(n.minion) - (n.minion == n.base) >= before + n.count);
    leader->hurt(leader->maxHp * 100, 1, 0, 0);
    a.run(2);
    bool rare = false;
    for (auto [id, o] : a.level.reg.view<Owned>().each())
      if (auto *d = dynamic_cast<ItemDrop *>(o.self.get());
          d && d->drop.kind == DropKind::Item && int(d->drop.item.rarity) >= int(Rarity::Rare))
        rare = true;
    assert(rare);
  }
  {
    Arena a;
    a.level.time = 600;
    int home = -1;
    double sx = a.p.x, sy = a.p.y;
    for (int r = 0; r < 4000 && home < 0; r += 64)
      for (int k = 0; k < 16 && home < 0; k++) {
        double x = sx + std::cos(TAU * k / 16) * r, y = sy + std::sin(TAU * k / 16) * r;
        for (int i = 0; i < NAMED_COUNT; i++)
          if (NAMED[i].home == world::biomeUnder(x, y))
            home = i, a.p.x = x, a.p.y = y;
      }
    assert(home >= 0);
    named::tick(a.level);
    assert(a.level.namedRaised == 1u << home);
    a.run(1);
    int leaders = 0;
    for (auto [id, o] : a.level.reg.view<Owned>().each())
      if (auto *e = dynamic_cast<Enemy *>(o.self.get()); e && e->pack)
        leaders++;
    a.level.namedScan = 0;
    named::tick(a.level);
    a.run(1);
    int again = 0;
    for (auto [id, o] : a.level.reg.view<Owned>().each())
      if (auto *e = dynamic_cast<Enemy *>(o.self.get()); e && e->pack)
        again++;
    assert(leaders == 1 && again == 1);
  }
  std::cout << "[TEST] Named packs: all ten rise with their own name, exactly their two traits "
               "and their minions, drop a rare when slain, and each rises once in its home "
               "biome: PASS"
            << std::endl;
}
