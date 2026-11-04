#include "item/talent/Talents.hpp"

#include "audio/Sound.hpp"
#include "entity/Ability.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "entity/pickup/Chest.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

namespace {

using S = ItemStats;

struct TalentDef {
  const char *name, *owner, *desc;
  float S::*stat;
  float rank;
  float S::*stat2;
  float rank2;
};

const TalentDef TALENTS[] = {
    {"Bloodthirst", nullptr, "Kills heal 0.5 per rank", &S::bloodthirst, 0.5f},
    {"Glass Cannon", nullptr, "+12% damage and -6 max health per rank"},
    {"Momentum", nullptr, "After 2s on the move, +4% damage per rank"},
    {"Cold Blood", nullptr, "+8% damage to chilled foes per rank", &S::coldBlood, 8},
    {"Pyromaniac", nullptr, "Burning foes may spread their fire on death, 10% per rank",
     &S::pyro, 10},
    {"Scavenger", nullptr, "+5% gem and gold value per rank", &S::scavenge, 5, &S::goldFind,
     5},
    {"Thick Skin", nullptr, "The first hit every 8s is 8% weaker per rank", &S::thickSkin, 8},
    {"Vengeance", nullptr, "When hurt, a nova deals 10 per rank", &S::vengeance, 10},
    {"Night Owl", nullptr, "+3% experience per lit shrine per rank", &S::nightOwl, 3},
    {"Hunter's Instinct", nullptr, "+6% damage to elites, champions and bosses per rank",
     &S::eliteDamage, 6, &S::bossDamage, 6},
    {"Overflow", nullptr, "20% of overhealing per rank becomes a shield", &S::overflow, 20},
    {"Afterimage", nullptr, "Getting hit leaves a decoy for 1s per rank", &S::afterimage, 1},
    {"Spellweave", "MAGE", "Statuses you inflict last 10% longer per rank", &S::weave, 10},
    {"Volley Master", "RANGER", "+1 projectile at ranks 3 and 5"},
    {"Juggernaut", "KNIGHT", "+15% knockback and longer stagger per rank", &S::juggernaut,
     15},
    {"Catalyst", "ALCHEMIST", "+8% poison and burn on marked foes per rank", &S::catalyst,
     8},
    {"Static Build", "STORMCALLER", "Every few hits chain lightning leaps free; faster per rank",
     &S::staticBuild, 1},
    {"Soul Hoard", "REAPER", "Kills store souls; 20 souls release a free reap"},
    {"Sanctified", "PRIESTESS", "Your heals burn foes within 60 for 5 per rank",
     &S::sanctified, 5},
    {"Flow", "DUELIST", "Crits stack 2% faster weapons per rank for 3s, up to 5"},
    {"Rooted", "WARDEN", "Standing still for 1s grants 0.5 armour per rank"},
    {"Spare Parts", "TINKERER", "Summons last 15% longer per rank", &S::spare, 15},
    {"Prophecy", "ORACLE", "Marked foes take 5% more damage per rank", &S::prophecy, 5},
    {"Plunderer", "CORSAIR", "Elites may drop an extra chest, 4% per rank"},
    {"Spellweave Thread", nullptr, "+15% status duration", &S::weave, 15},
    {"Wide Grip", nullptr, "+10% area", &S::area, 10},
    {"Swift Quiver", nullptr, "+12% projectile speed", &S::shotSpeed, 12},
    {"Keen Edge", nullptr, "+8% critical damage", &S::critDamage, 8},
    {"Fleet Heart", nullptr, "+0.3s untouchable after a hit", &S::grace, 0.3f},
    {"Ember Heart", nullptr, "+10% burn damage", &S::ember, 10},
    {"Collector", nullptr, "+10% gem value", &S::scavenge, 10},
    {"Summoner's Seal", nullptr, "+1 ally from every summon", &S::allyCap, 1},
};
static_assert(std::size(TALENTS) == size_t(TalentKind::Count), "every TalentKind needs a row");

constexpr Color SOUL{0.75f, 0.3f, 0.55f, 1};

} // namespace

Talent::Talent(TalentKind kind)
    : Upgrade(TALENTS[int(kind)].name,
              atlas::arsenal(kind < TalentKind::SpellweaveThread
                                 ? ICONS + int(kind)
                                 : PASSIVE_ICONS + int(kind) - int(TalentKind::SpellweaveThread)),
              {}, false),
      kind(kind) {
  const TalentDef &d = TALENTS[int(kind)];
  owner = d.owner;
  desc = {d.desc, d.desc, d.desc, d.desc, d.desc};
}

void Talent::levelUp(Mob &p) {
  Upgrade::levelUp(p);
  const TalentDef &d = TALENTS[int(kind)];
  if (d.stat)
    p.kit().*d.stat += d.rank;
  if (d.stat2)
    p.kit().*d.stat2 += d.rank2;
  if (kind == TalentKind::GlassCannon)
    p.might += 0.12f, p.maxHp -= 6, p.hp = std::min(p.hp, p.maxHp);
  if (kind == TalentKind::VolleyMaster && (level == 3 || level == 5))
    p.kit().shots += 1;
}

void Talent::hold(Mob &p, float Mob::*stat, float want) {
  p.*stat += want - applied;
  applied = want;
}

void Talent::tick(Mob &p) {
  switch (kind) {
  case TalentKind::Momentum:
    run = p.status.moving ? run + DT : 0;
    return hold(p, &Mob::might, run >= 2 ? 0.04f * level : 0);
  case TalentKind::Rooted:
    run = p.status.moving ? 0 : run + DT;
    return hold(p, &Mob::armor, run >= 1 ? 0.5f * level : 0);
  case TalentKind::Flow:
    if (Player *pl = p.asPlayer(); pl && pl->crits != seen) {
      stacks = std::min(5, stacks + std::max(0, pl->crits - seen));
      seen = pl->crits, flowTime = 3;
    }
    if ((flowTime -= DT) <= 0)
      stacks = 0;
    return hold(p, &Mob::haste, -0.02f * level * stacks);
  default:
    return;
  }
}

void Talent::slay(Mob &p, Enemy &e) {
  if (kind == TalentKind::SoulHoard && ++souls >= 20) {
    souls = 0;
    ability::reap(p, 90, 30 + 10.0f * level);
    Spark::burst(*p.level, p.x, p.bodyY(), SOUL, 16, 100, true);
  }
  if (kind == TalentKind::SoulHoard && Spark::alive < 600)
    p.level->add<Spark>(e.x, e.bodyY(), (p.x - e.x) * 2, (p.bodyY() - e.bodyY()) * 2, SOUL,
                        0.4f, 0.5f, true);
  if (kind == TalentKind::Plunderer && (e.elite || e.title) && rng.chance(0.04f * level)) {
    p.level->add<Chest>(e.x + 10, e.y);
    Sound::chest.play(0.4f, 0, 1.2f);
  }
}

void addTalents(std::vector<std::unique_ptr<Upgrade>> &u) {
  for (int i = 0; i < int(TalentKind::Count); i++)
    u.push_back(std::make_unique<Talent>(TalentKind(i)));
}
