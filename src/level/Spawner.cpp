#include "level/Spawner.hpp"

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/SupplyBarrel.hpp"
#include "level/Level.hpp"
#include "level/biome/Biome.hpp"

void Spawner::tick() {
  const Balance &b = level.game.balance;
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
    timer += std::max(b.spawnFloor, b.spawnInterval - t * b.spawnRamp);
    for (int i = 0, batch = 1 + int(t / b.batchEvery);
         i < batch && level.count<Enemy>() < int(b.enemyCap); i++) {
      double x, y;
      ring(rng.range(280, 320), x, y);
      spawn(pick(x, y), x, y, false);
    }
  }
  if (t >= nextWave) {
    nextWave += b.hordeEvery;
    int count = std::min(int(b.hordeMax), int(b.hordeBase + t / b.hordeRamp));
    double r = 270 + level.game.screen.reach();
    for (int i = 0; i < count; i++) {
      double dir = TAU * i / count;
      spawn(pool.back(), level.player->x + std::cos(dir) * r,
            level.player->y + std::sin(dir) * r);
    }
    level.game.announce("THE HORDE CLOSES IN");
    Sound::horde.play(0.7f, 0);
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
    struct BossDef {
      EnemyKind kind;
      const char *arrival;
    };
    static const BossDef BOSSES[] = {
        {EnemyKind::Cyclops, "A CYCLOPS APPROACHES"},
        {EnemyKind::Alpha, "THE PACK LORD HOWLS"},
        {EnemyKind::Necromancer, "THE LICH RISES"}};
    const BossDef &def = BOSSES[(bosses - 1) % 3];
    spawn(def.kind, x, y, true, def.kind != EnemyKind::Cyclops);
    level.game.announce(def.arrival);
    Sound::boss.play(0.8f, 0);
  }
}

void Spawner::spawn(EnemyKind kind, double x, double y, bool aware,
                    bool boss) {
  std::lock_guard<std::mutex> lock(spawnMutex);
  const Balance &b = level.game.balance;
  float hpMul = b.enemyHp * (1 + level.time / b.enemyHpRamp);
  float dmgMul = b.enemyDamage * (1 + level.time / b.enemyDamageRamp);
  float bossHp = 1 + (bosses - 1) * b.bossHpStep;
  if (kind == EnemyKind::Cyclops)
    hpMul *= bossHp;
  auto e = Enemy::create(kind, hpMul, dmgMul);
  e->x = x, e->y = y, e->aware = aware;
  if (boss)
    e->makeBoss(Enemy::typeOf(EnemyKind::Cyclops).hp * hpMul * bossHp, 1.6f);
  else if (kind != EnemyKind::Cyclops && level.time > b.eliteStart &&
      rng.chance((b.eliteBase + level.time / b.eliteRamp) * level.eliteMul))
    e->makeElite(b.eliteHp);
  Entity *added = level.add(std::move(e));
  if (boss || kind == EnemyKind::Cyclops)
    level.bossIds.push_back(added->id);
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
