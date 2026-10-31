#include "item/skill/Skill.hpp"

#include "audio/Sound.hpp"
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

void Skill::levelUp(Player &p) {
  if (p.skill == this)
    return;
  if (p.skill)
    p.skill->unequip(p);
  p.skill = this;
  p.skillTime = 0;
}

HeroSkill::HeroSkill(const Hero &hero)
    : Skill(hero.skill, hero.sprite(), hero.skillDesc, hero.name,
            hero.cooldown),
      hero(hero) {}

void HeroSkill::use(Player &p) { hero.cast(p); }

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
  void use(Player &p) override {
    p.level->enemiesNear(p.x, p.y, 190, [&](Enemy &e) {
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
  void use(Player &p) override {
    unequip(p);
    applied = std::max(0.0f, std::min(0.35f, p.haste - 0.2f));
    p.haste -= applied, time = 5;
    Spark::burst(*p.level, p.x, p.bodyY(), ARCANE, 20, 120, true);
    flourish::sigil(*p.level, p.x, p.bodyY(), 40, ARCANE, 6);
    Sound::cast.play(0.6f, 0);
  }
  void active(Player &p) override {
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
  void unequip(Player &p) override {
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
  void use(Player &p) override {
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
  void use(Player &p) override {
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
} // namespace

void addSkills(std::vector<std::unique_ptr<Upgrade>> &u) {
  for (const Hero *h : HEROES)
    u.push_back(std::make_unique<HeroSkill>(*h));
  u.push_back(std::make_unique<TimeWarp>());
  u.push_back(std::make_unique<ManaSurge>());
  u.push_back(std::make_unique<ArcaneMissiles>());
  u.push_back(std::make_unique<Singularity>());
}
