#include "level/Level.hpp"

#include <cmath>
#include <iterator>

#include "Game.hpp"
#include "entity/Faction.hpp"
#include "entity/Player.hpp"
#include "gui/Lang.hpp"
#include "level/Noise.h"
#include "level/World.hpp"
#include "particle/Spark.hpp"

bool Level::flares(int rx, int ry, int night, int slot) {
  uint32_t h = hash2(hash2(rx * 31 + night * 7, ry * 17 - slot * 13), int(noise_salt));
  return h % 1000 < FLARE_ODDS;
}

void Level::repute(const Enemy &e) {
  Faction f = factionOf(e);
  if (int(f) >= FAMILIES)
    return;
  if (!grudged)
    grudges = Grudges::roll(noise_salt), grudged = true;
  auto shift = [&](Faction g, int by) {
    int &r = rep[int(g)];
    bool was = r >= FRIENDLY;
    r = std::clamp(r + by, -REP_MAX, REP_MAX);
    if (was != (r >= FRIENDLY))
      game.announce(say(was ? lang.repLost : lang.repFriendly, tr(factionName(g))));
  };
  shift(f, -REP_KILL);
  for (int g = 0; g < FAMILIES; g++)
    if (grudges.between(f, Faction(g)))
      shift(Faction(g), REP_RIVAL);
}

void Level::watch() {
  if (!rules.synod)
    return;
  constexpr const char *Lang::*WARNS[] = {&Lang::synodPatrols, &Lang::synodInquisition, &Lang::synodBounty};
  int stage = scrutiny >= BOUNTY ? 3 : scrutiny >= INQUISITION ? 2 : scrutiny >= PATROLS ? 1 : 0;
  if (stage > scrutinySaid)
    game.announce(lang.*WARNS[stage - 1]);
  scrutinySaid = stage;
  if (!stage || world::layer() != world::Layer::Surface || player->dead() || rules.safeAt(player->x, player->y) ||
      (patrolTime -= DT) > 0)
    return;
  patrolTime = PATROL_EVERY;
  constexpr EnemyKind PATROL[] = {EnemyKind::Lamplighter, EnemyKind::Penitent, EnemyKind::Penitent, EnemyKind::Limer,
                                  EnemyKind::Censer, EnemyKind::Cutthroat, EnemyKind::Cutthroat};
  int size = stage == 1 ? 4 : stage == 2 ? 5 : 7;
  double a0 = rng.range(0, float(TAU));
  for (int i = 0; i < size; i++) {
    double a = a0 + i * 0.18, x = player->x + std::cos(a) * 300, y = player->y + std::sin(a) * 300;
    spawner.spawn(PATROL[i], x, y, true);
  }
  spdlog::info("synod: patrol of {} at scrutiny {:.0f}", size, scrutiny);
}

void Level::suspect(float by) {
  if (rules.synod)
    scrutiny = std::clamp(scrutiny + by, 0.0f, SCRUTINY_MAX);
}

int Level::ignite() {
  if (!brawlers.empty())
    return 0;
  if (!grudged)
    grudges = Grudges::roll(noise_salt), grudged = true;
  Enemy *near[160];
  int n = 0, counts[int(Faction::Count)]{};
  enemiesNear(player->x, player->y, FLARE_REACH, [&](Enemy &e) {
    Faction f = factionOf(e);
    if (n < 160 && !e.boss && !e.removed && f != Faction::Hearth)
      near[n++] = &e, counts[int(f)]++;
  });
  Faction pairs[Grudges::MAX + std::size(UNCLEAN)][2];
  int pn = 0;
  for (int g = 0; g < grudges.count; g++)
    pairs[pn][0] = grudges.pair[g][0], pairs[pn++][1] = grudges.pair[g][1];
  for (Faction u : UNCLEAN)
    pairs[pn][0] = Faction::Synod, pairs[pn++][1] = u;
  for (int g = 0; g < pn; g++) {
    Faction a = pairs[g][0], b = pairs[g][1];
    if (counts[int(a)] < 2 || counts[int(b)] < 2)
      continue;
    int sides[2]{};
    for (int i = 0; i < n; i++) {
      Faction f = factionOf(*near[i]);
      int side = f == a ? 0 : f == b ? 1 : -1;
      if (side < 0 || sides[side] >= BRAWLERS)
        continue;
      sides[side]++;
      near[i]->feud = rng.range(20, 40), near[i]->rival = entt::null, near[i]->brawl = 0;
      brawlers.push_back(near[i]->id);
    }
    game.announce(say(lang.feudFlares, tr(factionName(a)), tr(factionName(b))));
    spdlog::info("feud: {} and {} flare at {:.0f} s ({} brawlers)", factionName(a), factionName(b), time,
                 brawlers.size());
    return int(brawlers.size());
  }
  return 0;
}

void Level::host(int count) {
  constexpr EnemyKind HOST[] = {EnemyKind::Penitent, EnemyKind::Lamplighter, EnemyKind::Penitent, EnemyKind::Limer,
                                EnemyKind::Censer};
  int members = std::max(4, count / 4);
  for (int i = 0; i < members; i++) {
    double a = TAU * i / members, x = player->x + std::cos(a) * 280, y = player->y + std::sin(a) * 280;
    spawner.spawn(i == 0 ? EnemyKind::Confessor : HOST[i % std::size(HOST)], x, y, true);
  }
  game.announce(lang.inquisitionComes);
  spdlog::info("synod: inquisition of {} replaces the horde at scrutiny {:.0f}", members, scrutiny);
}

void Level::feuds() {
  watch();
  if (!rules.feuds || player->dead() || night.nights < GRACE || (flareTime -= DT) > 0)
    return;
  if (rules.sieged && rules.sieged(player->x, player->y))
    return void(flareTime = FLARE_EVERY);
  for (const SafeZone &z : rules.safe)
    if (std::hypot(player->x - z.x, player->y - z.y) < z.radius + WILD)
      return void(flareTime = FLARE_EVERY);
  flareTime = FLARE_EVERY;
  int rx = world::regionOf(world::tileOf(player->x)), ry = world::regionOf(world::tileOf(player->y));
  if (flares(rx, ry, night.nights, int(time / FLARE_EVERY)))
    ignite();
}

void Level::brawl() {
  std::erase_if(brawlers, [&](entt::entity id) {
    Enemy *e = get<Enemy>(id);
    if (!e)
      return true;
    if ((e->feud -= DT) <= 0)
      return e->feud = 0, true;
    Enemy *r = get<Enemy>(e->rival);
    Faction mine = factionOf(*e);
    if (!r) {
      Enemy *pick[24];
      double best = 1e18;
      for (int i = 0, k = gather(e->x, e->y, RIVAL_REACH, pick, 24); i < k; i++) {
        double dx = pick[i]->x - e->x, dy = pick[i]->y - e->y;
        if (pick[i]->feud > 0 && grudges.between(mine, factionOf(*pick[i])) && dx * dx + dy * dy < best)
          best = dx * dx + dy * dy, r = pick[i];
      }
      if (!r)
        return e->feud = 0, true;
      e->rival = r->id;
    }
    double dx = r->x - e->x, dy = r->y - e->y, d = std::max(1e-3, std::sqrt(dx * dx + dy * dy));
    e->xWant = dx / d * e->type.speed, e->yWant = dy / d * e->type.speed;
    if (d < e->radius + r->radius + 6 && (e->brawl -= DT) <= 0) {
      e->brawl = BRAWL_EVERY;
      r->hurt(e->type.damage * e->damageMul * BRAWL_DAMAGE, dx / d, dy / d, 60);
      Spark::burst(*this, r->x, r->y - 6, r->type.blood, 3, 30, false);
    }
    return false;
  });
}
