#undef NDEBUG
#include <cassert>

#include <cstring>
#include <iostream>

#include "test/Tests.hpp"

#include "Game.hpp"
#include "entity/Ability.hpp"
#include "entity/Ally.hpp"
#include "entity/Player.hpp"
#include "entity/pickup/Gem.hpp"
#include "item/Upgrade.hpp"
#include "item/talent/Talents.hpp"
#include "item/loot/Loot.hpp"
#include "item/loot/Unique.hpp"
#include "item/relic/WorldRelics.hpp"
#include "item/weapon/Recipe.hpp"
#include "level/World.hpp"
#include "level/Level.hpp"

namespace {

struct Stage {
  Game game;
  Level &level;
  Player &p;

  Stage() : level(*(game.level = std::make_unique<Level>(game, quiet()))), p(*level.player) {
    for (auto &up : p.upgrades)
      up->level = 0;
    p.luck = -1, p.regen = 0;
  }
  ~Stage() { game.level.reset(); }

  static RunRules quiet() {
    RunRules r;
    r.spawnRate = 0, r.waves = false, r.bosses = false;
    return r;
  }

  Enemy *foe(EnemyKind kind, double dx, double dy, float hp = 30) {
    auto made = Enemy::create(kind, hp, 1);
    Enemy *e = made.get();
    e->x = p.x + dx, e->y = p.y + dy, e->aware = false;
    level.add(std::move(made));
    return e;
  }

  void run(float seconds) {
    for (int i = 0; i < int(seconds / DT); i++)
      level.tick();
  }
};

bool near(float a, float b) { return std::abs(a - b) < 1e-3f; }

} // namespace

void testSupportArts() {
  {
    Stage s;
    float hp = s.p.hp, might = s.p.might, speed = s.p.speed, haste = s.p.haste;
    ability::rite(s.p, 0.15f, 0.3f, 6);
    s.run(0.1f);
    assert(near(s.p.hp, hp - s.p.maxHp * 0.15f) && near(s.p.might, might + 0.3f));
    ability::sprint(s.p, 3);
    s.run(0.1f);
    assert(near(s.p.speed, speed * 1.5f));
    ability::overclock(s.p, 4, 2);
    s.run(0.1f);
    assert(near(s.p.haste, haste * 0.5f));
    s.run(4.2f);
    assert(near(s.p.haste, haste + 1) && near(s.p.speed, speed));
    s.run(2.2f);
    assert(near(s.p.haste, haste) && near(s.p.might, might));
  }
  {
    Stage s;
    summon(s.level, AllyKind::Wolf, s.p.x + 20, s.p.y, 10, 30);
    s.run(0.05f);
    ability::mirror(s.p, 4);
    ability::horn(s.p, 140, 6);
    s.run(0.1f);
    assert(s.level.allies.size() == 2);
    for (Ally *a : s.level.allies)
      if (&a->def == &allyDef(AllyKind::Wolf))
        assert(near(a->haste, 0.7f));
  }
  {
    Stage s;
    s.p.hp = s.p.maxHp * 0.2f;
    ability::stand(s.p, 80, 20);
    float hp = s.p.hp;
    s.p.hurt(20);
    assert(s.p.hp == hp && s.p.invuln > 3);
    s.run(4.5f);
    assert(s.p.hp > hp + 20);
  }
  {
    Stage s;
    Enemy *knight = s.foe(EnemyKind::Knight, 40, 0);
    s.run(0.05f);
    float hp = s.p.hp, foeHp = knight->hp;
    ability::parry(s.p, 1);
    foeHp = knight->hp;
    s.p.invuln = 0;
    s.p.hurt(15);
    assert(s.p.hp == hp && knight->hp < foeHp - 25 && s.p.parry == 0);

    ability::foresight(s.p, 3);
    assert(s.p.sure == 3);
    for (int i = 0; i < 3; i++)
      knight->hurt(1, 0, 0, 0);
    assert(s.p.sure == 0);

    ability::mercy(s.p, 3);
    assert(s.p.traits.ward >= 3);
  }
  {
    Stage s;
    ability::transmute(s.p, 110, 6);
    Enemy *rat = s.foe(EnemyKind::Rat, 30, 0, 1);
    s.run(0.05f);
    size_t gems = s.level.count<Gem>();
    rat->hurt(1e6f, 0, 0, 0);
    s.run(0.05f);
    assert(s.level.count<Gem>() == gems + 2);
    s.run(6.5f);
    Enemy *late = s.foe(EnemyKind::Rat, 30, 0, 1);
    s.run(0.05f);
    gems = s.level.count<Gem>();
    late->hurt(1e6f, 0, 0, 0);
    s.run(0.05f);
    assert(s.level.count<Gem>() == gems + 1);
  }
  std::cout << "[TEST] Support skills: rites, sprints and overclocks restore, horns rally allies, "
               "last stands, parries, foresight, halos and gilded circles: PASS"
            << std::endl;
}

void testTalents() {
  auto rank = [](Stage &s, const char *name, int ranks) {
    Upgrade *u = s.p.upgrade(name);
    assert(u && !u->weapon);
    for (int i = 0; i < ranks; i++)
      u->levelUp(s.p);
    return u;
  };
  {
    Stage s;
    int general = 0;
    for (auto &u : s.p.upgrades)
      general += dynamic_cast<Talent *>(u.get()) && !u->owner;
    assert(general == 20);
    float might = s.p.might, maxHp = s.p.maxHp;
    rank(s, "Glass Cannon", 2);
    assert(near(s.p.might, might + 0.24f) && near(s.p.maxHp, maxHp - 12));
    rank(s, "Volley Master", 5);
    assert(near(s.p.bonus.shots, 2));
    rank(s, "Hunter's Instinct", 1);
    assert(near(s.p.bonus.eliteDamage, 6) && near(s.p.bonus.bossDamage, 6));

    might = s.p.might;
    Upgrade *momentum = rank(s, "Momentum", 5);
    s.p.status.moving = true;
    for (int i = 0; i < int(2.1f / DT); i++)
      momentum->tick(s.p);
    assert(near(s.p.might, might + 0.2f));
    s.p.status.moving = false;
    momentum->tick(s.p);
    assert(near(s.p.might, might));

    float armor = s.p.armor;
    Upgrade *rooted = rank(s, "Rooted", 2);
    for (int i = 0; i < int(1.1f / DT); i++)
      rooted->tick(s.p);
    assert(near(s.p.armor, armor + 1));
    s.p.status.moving = true;
    rooted->tick(s.p);
    assert(near(s.p.armor, armor));

    float haste = s.p.haste;
    Upgrade *flow = rank(s, "Flow", 1);
    s.p.crits += 3;
    flow->tick(s.p);
    assert(near(s.p.haste, haste - 0.06f));
    for (int i = 0; i < int(3.1f / DT); i++)
      flow->tick(s.p);
    assert(near(s.p.haste, haste));
  }
  {
    Stage s;
    Enemy *a = s.foe(EnemyKind::Knight, 60, 0), *b = s.foe(EnemyKind::Knight, -60, 0);
    s.run(0.05f);
    rank(s, "Cold Blood", 5);
    b->afflict(StatusKind::Chill, 5, 0.1f);
    float ha = a->hp, hb = b->hp;
    a->hurt(10, 0, 0, 0), b->hurt(10, 0, 0, 0);
    assert(near((hb - b->hp) / (ha - a->hp), 1.4f));

    rank(s, "Spellweave", 5);
    a->afflict(StatusKind::Burn, 2, 1);
    assert(near(a->status.effects[int(StatusKind::Burn)].time, 3));

    rank(s, "Vengeance", 2);
    rank(s, "Thick Skin", 5);
    ha = a->hp;
    float hp = s.p.hp;
    s.p.hurt(20);
    float first = hp - s.p.hp;
    assert(a->hp < ha);
    s.p.invuln = 0, hp = s.p.hp;
    s.p.hurt(20);
    assert(first < hp - s.p.hp);

    rank(s, "Overflow", 1);
    s.p.hp = s.p.maxHp;
    s.p.mend(50);
    assert(near(s.p.guard, 10));
    hp = s.p.hp;
    s.p.invuln = 0, s.p.skinTime = 1;
    s.p.hurt(5);
    assert(s.p.hp == hp && s.p.guard < 10);

    Upgrade *hoard = rank(s, "Soul Hoard", 1);
    float both = a->hp + b->hp;
    for (int i = 0; i < 20; i++)
      hoard->slay(s.p, *a);
    s.run(0.6f);
    assert(a->hp + b->hp < both);
  }
  {
    Stage s;
    rank(s, "Bloodthirst", 2);
    rank(s, "Spare Parts", 2);
    s.p.hp = 50;
    Enemy *rat = s.foe(EnemyKind::Rat, 40, 0, 1);
    s.run(0.05f);
    rat->hurt(1e6f, 0, 0, 0);
    assert(near(s.p.hp, 51));
    Ally *wolf = summon(s.level, AllyKind::Wolf, s.p.x, s.p.y, 1, 10);
    assert(near(wolf->life, 13));
  }
  std::cout << "[TEST] Talents: ranks raise their stats, momentum, roots and flow come and go, "
               "cold blood, spellweave, vengeance, thick skin, overflow, soul hoard, "
               "bloodthirst and spare parts: PASS"
            << std::endl;
}

void testRelicsAndUniques() {
  {
    Stage s;
    int shown = 0, hidden = 0;
    for (auto &u : s.p.upgrades)
      if (dynamic_cast<WorldRelic *>(u.get()))
        (u->hidden ? hidden : shown)++, assert(u->rare);
    assert(shown == 16 && hidden == 8);

    Enemy *knight = s.foe(EnemyKind::Knight, 50, 0);
    s.run(0.05f);
    s.p.sure = 1;
    float hp = knight->hp;
    knight->hurt(10, 0, 0, 0);
    float plain = hp - knight->hp;
    for (int i = 0; i < 5; i++)
      s.p.upgrade("Keen Edge")->levelUp(s.p);
    s.p.sure = 1, hp = knight->hp;
    knight->hurt(10, 0, 0, 0);
    assert(near((hp - knight->hp) / plain, 1.4f));

    s.p.upgrade("Fleet Heart")->levelUp(s.p);
    s.p.invuln = 0;
    s.p.hurt(5);
    assert(near(s.p.invuln, s.level.game.balance.invuln + 0.3f));

    s.p.upgrade("Ember Heart")->levelUp(s.p);
    knight->afflict(StatusKind::Burn, 2, 1);
    assert(near(knight->status.power(StatusKind::Burn), 1.1f));

    s.p.upgrade("Summoner's Seal")->levelUp(s.p);
    Recipe totem{"Wolf Totem", nullptr, "", "wolf", 0, Form::Summon, 0, 1, 3, 0, 0, 2,
                 col::white, atlas::pt::Rune};
    fireRecipe(totem, s.p, nullptr, 1, 0, 2);
    s.run(0.05f);
    assert(s.level.allies.size() == 3);
  }
  {
    Stage s;
    Upgrade *globe = s.p.upgrade("Snow Globe");
    globe->levelUp(s.p);
    int tx, ty;
    assert(world::findBiome(BIOME_SNOW, tx, ty));
    float armor = s.p.armor;
    double home = s.p.x, homeY = s.p.y;
    s.p.x = tx * 16 + 8, s.p.y = ty * 16 + 8;
    globe->tick(s.p);
    assert(near(s.p.armor, armor + 2));
    s.p.x = home, s.p.y = homeY;
    globe->tick(s.p);
    assert(near(s.p.armor, armor));

    Upgrade *crown = s.p.upgrade("Hollow Crown");
    crown->levelUp(s.p);
    float might = s.p.might;
    Enemy *champ = s.foe(EnemyKind::Knight, 300, 0);
    champ->title = "THE TEST";
    crown->slay(s.p, *champ);
    crown->slay(s.p, *champ);
    assert(near(s.p.might, might + 0.06f));
    crown->forget(s.p);
    assert(near(s.p.might, might));

    float night = s.level.rules.nightLength;
    s.p.upgrade("Dawnstone")->levelUp(s.p);
    assert(near(s.level.rules.nightLength, night - 30));

    Upgrade *staff = s.p.upgrade("Pilgrim's Staff");
    staff->levelUp(s.p);
    float speed = s.p.speed;
    s.level.landmarks = 3;
    staff->tick(s.p);
    assert(near(s.p.speed, speed + 2.22f));

    Upgrade *quill = s.p.upgrade("Cartographer's Quill");
    quill->levelUp(s.p);
    quill->tick(s.p);
    s.p.hp = 50;
    s.p.x += 16 * 22;
    quill->tick(s.p);
    assert(s.p.hp > 55);
  }
  {
    Stage s;
    for (int i = 0; i < UNIQUE_COUNT; i++) {
      Item item;
      forgeUnique(item, i);
      assert(item.base >= 0 && item.name() == UNIQUES[i].name && item.count > 0);
      Item back;
      assert(Item::load(item.save(), back) && back.name() == item.name());
      if (UNIQUES[i].grants)
        assert(s.p.upgrade(UNIQUES[i].grants));
    }
    auto wear = [&](const char *name) {
      for (int i = 0; i < UNIQUE_COUNT; i++)
        if (std::strcmp(UNIQUES[i].name, name) == 0) {
          Item item;
          forgeUnique(item, i);
          s.p.bag = {item};
          assert(s.p.equip(0));
          return item.gearSlot();
        }
      assert(false);
      return 0;
    };
    int revives = s.p.revives;
    wear("Ember Locket");
    assert(s.p.revives == revives + 1);

    int slot = wear("Pack Banner");
    s.run(0.1f);
    assert(s.level.allies.size() == 1);
    s.p.unequip(slot);
    s.run(0.1f);
    assert(s.level.allies.empty());

    wear("Last Light");
    s.p.hp = 5, s.p.invuln = 0;
    s.p.hurt(500);
    assert(s.p.hp == 1 && s.p.invuln > 2);
    s.p.invuln = 0;
    s.p.hurt(500);
    assert(s.p.dead() || s.p.revives < revives + 1);

    Rng rng;
    rng.state = 7;
    int uniques = 0;
    for (int i = 0; i < 4000; i++)
      uniques += rollItem(40, 500, false, rng, Rarity::Unique).rarity == Rarity::Unique;
    assert(uniques == 4000);
  }
  std::cout << "[TEST] Relics and uniques: passives sharpen crits, grace, burns and packs, "
               "relics react to biome, champions, landmarks and regions, uniques forge, save, "
               "grant and let go: PASS"
            << std::endl;
}
