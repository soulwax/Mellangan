#include "campaign/Campaign.hpp"

#include <spdlog/fmt/fmt.h>

#include <cmath>

#include "gui/Lang.hpp"
#include "Game.hpp"
#include "campaign/Bastion.hpp"
#include "campaign/Harvest.hpp"
#include "campaign/Sentry.hpp"
#include "campaign/Elder.hpp"
#include "campaign/Stash.hpp"
#include "campaign/Townsfolk.hpp"
#include "campaign/Watchtower.hpp"
#include "entity/Ally.hpp"
#include "campaign/Waypoint.hpp"
#include "entity/Merchant.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "hero/Hero.hpp"
#include "item/Upgrade.hpp"
#include "item/skill/Skill.hpp"
#include "item/tree/Tree.hpp"
#include "level/Level.hpp"
#include "level/World.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Drift.hpp"
#include "particle/Spark.hpp"
#include "level/Prop.hpp"

namespace {

double townX = 0, townY = 0;
float threatScale = 1;

bool burning(int tx, int ty) {
  const Edits::Change *c = world::edits ? world::edits->at(tx, ty) : nullptr;
  return c && c->placed && (c->piece == Piece::Torch || c->piece == Piece::Campfire);
}

bool torchlit(double x, double y) {
  int cx = int(std::floor(x / 16)), cy = int(std::floor(y / 16));
  for (int ty = cy - 7; ty <= cy + 7; ty++)
    for (int tx = cx - 7; tx <= cx + 7; tx++)
      if (burning(tx, ty) && (tx - cx) * (tx - cx) + (ty - cy) * (ty - cy) <= 49)
        return true;
  return false;
}

static_assert(std::tuple_size_v<decltype(Campaign::rep)> == FAMILIES);

constexpr float SIEGE = 400;

bool besieged(double x, double y) {
  for (int i = 0; i < bastion::count(); i++) {
    const bastion::Field &f = bastion::field(i);
    if (x > f.west - SIEGE && x < f.east + SIEGE && y > f.north - SIEGE && y < f.south + SIEGE)
      return true;
  }
  return false;
}

float areaThreat(double x, double y) {
  double xd = x - townX, yd = y - townY;
  return float(std::sqrt(xd * xd + yd * yd) / 8) * threatScale + (threatScale - 1) * 300;
}

} // namespace

RunRules Campaign::rules() const {
  RunRules r;
  r.nightLength = NIGHT, r.dayLength = DAY, r.endsAtDawn = false;
  r.loot = true, r.bosses = false, r.town = true, r.omen = -1, r.dayRate = DAY_RATE;
  r.safe.push_back({xTown, yTown, SAFE});
  r.threat = areaThreat, r.warded = torchlit, r.feuds = true, r.matter = true, r.synod = true, r.mutate = true, r.sieged = besieged;
  r.route = bastion::route, r.wall = bastion::wall;
  if (layer) {
    r.sky = false, r.waves = false, r.nightLength = 1e9f, r.dayLength = 0, r.dayRate = 0;
    r.spawnRate = 0.6f, r.safe.clear(), r.route = nullptr, r.wall = nullptr;
  }
  return r;
}

void Campaign::capture(const Level &level) {
  const Player &p = *level.player;
  lvl = p.lvl, exp = p.exp, expNext = p.expNext, gold = p.gold;
  for (int m = 0; m < MATS; m++)
    materials[m] = p.materials[m];
  bag = p.bag, gear = p.gear, worn = p.worn;
  x = p.x, y = p.y, day = level.night.nights, layer = int(world::layer());
  dread = level.dread, scrutiny = level.scrutiny, mutations = level.player->mutations,
  afflictions = level.player->afflictions;
  std::copy(level.rep.begin(), level.rep.end(), rep.begin());
  slain.insert(level.felled.begin(), level.felled.end());
  active = p.skill ? p.skill->name : "";
  upgrades.clear();
  for (const auto &up : p.upgrades) {
    int own = up->level;
    for (int s = 0; s < GEAR; s++)
      if (p.worn[s] && p.gear[s].grants() && up.get() == p.upgrade(p.gear[s].grants()))
        own -= p.gearLevels[s];
    if (own > 0)
      upgrades.emplace_back(up->name, own);
  }
}

void Campaign::enter(Level &level) {
  world::edits = &editsOf(int(world::layer()));
  std::copy(rep.begin(), rep.end(), level.rep.begin());
  level.scrutiny = scrutiny;
  if (world::layer() != world::Layer::Surface)
    return underground(level);
  dread = 0;
  if (!founded)
    found(level);
  Delve::unseal(edits);
  delve = Delve{};
  companion = entt::null;
  barricade();
  threatScale = float(1 + nightmare);
  townX = xTown, townY = yTown;
  level.rules.safe.clear();
  for (const Town &t : towns())
    level.rules.safe.push_back({t.x, t.y, SAFE});
  for (const auto &[k, e] : edits.changes)
    if (e.placed && e.piece == Piece::Campfire) {
      double fx = int(int32_t(k >> 32)) * 16 + 8.0, fy = int(int32_t(k & 0xFFFFFFFFu)) * 16 + 16.0;
      double xd = fx - xTown, yd = fy - yTown;
      if (xd * xd + yd * yd > harvest::TOWN * harvest::TOWN)
        level.rules.safe.push_back({fx, fy, harvest::CAMPFIRE_SAFE});
    }
  regrow(day);
  level.night.dawn = true, level.night.dayEnds = level.time + DAY;
  level.night.nights = day;
  restore(level);
  raise(level);
}

void Campaign::awaken(Level &level) {
  nightmare++, act = 1, quest = 0, progress = 0, qsite = false;
  stage = QuestStage::Offered;
  threatScale = float(1 + nightmare);
  delves.clear();
  staff(level);
  level.game.announce(say(lang.nightmareReturns, nightmare));
}

void Campaign::fall(Level &level) {
  delve.abort(level, *this);
  if (const QuestDef *q = quests::current(*this);
      q && q->goal == Objective::Fetch && stage == QuestStage::Ready) {
    stage = QuestStage::Active, progress = 0, cache = entt::null;
    level.game.announce(lang.reliquarySlips);
  }
  Player &p = *level.player;
  int lost = p.gold / 10;
  p.gold -= lost;
  p.hp = p.maxHp, p.invuln = 3;
  double bx = home().x, by = home().y + 20, best = std::hypot(bx - p.x, by - p.y);
  bool fire = false;
  for (const auto &[k, e] : edits.changes) {
    int tx = int(int32_t(k >> 32)), ty = int(int32_t(k & 0xFFFFFFFFu));
    if (!e.placed || e.piece != Piece::Campfire || founding(tx, ty))
      continue;
    double fx = tx * 16 + 8.0, fy = ty * 16 + 30.0, d = std::hypot(fx - p.x, fy - p.y);
    if (d < best)
      best = d, bx = fx, by = fy, fire = true;
  }
  p.x = bx, p.y = by;
  p.status.clear();
  level.add<Shockwave>(p.x, p.bodyY(), 5, 50, 0.6f, Color{0.5f, 0.85f, 1, 1});
  const char *where = fire ? lang.wakeCampfire : lang.wakeWaypoint;
  level.game.announce(lost > 0 ? say(lang.wakeLost, where, lost) : std::string(where));
}

void Campaign::regrow(int today) {
  for (auto it = edits.changes.begin(); it != edits.changes.end();)
    if (!it->second.placed && it->second.regrow > 0 && it->second.regrow <= float(today))
      it = edits.changes.erase(it);
    else
      ++it;
}

void Campaign::smoulder(Level &level) {
  if ((smoke -= DT) > 0 || !Drift::room())
    return;
  smoke = 0.22f;
  Town near = nearest(level.player->x, level.player->y);
  double cx = std::floor(near.x / 16) * 16 + 8, cy = std::floor(near.y / 16) * 16 + 16;
  auto plume = [&](double px, double py, float shade) {
    Drift *d = level.add<Drift>(px + rng.range(-3, 3), py, rng.range(-4, 4), -rng.range(10, 16),
                                atlas::pt::Puff, Color{shade, shade, shade + 0.03f, 0.55f},
                                rng.range(2.4f, 3.4f));
    d->grow = 1.1f, d->sway = 5, d->drag = 0.4f, d->spin = rng.range(-0.6f, 0.6f);
  };
  plume(cx + 3 * 16, cy - 3 * 16 - 26, 0.32f);
  if (rng.chance(0.45f))
    plume(cx, cy + 2 * 16 - 8, 0.45f);
  if (rng.chance(0.3f)) {
    Drift *e = level.add<Drift>(cx + 3 * 16 + rng.range(-5, 5), cy - 3 * 16 - 20,
                                rng.range(-10, 10), -rng.range(22, 36), atlas::pt::Ember,
                                Color{1, 0.55f, 0.2f, 1}, rng.range(0.7f, 1.3f));
    e->sway = 8, e->drag = 0.6f;
  }
}

void Campaign::lure(Level &level) {
  if (level.night.dawn || delve.active || (shadeTime -= DT) > 0)
    return;
  shadeTime = SHADE_GAP;
  int shades = 0;
  level.each<Enemy>([&](Enemy &e) { shades += e.is(EnemyKind::Shade); });
  const Player &p = *level.player;
  int px = int(std::floor(p.x / 16)), py = int(std::floor(p.y / 16)), lx = 0, ly = 0;
  bool found = false;
  for (int ty = py - 20; ty <= py + 20 && !found; ty++)
    for (int tx = px - 20; tx <= px + 20 && !found; tx++)
      if (burning(tx, ty))
        lx = tx, ly = ty, found = true;
  if (!found || shades >= MAX_SHADES)
    return;
  for (int i = 0; i < 12; i++) {
    double a = rng.range(0, TAU), sx = lx * 16 + 8 + std::cos(a) * 240,
           sy = ly * 16 + 16 + std::sin(a) * 240;
    if (!level.rules.safeAt(sx, sy) && level.spawner.spawn(EnemyKind::Shade, sx, sy, true))
      return;
  }
}

void Campaign::stray(Level &level) {
  level.vendor.discount = discount();
  Player &p = *level.player;
  std::vector<Town> all = towns();
  bool lost = false;
  level.each<Townsfolk>([&](Townsfolk &f) {
    if (f.mode == Townsfolk::Mode::Home)
      return;
    lost = true;
    if (f.mode != Townsfolk::Mode::Following)
      return;
    for (size_t i = 0; i < all.size(); i++)
      if (std::hypot(f.x - all[i].x, f.y - all[i].y) < HOME) {
        f.settle(all[i].x, all[i].y, int(i));
        rescued++;
        for (int k = 0; k < 3; k++) {
          Mat m = Mat(rng.irange(0, int(Mat::Bone)));
          p.materials[int(m)]++;
        }
        level.vendor.discount = discount();
        level.add<Shockwave>(f.x, f.y - 6, 4, 40, 0.5f, col::gold, false);
        level.game.announce(say(lang.seekerJoins, rescued, int(discount() * 100 + 0.5f)));
        if (rescued >= HOUND_AFTER && !hound) {
          hound = true;
          level.game.announce(lang.hunterHound);
        }
        level.game.saveCampaign();
        return;
      }
  });
  if (lost || !level.night.dawn || delve.active || (lostTime -= DT) > 0)
    return;
  lostTime = LOST_GAP;
  for (int i = 0; i < 24; i++) {
    double a = rng.range(0, TAU), r = rng.range(420, 700);
    double lx = p.x + std::cos(a) * r, ly = p.y + std::sin(a) * r;
    int tx = int(std::floor(lx / 16)), ty = int(std::floor(ly / 16));
    Prop prop;
    if (world::isWater(tx, ty) || level.rules.safeAt(lx, ly) || world::propAt(tx, ty, prop))
      continue;
    constexpr int FOLK[3] = {85, 88, 98};
    auto *f = level.add<Townsfolk>(lx, ly, lx, ly, FOLK[rng.irange(0, 2)], std::max(0, act - 1),
                                   rng.irange(0, 3));
    f->mode = Townsfolk::Mode::Lost;
    level.game.announce(lang.criesForHelp);
    return;
  }
}

void Campaign::heel(Level &level) {
  if (!hound)
    return;
  Player &p = *level.player;
  if (companion != entt::null && !level.reg.valid(companion)) {
    companion = entt::null, houndDown = true;
    level.game.announce(lang.houndLimps);
  }
  if (houndDown || p.dead())
    return;
  float bite = 6 + 2.0f * p.lvl;
  if (Ally *dog = level.get<Ally>(companion)) {
    dog->damage = bite;
    if (std::hypot(dog->x - p.x, dog->y - p.y) > 260) {
      Spark::burst(level, dog->x, dog->y - 6, Color{0.6f, 0.55f, 0.5f, 1}, 6, 40, true);
      dog->x = p.x - p.facing * 16, dog->y = p.y + 6;
      level.collide(dog->x, dog->y, dog->radius);
      Spark::burst(level, dog->x, dog->y - 6, Color{0.6f, 0.55f, 0.5f, 1}, 6, 40, true);
    }
    return;
  }
  if (companion != entt::null)
    return;
  Ally *dog = summon(level, AllyKind::Wolf, p.x + 14, p.y + 4, bite, 1e9f);
  dog->hp = dog->maxHp = dog->def.hp * (2 + 0.15f * p.lvl);
  companion = dog->id;
}

void Campaign::honour(Level &level) {
  int fell = 0;
  level.each<Sentry>([&](Sentry &s) { fell += s.kills, s.kills = 0; });
  if (fell <= 0)
    return;
  Player &p = *level.player;
  int reward = fell * HONOUR_GOLD;
  p.gold += reward;
  p.gainXp(fell * HONOUR_XP);
  level.game.announce(say(lang.bastionHeld, fell, reward));
}

void Campaign::underground(Level &level) {
  delve = Delve{};
  companion = entt::null;
  threatScale = float(1 + nightmare);
  townX = xTown, townY = yTown;
  level.rules.safe.clear();
  for (const auto &[k, e] : editsOf(layer).changes)
    if (e.placed && e.piece == Piece::Campfire)
      level.rules.safe.push_back({int(int32_t(k >> 32)) * 16 + 8.0,
                                  int(int32_t(k & 0xFFFFFFFFu)) * 16 + 16.0, harvest::CAMPFIRE_SAFE});
  level.night.dawn = false, level.night.nights = day;
  level.dread = dread;
  level.felled.insert(slain.begin(), slain.end());
  restore(level);
}

void Campaign::tick(Level &level) {
  if (world::layer() != world::Layer::Surface)
    return smoulder(level);
  bool dawn = level.night.dawn;
  if (dawn && !wasDay) {
    regrow(level.night.nights);
    level.scrutiny = std::max(0.0f, level.scrutiny - Level::DAWN_PARDON);
    if (hound && houndDown)
      houndDown = false, level.game.announce(lang.houndBack);
    honour(level);
  }
  wasDay = dawn;
  smoulder(level);
  lure(level);
  stray(level);
  heel(level);
  quests::tick(level, *this);
  delve.tick(level, *this);
}

void Campaign::leave() {
  bastion::clear();
  if (world::edits == &edits || world::edits == &below[0] || world::edits == &below[1])
    world::edits = nullptr;
}
