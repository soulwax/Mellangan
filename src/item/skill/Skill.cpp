#include "item/skill/Skill.hpp"

#include "audio/Sound.hpp"
#include "entity/Ability.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "hero/Hero.hpp"
#include "item/weapon/Recipe.hpp"
#include "level/Level.hpp"
#include "particle/Drift.hpp"
#include "particle/Flourish.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"

Skill::Skill(const char *name, Sprite icon, const char *desc,
             const char *owner, float cooldown)
    : Upgrade(name, icon, {desc, desc, desc, desc, desc}, false),
      cooldown(cooldown) {
  this->owner = owner;
  skill = true;
}

void Skill::levelUp(Mob &m) {
  Player *p = m.asPlayer();
  if (!p) {
    level = 1;
    return;
  }
  if (p->skill == this)
    return;
  if (p->skill)
    p->skill->unequip(*p);
  p->skill = this;
  p->skillTime = 0;
}

HeroSkill::HeroSkill(const Hero &hero)
    : Skill(hero.skill, hero.sprite(), hero.skillDesc, hero.name,
            hero.cooldown),
      hero(hero) {}

void HeroSkill::use(Mob &p) { hero.cast(p); }

namespace {
constexpr Color ARCANE{0.72f, 0.45f, 1, 1}, FROST{0.6f, 0.85f, 1, 1};

const Recipe MISSILE{"Arcane Missiles", "MAGE", "", "missile", 0,
                     Form::Seeker, pay::Mark, 9, 0, 240, 170, 10, ARCANE,
                     atlas::pt::ArcaneStar};
const Recipe SINGULARITY{"Singularity", "MAGE", "", "rift", 0, Form::Field,
                         pay::Pull, 6, 0, 80, 0, 1, {0.5f, 0.25f, 0.95f, 1},
                         atlas::pt::Wisp};

class TimeWarp : public Skill {
public:
  TimeWarp()
      : Skill("TIME WARP", atlas::arsenal(192),
              "Slow every foe within 170 to a crawl for 3.5s", "MAGE", 12) {}
  void use(Mob &p) override {
    p.level->foes(p, p.x, p.y, 190, [&](Mob &e) {
      double xd = e.x - p.x, yd = e.y - p.y;
      if (xd * xd + yd * yd < 170 * 170)
        e.afflict(StatusKind::Chill, 3.5f, 0.6f, this);
    });
    p.level->add<Shockwave>(p.x, p.bodyY(), 6, 170, 0.6f, FROST);
    p.level->add<Shockwave>(p.x, p.bodyY(), 4, 130, 0.9f, ARCANE.alpha(0.6f),
                            false);
    flourish::sigil(*p.level, p.x, p.bodyY(), 60, FROST, 12);
    for (int i = 0; i < 16 && Drift::room(); i++) {
      float a = rng.range(0, TAU), r = rng.range(30, 170);
      Drift *d = p.level->add<Drift>(p.x + std::cos(a) * r,
                                     p.bodyY() + std::sin(a) * r, 0, -6,
                                     atlas::pt::Snowflake, FROST.alpha(0.8f),
                                     rng.range(0.8f, 1.4f));
      d->spin = 1, d->scale = 0.7f;
    }
    Sound::nova.play(0.5f, 0);
  }
};

class ManaSurge : public Skill {
public:
  ManaSurge()
      : Skill("MANA SURGE", atlas::arsenal(193),
              "Weapons fire 35% faster for 5s", "MAGE", 14) {}
  void use(Mob &p) override {
    unequip(p);
    applied = std::max(0.0f, std::min(0.35f, p.haste - 0.2f));
    p.haste -= applied, time = 5;
    Spark::burst(*p.level, p.x, p.bodyY(), ARCANE, 20, 120, true);
    flourish::sigil(*p.level, p.x, p.bodyY(), 40, ARCANE, 6);
    Sound::cast.play(0.6f, 0);
  }
  void active(Mob &p) override {
    if (time <= 0)
      return;
    time -= DT;
    if (rng.chance(0.4f) && Spark::alive < 600) {
      float a = rng.range(0, TAU);
      p.level->add<Spark>(p.x + std::cos(a) * 8, p.bodyY() + std::sin(a) * 8,
                          std::cos(a) * 20, -rng.range(20, 40), ARCANE, 0.4f,
                          0.5f, true);
    }
    if (time <= 0)
      unequip(p);
  }
  void unequip(Mob &p) override {
    p.haste += applied;
    applied = 0, time = 0;
  }

private:
  float time = 0, applied = 0;
};

class ArcaneMissiles : public Skill {
public:
  ArcaneMissiles()
      : Skill("ARCANE MISSILES", atlas::arsenal(194),
              "Loose ten homing missiles that mark foes", "MAGE", 8) {}
  void use(Mob &p) override {
    fireRecipe(MISSILE, p, this, MISSILE.damage * p.might, MISSILE.reach,
               MISSILE.count);
    Spark::burst(*p.level, p.x, p.bodyY(), ARCANE, 12, 100, true);
    Sound::cast.play(0.5f, 0);
  }
};

class Singularity : public Skill {
public:
  Singularity()
      : Skill("SINGULARITY", atlas::arsenal(195),
              "Open a rift ahead that pulls foes in, then collapses", "MAGE",
              16) {}
  void use(Mob &p) override {
    double x = p.x + p.xAim * 90, y = p.bodyY() + p.yAim * 90;
    RecipeShot *s = p.level->add<RecipeShot>(
        SINGULARITY, x, y, 1, 0, SINGULARITY.damage * p.might,
        SINGULARITY.reach);
    s->life = 2.5f, s->burstOnEnd = true, s->source = this;
    flourish::streak(*p.level, p.x, p.bodyY(), x, y, atlas::pt::Wisp,
                     SINGULARITY.tint, 6);
    Sound::orb.play(0.6f, 0);
  }
};
const Recipe SCRAP{"Scrap Barrage", "TINKERER", "", "bomb", 0, Form::Lob,
                   pay::Burn | pay::Stagger, 20, 0, 220, 0, 6, {1, 0.7f, 0.35f, 1},
                   atlas::pt::Pebble};
const Recipe WHIRLPOOL{"Whirlpool", "CORSAIR", "", "tide", 0, Form::Field,
                       pay::Pull | pay::Chill, 7, 0, 70, 0, 1,
                       {0.4f, 0.85f, 0.9f, 1}, atlas::pt::Bubble};

struct Art {
  const char *name, *owner, *desc;
  int icon;
  float cooldown;
  void (*use)(Mob &);
};

const Art ARTS[] = {
    {"ARROW STORM", "RANGER", "Loose sixteen arrows in every direction", 196, 9,
     [](Mob &p) { ability::volley(p, 16, TAU / 16, 230, 16, 2); }},
    {"SHIELD CHARGE", "KNIGHT", "Charge ahead, battering every foe in your path",
     197, 7, [](Mob &p) { ability::charge(p, 110, 40); }},
    {"MIASMA", "ALCHEMIST", "Poison every foe within 120 for 5s", 198, 10,
     [](Mob &p) { ability::miasma(p, 120, 10, 5); }},
    {"CHAIN STORM", "STORMCALLER", "Lightning leaps between eight nearby foes",
     199, 7, [](Mob &p) { ability::chain(p, 8, 260, 34); }},
    {"DEATH GRIP", "REAPER", "Drag foes within 140 to you and chill them", 200,
     9, [](Mob &p) { ability::grasp(p, 140, 22); }},
    {"SACRED LIGHT", "PRIESTESS", "Call pillars of light down on six foes", 201,
     8, [](Mob &p) { ability::smite(p, 6, 42); }},
    {"LUNGE", "DUELIST", "Lunge forward with a long piercing thrust", 202, 5,
     [](Mob &p) { ability::lunge(p, 100, 55); }},
    {"BRAMBLES", "WARDEN", "Root foes within 110 in thorns for 2.5s", 203, 10,
     [](Mob &p) { ability::entangle(p, 110, 16); }},
    {"MINEFIELD", "TINKERER", "Scatter eight mines in a ring around you", 204,
     12, [](Mob &p) { ability::mines(p, 8, 32); }},
    {"COMET", "ORACLE", "Call three comets down on the nearest foes", 205, 11,
     [](Mob &p) { ability::comet(p, 3, 60); }},
    {"POWDER KEG", "CORSAIR", "Lob a keg that bursts into a wide fire", 206, 9,
     [](Mob &p) { ability::keg(p, 70); }},
    {"WAR BANNER", "KNIGHT", "Plant a banner: heal 4/s and slow foes near it for 8s",
     215, 16, [](Mob &p) { ability::banner(p, 90, 8); }},
    {"HUNTER'S MARK", "RANGER", "Mark six foes and loose an arrow at each", 216, 8,
     [](Mob &p) { ability::hunt(p, 6, 26); }},
    {"FLASK BARRAGE", "ALCHEMIST", "Lob five fire flasks at the nearest foes", 217,
     10, [](Mob &p) { ability::barrage(p, 5, 24); }},
    {"TEMPEST", "STORMCALLER", "Lightning strikes foes around you for 5s", 218, 14,
     [](Mob &p) { ability::tempest(p, 5, 22); }},
    {"BLOOD PACT", "REAPER", "Strike foes within 110 and heal 3 for each", 219, 10,
     [](Mob &p) { ability::drain(p, 110, 30); }},
    {"HALLOWED GROUND", "PRIESTESS", "Hallow the ground: heal 6/s and burn foes for 6s",
     220, 16, [](Mob &p) { ability::sanctuary(p, 90, 6); }},
    {"FLASH STEP", "DUELIST", "Flash between four foes, striking and marking each",
     221, 7, [](Mob &p) { ability::flash(p, 4, 45); }},
    {"OVERGROWTH", "WARDEN", "Wild growth poisons and slows foes for 7s", 222, 14,
     [](Mob &p) { ability::overgrowth(p, 100, 7); }},
    {"SCRAP BARRAGE", "TINKERER", "Lob six scrap bombs that burn and stagger", 223,
     9, [](Mob &p) {
       fireRecipe(SCRAP, p, nullptr, SCRAP.damage * p.power(), SCRAP.reach,
                  SCRAP.count);
     }},
    {"ECLIPSE", "ORACLE", "A shadow follows you, marking and slowing foes for 6s",
     224, 15, [](Mob &p) { ability::eclipse(p, 110, 6); }},
    {"WHIRLPOOL", "CORSAIR", "Open a whirlpool that drags foes in and chills them",
     225, 12, [](Mob &p) {
       fireRecipe(WHIRLPOOL, p, nullptr, WHIRLPOOL.damage * p.power(),
                  WHIRLPOOL.reach, WHIRLPOOL.count);
     }},
    {"WAR CRY", nullptr, "Hurl back foes within 130; they take 30% more damage for 5s",
     207, 10, [](Mob &p) { ability::warcry(p, 130, 5); }},
    {"BANDAGE", nullptr, "Heal 30 and shake off burns, poison and chill", 208,
     14, [](Mob &p) { ability::bandage(p, 30); }},
    {"TUMBLE", nullptr, "Roll ahead, untouchable for a moment", 209, 4,
     [](Mob &p) { ability::tumble(p, 70); }},
    {"WHIRLWIND", nullptr, "Spin, cutting every foe within 64", 210, 6,
     [](Mob &p) { ability::whirlwind(p, 64, 40); }},
    {"GROUND SLAM", nullptr, "Shake the earth: damage and stun foes within 100",
     211, 9, [](Mob &p) { ability::quake(p, 100, 35); }},
    {"FIRE RING", nullptr, "Ring yourself in flame for 4s", 212, 12,
     [](Mob &p) { ability::firering(p, 8, 14); }},
    {"LODESTAR", nullptr, "Draw in every gem within 420", 213, 20,
     [](Mob &p) { ability::lodestar(p, 420); }},
    {"SNARE NET", nullptr, "Cast a net that binds foes ahead for 3s", 214, 8,
     [](Mob &p) { ability::snare(p, 120, 10); }},
};

class ArtSkill : public Skill {
public:
  explicit ArtSkill(const Art &art)
      : Skill(art.name, atlas::arsenal(art.icon), art.desc, art.owner,
              art.cooldown),
        art(art) {}
  void use(Mob &p) override { art.use(p); }

  const Art &art;
};
} // namespace

void addSkills(std::vector<std::unique_ptr<Upgrade>> &u) {
  for (const Hero *h : HEROES)
    u.push_back(std::make_unique<HeroSkill>(*h));
  u.push_back(std::make_unique<TimeWarp>());
  u.push_back(std::make_unique<ManaSurge>());
  u.push_back(std::make_unique<ArcaneMissiles>());
  u.push_back(std::make_unique<Singularity>());
  for (const Art &a : ARTS)
    u.push_back(std::make_unique<ArtSkill>(a));
}
