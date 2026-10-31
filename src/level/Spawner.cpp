#include "level/Spawner.hpp"

#include "Game.hpp"
#include <spdlog/fmt/fmt.h>
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/SupplyBarrel.hpp"
#include "level/Level.hpp"
#include "entity/enemy/Boss.hpp"
#include "level/Flavour.hpp"
#include "level/biome/Biome.hpp"
#include "particle/Spark.hpp"

namespace {
struct WaveDef {
  const char *call;
  float from;
};
const WaveDef WAVE_DEFS[Spawner::WAVES] = {
    {"THE HORDE CLOSES IN", 0},  {"A SWARM DESCENDS", 60},
    {"THE PACK HUNTS", 100},     {"AMBUSH!", 120},
    {"THE DEAD RISE", 150},      {"THE LAND STIRS", 90},
};
const char *const NAMES[] = {"GRISK", "MORGRA", "VAEL", "UTHRAK", "SELKA",
                             "BRANNOCH", "ISKRA", "DOLM", "HEXWYN", "KORR",
                             "AMSEL", "TOVRIK", "NYX", "RUSKA", "OSSIAN",
                             "MALVEK"};
const char *const EPITHETS[] = {"THE HOLLOW",   "IRONHIDE",     "BONEGNAW",
                                "THE UNBOWED",  "OF THE MIRE",  "ASHEN",
                                "THE PALE",     "STORMBORN",    "THE GLUTTON",
                                "NIGHTSTRIDER", "THE CROWNED",  "OF MANY SCARS"};
const char *const ARMS[] = {"Arcane Staff",   "Orbiting Knives", "Frost Nova",
                            "Chain Lightning", "Hurled Axe",      "Hunter's Bow",
                            "Warding Halo",   "Storm Rod"};
bool crownable(EnemyKind k) {
  return k < EnemyKind::Mimic && k != EnemyKind::Cyclops && k != EnemyKind::Thief &&
         k != EnemyKind::Skeleton && k != EnemyKind::Alpha;
}
} // namespace

void Spawner::tick() {
  const Balance &b = level.game.balance;
  const Omen &o = level.portent();
  float t = level.time;
  std::vector<EnemyKind> pool, common;
  for (int k = 0; k < int(EnemyKind::Count); k++)
    if (Enemy::typeOf(EnemyKind(k)).unlockAt <= t)
      (k < int(EnemyKind::Cyclops) ? common : pool).push_back(EnemyKind(k));
  pool.insert(pool.end(), common.begin(), common.end());
  int n = int(common.size());
  auto pick = [&](double x, double y) {
    if (t >= 70 && rng.chance(0.015f))
      return EnemyKind::Thief;
    uint32_t foes =
        BIOMES[world::biomeAt(int(std::floor(x / 16)), int(std::floor(y / 16)))]
            ->foes;
    BiomeId region =
        world::biomeAt(int(std::floor(x / 16)), int(std::floor(y / 16)));
    if (region == BIOME_MIRE)
      foes |= foe(EnemyKind::MireLeech) | foe(EnemyKind::BogLantern);
    if (region == BIOME_DESERT || region == BIOME_BADLANDS)
      foes |= foe(EnemyKind::GlassMantis) | foe(EnemyKind::DustVulture);
    if (region == BIOME_SNOW || region == BIOME_TAIGA)
      foes |= foe(EnemyKind::FrostWisp) | foe(EnemyKind::Iceback);
    if (region == BIOME_FUNGAL)
      foes |= foe(EnemyKind::Sporeling) | foe(EnemyKind::MycelialBrute);
    if (region == BIOME_ASHLANDS)
      foes |= foe(EnemyKind::EmberImp) | foe(EnemyKind::CinderGolem);
    EnemyKind local[int(EnemyKind::Count)];
    int m = 0;
    for (EnemyKind k : pool)
      if (foes & foe(k))
        local[m++] = k;
    if (m && rng.chance(b.localChance))
      return local[rng.irange(0, m - 1)];
    return n > 2 && rng.chance(b.strongChance) ? common[n - 1 - rng.irange(0, 1)]
                                      : common[rng.irange(0, n - 1)];
  };

  if ((timer -= DT) <= 0) {
    timer += std::max(b.spawnFloor, b.spawnInterval - t * b.spawnRamp) * o.rate;
    for (int i = 0, batch = 1 + int(t / b.batchEvery);
         i < batch && level.count<Enemy>() < int(b.enemyCap); i++) {
      double x, y;
      ring(rng.range(280, 320), x, y);
      spawn(pick(x, y), x, y, false);
    }
  }
  if (t >= nextWave) {
    nextWave += b.hordeEvery * o.waves;
    int count = int(std::min(b.hordeMax, b.hordeBase + t / b.hordeRamp) * o.horde);
    int open[WAVES], n = 0;
    for (int i = 0; i < WAVES; i++)
      if (WAVE_DEFS[i].from <= t && i != lastWave)
        open[n++] = i;
    wave(n ? open[rng.irange(0, n - 1)] : 0, count);
  }
  if (t >= nextMimic * o.mimics) {
    nextMimic += rng.range(45, 75);
    int mimics = 0;
    level.each<Enemy>([&](Enemy &e) { mimics += e.is(EnemyKind::Mimic); });
    if (mimics < (o.mimics < 1 ? 4 : 2)) {
      double x, y;
      ring(rng.range(150, 230), x, y);
      spawn(EnemyKind::Mimic, x, y, false);
    }
  }
  if (t >= nextBand * o.bands) {
    nextBand += rng.range(70, 110);
    int renegades = 0;
    level.each<Enemy>([&](Enemy &e) {
      renegades += e.kind() >= int(EnemyKind::Outlaw);
    });
    if (renegades < 4) {
      double x, y;
      ring(rng.range(280, 310), x, y);
      for (int i = 0, n = 2 + (t > 420) + (t > 720); i < n; i++) {
        auto k = EnemyKind(int(EnemyKind::Outlaw) + rng.irange(0, 3));
        if (Enemy::typeOf(k).unlockAt * o.bands > t)
          k = EnemyKind::Outlaw;
        spawn(k, x + rng.range(-24, 24), y + rng.range(-24, 24));
      }
      level.game.announce("RENEGADES ON YOUR TRAIL");
    }
  }
  if (t >= nextChampion * o.champions) {
    nextChampion += rng.range(150, 230);
    bool alive = false;
    level.each<Enemy>([&](Enemy &e) { alive |= e.title && !e.def; });
    if (!alive) {
      double x, y;
      ring(rng.range(300, 340), x, y);
      if (Enemy *c = champion(x, y))
        level.game.announce(fmt::format("A CHAMPION STALKS: {}", c->title));
    }
  }
  if (t >= nextBarrel) {
    nextBarrel += 20;
    auto b = std::make_unique<SupplyBarrel>();
    ring(rng.range(270, 300), b->x, b->y);
    level.add(std::move(b));
  }
  if (t >= nextBoss) {
    nextBoss += b.bossEvery;
    bosses++;
    double x, y;
    ring(300, x, y);
    const BossDef *def = bossFor(world::biomeAt(int(std::floor(level.player->x / 16)),
                                                int(std::floor(level.player->y / 16))));
    if (!def || rng.chance(0.3f))
      def = &BOSS_DEFS[(bosses - 1) % 3];
    spawn(def->kind, x, y, true, def->kind != EnemyKind::Cyclops);
    level.game.announce(def->arrival);
    Sound::boss.play(0.8f, 0);
  }
}

void Spawner::wave(int index, int count) {
  lastWave = index;
  float t = level.time;
  Player &p = *level.player;
  double reach = level.game.screen.reach(), r = 270 + reach;
  std::vector<EnemyKind> common;
  for (int k = 0; k < int(EnemyKind::Cyclops); k++)
    if (Enemy::typeOf(EnemyKind(k)).unlockAt <= t)
      common.push_back(EnemyKind(k));
  if (common.empty())
    common.push_back(EnemyKind::Bat);
  auto around = [&](EnemyKind k, int n, double radius, double from, double arc) {
    for (int i = 0; i < n; i++) {
      double dir = from + arc * (i + 0.5) / n;
      spawn(k, p.x + std::cos(dir) * radius, p.y + std::sin(dir) * radius);
    }
  };
  switch (index) {
  case 1: {
    around(common[rng.irange(0, std::min(1, int(common.size()) - 1))],
           count * 3 / 2, r, 0, TAU);
    for (int i = 0; i < 2; i++) {
      double dir = rng.range(0, TAU);
      if (Enemy *e = spawn(common.back(), p.x + std::cos(dir) * r,
                           p.y + std::sin(dir) * r))
        if (!e->elite)
          e->makeElite(level.game.balance.eliteHp);
    }
    break;
  }
  case 2: {
    double dir = rng.range(0, TAU);
    for (int i = 0, n = std::max(1, count / 10); i < n; i++)
      spawn(EnemyKind::Alpha, p.x + std::cos(dir + i * 0.4) * r,
            p.y + std::sin(dir + i * 0.4) * r);
    break;
  }
  case 3: {
    double ahead = p.xa * p.xa + p.ya * p.ya > 100 ? std::atan2(p.ya, p.xa)
                                                   : rng.range(0, TAU);
    for (int i = 0; i < count; i++) {
      double dir = ahead - TAU / 6 + TAU / 3 * (i + 0.5) / count,
             d = 190 + reach + rng.range(0, 40);
      spawn(common[rng.irange(0, int(common.size()) - 1)],
            p.x + std::cos(dir) * d, p.y + std::sin(dir) * d);
    }
    break;
  }
  case 4: {
    for (int i = 0, n = count * 2 / 3; i < n; i++) {
      double dir = TAU * i / n, d = rng.range(130, 170);
      double x = p.x + std::cos(dir) * d, y = p.y + std::sin(dir) * d;
      spawn(EnemyKind::Skeleton, x, y);
      if (Spark::alive < 500)
        Spark::burst(level, x, y, Color{0.85f, 0.85f, 0.8f, 1}, 4, 40, false);
    }
    double dir = rng.range(0, TAU);
    spawn(EnemyKind::Necromancer, p.x + std::cos(dir) * r, p.y + std::sin(dir) * r);
    break;
  }
  case 5: {
    uint32_t foes = BIOMES[world::biomeAt(int(std::floor(p.x / 16)),
                                          int(std::floor(p.y / 16)))]->foes;
    std::vector<EnemyKind> local;
    for (int k = 0; k < int(EnemyKind::Count); k++)
      if ((foes & foe(EnemyKind(k))) && k != int(EnemyKind::Cyclops) &&
          k != int(EnemyKind::Alpha) && k != int(EnemyKind::Thief))
        local.push_back(EnemyKind(k));
    if (local.empty())
      local = common;
    for (int i = 0; i < count; i++) {
      double dir = TAU * i / count;
      spawn(local[i % local.size()], p.x + std::cos(dir) * r, p.y + std::sin(dir) * r);
    }
    break;
  }
  default:
    around(common.back(), count, r, 0, TAU);
  }
  level.game.announce(WAVE_DEFS[index].call);
  Sound::horde.play(0.7f, 0);
}

Enemy *Spawner::champion(double x, double y) {
  uint32_t foes = BIOMES[world::biomeAt(int(std::floor(x / 16)),
                                        int(std::floor(y / 16)))]->foes;
  std::vector<EnemyKind> pick;
  for (int k = 0; k < int(EnemyKind::Count); k++)
    if ((foes & foe(EnemyKind(k))) && crownable(EnemyKind(k)))
      pick.push_back(EnemyKind(k));
  if (pick.empty())
    for (int k = 0; k < int(EnemyKind::Cyclops); k++)
      if (Enemy::typeOf(EnemyKind(k)).unlockAt <= level.time)
        pick.push_back(EnemyKind(k));
  if (pick.empty())
    pick.push_back(EnemyKind::Rat);
  Enemy *e = spawn(pick[rng.irange(0, int(pick.size()) - 1)], x, y, true);
  if (!e)
    return nullptr;
  level.titles.push_back(fmt::format("{} {}", NAMES[rng.irange(0, int(std::size(NAMES)) - 1)],
                                     EPITHETS[rng.irange(0, int(std::size(EPITHETS)) - 1)]));
  e->crown(level.titles.back().c_str());
  e->might = 0.5f, e->haste = 1.6f;
  e->grant(ARMS[rng.irange(0, int(std::size(ARMS)) - 1)], 2);
  level.bossIds.push_back(e->id);
  return e;
}

Enemy *Spawner::spawn(EnemyKind kind, double x, double y, bool aware,
                      bool boss) {
  std::lock_guard<std::mutex> lock(spawnMutex);
  const Balance &b = level.game.balance;
  const Omen &o = level.portent();
  float hpMul = b.enemyHp * (1 + level.time / b.enemyHpRamp) * o.hp;
  float dmgMul = b.enemyDamage * (1 + level.time / b.enemyDamageRamp);
  float bossHp = 1 + (bosses - 1) * b.bossHpStep;
  if (kind == EnemyKind::Cyclops)
    hpMul *= bossHp;
  auto e = Enemy::create(kind, hpMul, dmgMul);
  e->x = x, e->y = y, e->aware = aware;
  if (boss)
    e->makeBoss(Enemy::typeOf(EnemyKind::Cyclops).hp * hpMul * bossHp, 1.6f);
  else if (kind != EnemyKind::Cyclops && level.time > b.eliteStart &&
      rng.chance((b.eliteBase + level.time / b.eliteRamp) * level.eliteMul)) {
    e->makeElite(b.eliteHp);
    if (rng.chance(o.skilled))
      e->learn();
  }
  Enemy *made = e.get();
  Entity *added = level.add(std::move(e));
  if (boss || kind == EnemyKind::Cyclops) {
    made->rule(bossOf(kind));
    level.bossIds.push_back(added->id);
  }
  entt::entity alphaId = added->id;

  if (kind == EnemyKind::Alpha) {
    int packSize = boss ? rng.irange(6, 8) : rng.irange(2, 4);
    for (int i = 0; i < packSize; i++) {
      double a = TAU * i / packSize + rng.range(-0.3f, 0.3f);
      double d = rng.range(22, 38);
      auto w = Enemy::create(EnemyKind::Wolf, hpMul, dmgMul);
      w->x = x + std::cos(a) * d;
      w->y = y + std::sin(a) * d;
      w->aware = aware;
      w->leader = alphaId;
      level.add(std::move(w));
    }
  }
  return made;
}

void Spawner::ring(float dist, double &x, double &y) const {
  Player &p = *level.player;
  double dir;
  if (p.xa * p.xa + p.ya * p.ya > 100 && rng.chance(0.5f))
    dir = std::atan2(p.ya, p.xa) + rng.range(-0.8f, 0.8f);
  else
    dir = rng.range(0, TAU);
  dist += level.game.screen.reach();
  x = p.x + std::cos(dir) * dist;
  y = p.y + std::sin(dir) * dist;
}
