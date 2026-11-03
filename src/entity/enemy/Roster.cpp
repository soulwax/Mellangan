#include "entity/enemy/Enemy.hpp"
#include "entity/enemy/Boss.hpp"
#include "entity/enemy/Hazard.hpp"
#include "entity/Ability.hpp"

#include "Game.hpp"
#include <spdlog/fmt/fmt.h>
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/AlphaWolf.hpp"
#include "entity/enemy/Burrower.hpp"
#include "entity/enemy/Cyclops.hpp"
#include "entity/enemy/Ghost.hpp"
#include "entity/enemy/Mimic.hpp"
#include "entity/enemy/Necromancer.hpp"
#include "entity/enemy/Renegade.hpp"
#include "entity/enemy/Thief.hpp"
#include "entity/enemy/ExpansionEnemy.hpp"
#include "entity/enemy/WildFoe.hpp"
#include "entity/enemy/Wolf.hpp"
#include "entity/enemy/ai/Plans.hpp"
#include "entity/enemy/ai/Melee.hpp"
#include "entity/pickup/Chest.hpp"
#include "entity/pickup/Gem.hpp"
#include "entity/pickup/Potion.hpp"
#include "hero/Hero.hpp"
#include "item/Upgrade.hpp"
#include "entity/projectile/Orb.hpp"
#include "level/Level.hpp"
#include "particle/TextParticle.hpp"
#include "particle/DeathEffect.hpp"
#include "particle/Drift.hpp"
#include "particle/Shockwave.hpp"
#include "particle/debris/Chip.hpp"
#include "particle/Spark.hpp"
#include "particle/debris/Debris.hpp"
#include "particle/debris/FlameDebris.hpp"

namespace {
namespace t = atlas::t;
namespace fr = atlas::fr;
namespace bs = atlas::bs;

const EnemyType TYPES[] = {
    {t::Bat, 5, 50, 5, 5, 1, 1, 1, {0.85f, 0.55f, 0.35f, 1}, true, 0},
    {t::Rat, 9, 40, 6, 6, 1, 1, 1, {0.75f, 0.55f, 0.4f, 1}, false, 25},
    {t::Slime, 16, 30, 8, 6, 1, 2, 1, {0.3f, 0.95f, 0.6f, 1}, false, 50},
    {t::Scorpion, 14, 58, 7, 6, 1, 2, 1, {0.7f, 0.75f, 0.8f, 1}, false, 70},
    {t::Spider, 22, 48, 10, 6, 1, 3, 1, {0.9f, 0.6f, 0.4f, 1}, false, 100},
    {t::Ghost, 30, 36, 12, 6, 1, 4, 1, {0.8f, 0.85f, 1, 1}, true, 160},
    {t::Cultist, 40, 38, 12, 6, 1, 5, 1, {0.6f, 0.3f, 0.8f, 1}, false, 200},
    {t::Demon, 70, 32, 16, 8, 1.25f, 8, 0.8f, {1, 0.3f, 0.25f, 1}, false, 240},
    {t::Knight,
     120,
     26,
     18,
     7,
     1.15f,
     10,
     0.4f,
     {0.75f, 0.78f, 0.85f, 1},
     false,
     300},
    {t::Cyclops,
     900,
     30,
     28,
     15,
     2.5f,
     40,
     0.15f,
     {1, 0.75f, 0.5f, 1},
     false,
     1e9f},
    {fr::Wolf, 28, 62, 9, 7, 1.0f, 3, 0.85f, {0.6f, 0.15f, 0.15f, 1}, false, 40,
     atlas::frontier},
    {fr::Alpha, 85, 54, 16, 9, 1.3f, 8, 0.5f, {0.55f, 0.12f, 0.12f, 1}, false, 80,
     atlas::frontier},
    {fr::Burrower, 48, 65, 15, 8, 1.1f, 5, 0.6f, {0.85f, 0.55f, 0.3f, 1}, false, 60,
     atlas::frontier},
    {fr::Necromancer, 60, 30, 10, 7, 1.1f, 7, 0.7f, {0.35f, 0.1f, 0.45f, 1}, false, 110,
     atlas::frontier},
    {fr::Skeleton, 14, 42, 7, 6, 0.95f, 1, 1.0f, {0.85f, 0.85f, 0.8f, 1}, false, 1e9f,
     atlas::frontier},
    {fr::Thief, 36, 68, 0, 7, 1.0f, 10, 0.9f, {0.95f, 0.8f, 0.3f, 1}, false, 90,
     atlas::frontier},
    {0,22,42,4,6,1,3,1,{0.7f,0.3f,0.5f,1},false,35,atlas::expansion},
    {1,32,30,3,6,1,4,1,{0.5f,0.9f,0.5f,1},true,55,atlas::expansion},
    {2,38,50,10,6,1,4,0.8f,{0.5f,0.8f,0.9f,1},false,75,atlas::expansion},
    {3,28,48,8,6,1,3,1,{0.7f,0.5f,0.3f,1},true,65,atlas::expansion},
    {4,26,33,4,5,1,4,1,{0.5f,0.8f,1,1},true,80,atlas::expansion},
    {5,100,25,14,9,1.3f,8,0.3f,{0.6f,0.8f,0.9f,1},false,130,atlas::expansion},
    {6,18,38,4,5,1,3,1,{0.6f,0.8f,0.3f,1},false,50,atlas::expansion},
    {7,130,24,12,10,1.3f,9,0.3f,{0.6f,0.4f,0.7f,1},false,150,atlas::expansion},
    {8,36,45,7,6,1,5,0.8f,{1,0.4f,0.2f,1},false,110,atlas::expansion},
    {9,160,22,16,10,1.4f,12,0.25f,{0.7f,0.3f,0.1f,1},false,190,atlas::expansion},
    {92, 60, 66, 14, 7, 1.1f, 12, 0.6f, {0.8f, 0.3f, 0.3f, 1}, false, 60},
    {85, 45, 40, 8, 6, 1, 8, 0.9f, {0.65f, 0.08f, 0.1f, 1}, false, 150},
    {87, 110, 40, 16, 7, 1.15f, 12, 0.5f, {0.65f, 0.08f, 0.1f, 1}, false, 180},
    {88, 70, 58, 12, 6, 1.05f, 10, 0.7f, {0.65f, 0.08f, 0.1f, 1}, false, 210},
    {98, 130, 30, 16, 7, 1.1f, 12, 0.4f, {0.65f, 0.08f, 0.1f, 1}, false, 240},
    {bs::Thornling, 12, 58, 5, 5, 1, 1, 1, {0.35f, 0.6f, 0.25f, 1}, false, 20, atlas::beast},
    {bs::Boar, 110, 32, 14, 8, 1.2f, 8, 0.4f, {0.7f, 0.15f, 0.12f, 1}, false, 60, atlas::beast},
    {bs::GoblinArcher, 28, 46, 6, 6, 1, 3, 1, {0.5f, 0.75f, 0.3f, 1}, false, 90, atlas::beast},
    {bs::GoblinShaman, 34, 40, 6, 6, 1, 5, 1, {0.5f, 0.75f, 0.3f, 1}, false, 120, atlas::beast},
    {bs::Sapling, 40, 30, 8, 6, 1, 4, 0.8f, {0.55f, 0.4f, 0.25f, 1}, false, 80, atlas::beast},
    {bs::Treant, 170, 22, 18, 10, 1.4f, 12, 0.25f, {0.55f, 0.4f, 0.25f, 1}, false, 1e9f,
     atlas::beast},
    {bs::MothWisp, 14, 55, 5, 5, 1, 2, 1, {0.85f, 0.8f, 1, 1}, true, 100, atlas::beast},
    {bs::Cutthroat, 36, 60, 10, 6, 1, 5, 0.9f, {0.7f, 0.1f, 0.12f, 1}, false, 140, atlas::beast},
    {bs::BogToad, 34, 44, 9, 6, 1, 4, 0.9f, {0.4f, 0.6f, 0.3f, 1}, false, 40, atlas::beast},
    {bs::Drowned, 40, 26, 10, 6, 1, 4, 0.9f, {0.35f, 0.55f, 0.65f, 1}, false, 60, atlas::beast},
    {bs::Snapjaw, 140, 30, 18, 9, 1.3f, 10, 0.3f, {0.45f, 0.55f, 0.3f, 1}, false, 120,
     atlas::beast},
    {bs::StranglerVine, 60, 0, 6, 8, 1.1f, 5, 0, {0.3f, 0.55f, 0.25f, 1}, false, 70,
     atlas::beast},
    {bs::AmberBeetle, 55, 34, 11, 7, 1.05f, 6, 0.4f, {0.85f, 0.6f, 0.2f, 1}, false, 100,
     atlas::beast},
    {bs::SporePuffer, 10, 52, 3, 5, 1, 1, 1, {0.75f, 0.6f, 0.8f, 1}, false, 30, atlas::beast},
    {bs::SunkenPriest, 45, 34, 8, 6, 1, 7, 0.8f, {0.4f, 0.6f, 0.7f, 1}, false, 140,
     atlas::beast},
    {bs::Yeti, 150, 26, 16, 9, 1.3f, 10, 0.3f, {0.85f, 0.9f, 1, 1}, false, 90, atlas::beast},
    {bs::IceWraith, 34, 40, 9, 6, 1, 5, 0.9f, {0.6f, 0.85f, 1, 1}, true, 70, atlas::beast},
    {bs::BarrowWight, 50, 34, 10, 6, 1, 7, 0.7f, {0.45f, 0.6f, 0.5f, 1}, false, 120,
     atlas::beast},
    {bs::CrystalGolem, 160, 22, 16, 10, 1.35f, 12, 0.25f, {0.75f, 0.6f, 1, 1}, false, 150,
     atlas::beast},
    {bs::CrystalGolem, 14, 62, 5, 5, 0.6f, 1, 1, {0.75f, 0.6f, 1, 1}, false, 1e9f,
     atlas::beast},
    {bs::MoonHound, 36, 64, 10, 7, 1, 5, 0.8f, {0.7f, 0.75f, 0.95f, 1}, false, 110,
     atlas::beast},
    {bs::Lynx, 38, 60, 11, 6, 1, 5, 0.85f, {0.75f, 0.2f, 0.15f, 1}, false, 60, atlas::beast},
    {bs::BarrowArcher, 30, 40, 7, 6, 1, 4, 1, {0.8f, 0.8f, 0.75f, 1}, false, 100,
     atlas::beast},
    {bs::SandWyrm, 480, 34, 22, 14, 2.2f, 30, 0, {0.85f, 0.7f, 0.4f, 1}, false, 300,
     atlas::beast},
    {bs::Hyena, 16, 72, 6, 5, 1, 1, 1, {0.7f, 0.5f, 0.3f, 1}, false, 50, atlas::beast},
    {bs::SaltGolem, 140, 24, 16, 10, 1.35f, 10, 0.25f, {0.92f, 0.92f, 0.86f, 1}, false, 130,
     atlas::beast},
    {bs::GlassSpitter, 30, 40, 7, 6, 1, 4, 1, {0.6f, 0.9f, 0.85f, 1}, false, 90,
     atlas::beast},
    {bs::StormKite, 26, 50, 8, 6, 1, 4, 1, {0.6f, 0.7f, 1, 1}, true, 110, atlas::beast},
    {bs::DuneRaider, 40, 66, 11, 6, 1, 5, 0.85f, {0.75f, 0.2f, 0.15f, 1}, false, 70,
     atlas::beast},
    {bs::TotemShaman, 36, 36, 7, 6, 1, 6, 0.8f, {0.6f, 0.3f, 0.2f, 1}, false, 120,
     atlas::beast},
    {bs::WarTotem, 30, 0, 0, 6, 1, 2, 0, {0.6f, 0.4f, 0.25f, 1}, false, 1e9f, atlas::beast},
    {bs::MagmaSlug, 45, 26, 10, 7, 1.1f, 5, 0.5f, {1, 0.5f, 0.2f, 1}, false, 100, atlas::beast},
    {bs::AshWraith, 36, 44, 10, 6, 1, 6, 0.9f, {0.5f, 0.45f, 0.45f, 1}, true, 130,
     atlas::beast},
    {bs::CaveHorror, 170, 30, 20, 10, 1.4f, 14, 0.2f, {0.5f, 0.3f, 0.4f, 1}, false, 160,
     atlas::beast},
    {bs::StarSpawn, 40, 46, 10, 6, 1, 6, 0.8f, {0.6f, 0.6f, 1, 1}, false, 140, atlas::beast},
    {bs::StarSpawn, 10, 70, 4, 4, 0.55f, 1, 1, {0.6f, 0.6f, 1, 1}, false, 1e9f, atlas::beast},
    {bs::FlameCultist, 38, 36, 9, 6, 1, 7, 0.8f, {0.9f, 0.35f, 0.15f, 1}, false, 150,
     atlas::beast},
    {bs::HollowSentinel, 160, 24, 18, 9, 1.3f, 12, 0.15f, {0.55f, 0.55f, 0.6f, 1}, false, 180,
     atlas::beast},
    {bs::EmberDrake, 520, 46, 20, 14, 2.2f, 32, 0, {1, 0.45f, 0.15f, 1}, true, 320,
     atlas::beast},
    {bs::NightStalker, 44, 62, 12, 6, 1, 7, 0.8f, {0.3f, 0.2f, 0.4f, 1}, false, 120,
     atlas::beast},
    {bs::GraveRobber, 34, 56, 8, 6, 1, 5, 0.9f, {0.5f, 0.4f, 0.3f, 1}, false, 60, atlas::beast},
    {bs::CaveBat, 6, 74, 3, 4, 0.8f, 1, 1, {0.5f, 0.35f, 0.35f, 1}, true, 40, atlas::beast},
    {bs::RockCrawler, 40, 40, 11, 7, 1, 6, 0.6f, {0.5f, 0.45f, 0.4f, 1}, false, 90,
     atlas::beast},
    {bs::OreGolem, 180, 18, 18, 10, 1.4f, 14, 0.15f, {0.7f, 0.5f, 0.3f, 1}, false, 150,
     atlas::beast},
    {bs::KoboldMiner, 14, 62, 5, 5, 0.9f, 2, 1, {0.8f, 0.4f, 0.2f, 1}, false, 60, atlas::beast},
    {bs::Shade, 30, 34, 0, 6, 1, 6, 0.9f, {0.35f, 0.25f, 0.5f, 1}, true, 1e9f, atlas::beast},
    {bs::Nest, 140, 0, 0, 10, 1.4f, 10, 0, {0.55f, 0.35f, 0.5f, 1}, false, 1e9f, atlas::beast},
};
static_assert(std::size(TYPES) == int(EnemyKind::Count));
} // namespace

const EnemyType &Enemy::typeOf(EnemyKind kind) { return TYPES[int(kind)]; }

std::unique_ptr<Enemy> Enemy::create(EnemyKind kind, float hpMul,
                                     float damageMul) {
  const EnemyType &t = typeOf(kind);
  std::unique_ptr<Enemy> e;
  if (kind >= WildFoe::FIRST && kind <= WildFoe::LAST)
    e = std::make_unique<WildFoe>(t, hpMul, damageMul, int(kind) - int(WildFoe::FIRST));
  else if (kind >= EnemyKind::Outlaw && kind <= EnemyKind::Deserter)
    e = std::make_unique<Renegade>(t, hpMul, damageMul,
                                   int(kind) - int(EnemyKind::Outlaw));
  else if (kind == EnemyKind::Mimic)
    e = std::make_unique<Mimic>(t, hpMul, damageMul);
  else if (kind >= EnemyKind::MireLeech)
    e = std::make_unique<ExpansionEnemy>(t,hpMul,damageMul,int(kind)-int(EnemyKind::MireLeech));
  else if (kind == EnemyKind::Ghost)
    e = std::make_unique<Ghost>(t, hpMul, damageMul);
  else if (kind == EnemyKind::Cyclops)
    e = std::make_unique<Cyclops>(t, hpMul, damageMul);
  else if (kind == EnemyKind::Wolf)
    e = std::make_unique<Wolf>(t, hpMul, damageMul);
  else if (kind == EnemyKind::Alpha)
    e = std::make_unique<AlphaWolf>(t, hpMul, damageMul);
  else if (kind == EnemyKind::Burrower)
    e = std::make_unique<Burrower>(t, hpMul, damageMul);
  else if (kind == EnemyKind::Necromancer)
    e = std::make_unique<Necromancer>(t, hpMul, damageMul);
  else if (kind == EnemyKind::Thief)
    e = std::make_unique<Thief>(t, hpMul, damageMul);
  else
    e = std::make_unique<Enemy>(t, hpMul, damageMul);
  e->plan = planOf(kind);
  return e;
}

const char *Enemy::name() const {
  if (title)
    return title;
  static const char *const NAMES[] = {
      "a bat",          "a rat",           "a slime",
      "a scorpion",     "a spider",        "a ghost",
      "a cultist",      "a demon",         "a knight",
      "the cyclops",    "a wolf",          "an alpha wolf",
      "a burrower",     "a necromancer",   "a skeleton",
      "a treasure thief", "a mire leech", "a bog lantern", "a glass mantis",
      "a dust vulture", "a frost wisp", "an iceback", "a sporeling",
      "a mycelial brute", "an ember imp", "a cinder golem", "a mimic",
      "an outlaw", "a berserker", "a pit fighter", "a deserter",
      "a thornling", "a wild boar", "a goblin archer", "a goblin shaman",
      "a sapling", "a treant", "a moth wisp", "a cutthroat",
      "a bog toad", "a drowned", "a snapjaw", "a strangler vine",
      "an amber beetle", "a spore puffer", "a sunken priest",
      "a yeti", "an ice wraith", "a barrow wight", "a crystal golem",
      "a crystal shard", "a moon hound", "a lynx", "a barrow archer",
      "the sand wyrm", "a hyena", "a salt golem", "a glass spitter",
      "a storm kite", "a dune raider", "a totem shaman", "a war totem",
      "a magma slug", "an ash wraith", "a cave horror", "a star spawn",
      "a starlet", "a flame cultist", "a hollow sentinel", "the ember drake",
      "a night stalker", "a grave robber", "a cave bat", "a rock crawler",
      "an ore golem", "a kobold miner", "a shade", "a nest"
  };
  static_assert(std::size(NAMES) == int(EnemyKind::Count));
  return NAMES[&type - TYPES];
}

int Enemy::kind() const { return int(&type - TYPES); }
