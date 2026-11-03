#include "campaign/Campaign.hpp"

#include <spdlog/fmt/fmt.h>

#include <cmath>

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

float areaThreat(double x, double y) {
  double xd = x - townX, yd = y - townY;
  return float(std::sqrt(xd * xd + yd * yd) / 8) * threatScale + (threatScale - 1) * 300;
}

constexpr int TOWN_W = bastion::W, TOWN_H = bastion::H;

struct Spot {
  int dx, dy;
  Piece piece;
};

const Spot BARRICADE[] = {
    {-1, -TOWN_H, Piece::Cart},          {1, -TOWN_H, Piece::Crates},
    {-1, TOWN_H, Piece::Logs},           {1, TOWN_H, Piece::Sacks},
    {-TOWN_W, -1, Piece::Crates},        {-TOWN_W, 1, Piece::Sacks},
    {TOWN_W, -1, Piece::Logs},           {TOWN_W, 1, Piece::Crates},
    {-1, -TOWN_H - 2, Piece::Thorns},    {1, -TOWN_H - 2, Piece::Thorns},
    {-1, TOWN_H + 2, Piece::Thorns},     {1, TOWN_H + 2, Piece::Thorns},
    {-TOWN_W - 2, -1, Piece::Thorns},    {-TOWN_W - 2, 1, Piece::Thorns},
    {TOWN_W + 2, -1, Piece::Thorns},     {TOWN_W + 2, 1, Piece::Thorns},
    {2, TOWN_H + 1, Piece::Sign},        {-3, -TOWN_H - 1, Piece::Skull},
    {4, TOWN_H + 2, Piece::Ribs},        {-TOWN_W - 1, 3, Piece::Skull},
    {TOWN_W + 1, -3, Piece::Thorns},     {-TOWN_W - 1, -3, Piece::Thorns},
    {3, -3, Piece::Charred},             {-5, -4, Piece::Sacks},
    {5, 4, Piece::Crates},               {-5, 4, Piece::Logs},
    {0, 2, Piece::Campfire},
};

void clearing(Edits &edits, double x, double y) {
  int cx = int(std::floor(x / 16)), cy = int(std::floor(y / 16));
  for (int ty = cy - TOWN_H; ty <= cy + TOWN_H; ty++)
    for (int tx = cx - TOWN_W; tx <= cx + TOWN_W; tx++)
      edits.harvest(tx, ty);
}

struct Dressing {
  Piece hedge, debris, ruin;
};

const Dressing DRESS[] = {
    {Piece::Thorns, Piece::Skull, Piece::Charred},
    {Piece::Reeds, Piece::Driftwood, Piece::Charred},
    {Piece::Ice, Piece::Ribs, Piece::Crystal},
    {Piece::Cactus, Piece::Ribs, Piece::Charred},
    {Piece::Embers, Piece::Skull, Piece::Charred},
};

Piece dress(Piece piece, int town) {
  const Dressing &d = DRESS[std::clamp(town, 0, int(std::size(DRESS)) - 1)];
  return piece == Piece::Thorns ? d.hedge
         : piece == Piece::Skull ? d.debris
         : piece == Piece::Charred ? d.ruin
                                   : piece;
}

template <class F> void townLayout(int cx, int cy, F &&fn) {
  int open = bastion::gateOf(cx, cy);
  auto lane = [&](int dx, int dy) {
    switch (open) {
    case bastion::North:
      return dy <= -TOWN_H && std::abs(dx) <= bastion::GAP;
    case bastion::South:
      return dy >= TOWN_H && std::abs(dx) <= bastion::GAP;
    case bastion::West:
      return dx <= -TOWN_W && std::abs(dy) <= bastion::GAP;
    default:
      return dx >= TOWN_W && std::abs(dy) <= bastion::GAP;
    }
  };
  auto put = [&](int dx, int dy, Piece p) { fn(cx + dx, cy + dy, lane(dx, dy) ? Piece::Count : p); };
  put(-TOWN_W, -TOWN_H, Piece::CornerNW), put(TOWN_W, -TOWN_H, Piece::CornerNE);
  put(-TOWN_W, TOWN_H, Piece::CornerSW), put(TOWN_W, TOWN_H, Piece::CornerSE);
  for (int d = 2; d < TOWN_W; d++) {
    Piece west = d == 2 ? Piece::EndEast : Piece::Rail, east = d == 2 ? Piece::EndWest : Piece::Rail;
    put(-d, -TOWN_H, west), put(-d, TOWN_H, west);
    put(d, -TOWN_H, east), put(d, TOWN_H, east);
  }
  for (int d = 2; d < TOWN_H; d++)
    for (int s : {-1, 1})
      put(-TOWN_W, s * d, Piece::Post), put(TOWN_W, s * d, Piece::PostEast);
  for (const Spot &s : BARRICADE)
    put(s.dx, s.dy, s.piece);
  put(0, -TOWN_H, Piece::Logs), put(0, TOWN_H, Piece::Crates);
  put(-TOWN_W, 0, Piece::Sacks), put(TOWN_W, 0, Piece::Logs);
  for (int sx : {-4, 4})
    for (int sy : {-3, 3})
      put(sx, sy, Piece::Torch);
}

} // namespace

RunRules Campaign::rules() const {
  RunRules r;
  r.nightLength = NIGHT, r.dayLength = DAY, r.endsAtDawn = false;
  r.loot = true, r.bosses = false, r.town = true, r.omen = -1, r.dayRate = DAY_RATE;
  r.safe.push_back({xTown, yTown, SAFE});
  r.threat = areaThreat, r.warded = torchlit;
  r.route = bastion::route, r.wall = bastion::wall;
  return r;
}

void Campaign::capture(const Level &level) {
  const Player &p = *level.player;
  lvl = p.lvl, exp = p.exp, expNext = p.expNext, gold = p.gold;
  for (int m = 0; m < MATS; m++)
    materials[m] = p.materials[m];
  bag = p.bag, gear = p.gear, worn = p.worn;
  x = p.x, y = p.y, day = level.night.nights;
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
  world::edits = &edits;
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

void Campaign::found(Level &level) {
  Player &p = *level.player;
  xTown = p.x, yTown = p.y, x = p.x, y = p.y, founded = true;
  clearing(edits, xTown, yTown);
}

Campaign::Town Campaign::nearest(double px, double py) const {
  Town near = home();
  for (const Town &t : towns())
    if (std::hypot(t.x - px, t.y - py) < std::hypot(near.x - px, near.y - py))
      near = t;
  return near;
}

std::vector<Campaign::Town> Campaign::towns() const {
  std::vector<Town> all{{xTown, yTown}};
  all.insert(all.end(), outposts.begin(), outposts.end());
  return all;
}

void Campaign::settle(Level &level, double tx, double ty) {
  Town t{tx, ty};
  outposts.push_back(t);
  clearing(edits, tx, ty);
  barricade();
  level.rules.safe.push_back({tx, ty, SAFE});
  post(level, t);
  staff(level);
  level.add<Shockwave>(tx, ty, 8, 120, 0.8f, col::gold);
}

void Campaign::travel(Level &level, int town) {
  std::vector<Town> all = towns();
  if (town < 0 || town >= int(all.size()))
    return;
  Player &p = *level.player;
  p.x = all[size_t(town)].x, p.y = all[size_t(town)].y + 20;
  level.add<Shockwave>(p.x, p.bodyY(), 5, 50, 0.6f, Color{0.45f, 0.85f, 1, 1});
}

void Campaign::barricade() {
  std::vector<Town> all = towns();
  bastion::clear();
  for (size_t i = 0; i < all.size(); i++) {
    int cx = int(std::floor(all[i].x / 16)), cy = int(std::floor(all[i].y / 16));
    townLayout(cx, cy, [&](int tx, int ty, Piece piece) {
      if (piece == Piece::Count)
        edits.harvest(tx, ty);
      else
        edits.place(tx, ty, dress(piece, int(i)));
    });
    bastion::raise(cx, cy);
  }
}

bool Campaign::founding(int tx, int ty) const {
  bool hit = false;
  for (const Town &t : towns())
    townLayout(int(std::floor(t.x / 16)), int(std::floor(t.y / 16)),
               [&](int x, int y, Piece) { hit |= x == tx && y == ty; });
  return hit;
}

void Campaign::restore(Level &level) {
  Player &p = *level.player;
  p.x = x, p.y = y;
  p.lvl = lvl, p.exp = exp, p.expNext = expNext, p.gold = gold;
  for (int m = 0; m < MATS; m++)
    p.materials[m] = materials[m];
  for (const auto &[name, want] : upgrades) {
    Upgrade *up = p.upgrade(name);
    bool evolved = false;
    for (auto &u : p.upgrades)
      if (!up && u->evolvedName && name == u->evolvedName)
        up = u.get(), evolved = true;
    if (up && up->skill)
      Tree::rank(*up, want);
    else if (up)
      while (up->level < want && !up->maxed())
        up->levelUp(p);
    if (up && evolved && !up->evolved)
      up->evolve();
  }
  if (Upgrade *up = active.empty() ? nullptr : p.upgrade(active); up && up->level > 0)
    up->levelUp(p);
  if (p.skill && p.skill->level == 0)
    Tree::rank(*p.skill, 1);
  p.bag.clear();
  for (int s = 0; s < GEAR; s++)
    if (worn[s])
      p.bag.push_back(gear[s]), p.equip(p.bag.size() - 1);
  p.bag = bag;
  p.hp = p.maxHp;
}

void Campaign::raise(Level &level) {
  waypoints.clear(), chests.clear();
  for (const Town &t : towns())
    post(level, t);
  staff(level);
  quests::begin(level, *this);
}

void Campaign::post(Level &level, const Town &t) {
  bastion::Gate g = bastion::gate(int(std::floor(t.x / 16)), int(std::floor(t.y / 16)));
  level.add<Bridge>(g.x, g.y, g.nx != 0);
  for (int k = 0; k < Sentry::PER_TOWN; k++) {
    double side = (k % 2 ? 1 : -1) * (k < 2 ? 14 : 34), out = k < 2 ? 26 : 12;
    level.add<Sentry>(g.x + g.nx * out - g.ny * side, g.y + g.ny * out + g.nx * side,
                      k % 2 ? 97 : 96);
  }
  waypoints.push_back(level.add<Waypoint>(t.x, t.y - 6)->id);
  chests.push_back(level.add<Stash>(t.x + 40, t.y)->id);
  constexpr int FOLK[3] = {85, 88, 98};
  int town = int(waypoints.size()) - 1;
  if (fortified(town))
    level.add<Watchtower>(t.x + 72, t.y - 8);
  for (int i = 0; i < 3 + (town == 0 ? std::min(rescued, 4) : 0); i++)
    level.add<Townsfolk>(t.x - 30 + (i % 3) * 30, t.y + 30 + (i % 2) * 12 - (i / 3) * 40, t.x, t.y,
                         FOLK[(i + town) % 3], town, i);
}

void Campaign::staff(Level &level) {
  Town h = home();
  if (Elder *old = level.get<Elder>(elder))
    old->remove();
  Merchant::camp(level, h.x - 62, h.y + 34);
  elder = level.add<Elder>(h.x - 36, h.y - 26)->id;
}

int Campaign::spent() const {
  int n = 0;
  for (const auto &[name, level] : upgrades)
    n += level;
  return n;
}

int Campaign::forgetCost() const { return 25 * lvl; }

bool Campaign::fortified(int town) const {
  return std::find(towers.begin(), towers.end(), town) != towers.end();
}

std::string Campaign::fortify(Level &level) {
  int town = std::clamp(act, 1, int(towns().size())) - 1;
  if (fortified(town))
    return "this town already has its watchtower";
  Player &p = *level.player;
  int &stone = p.materials[int(Mat::Stone)], &wood = p.materials[int(Mat::Wood)];
  if (p.gold < TOWER_GOLD || stone < TOWER_STONE || wood < TOWER_WOOD)
    return fmt::format("a watchtower needs {} gold, {} stone and {} wood", TOWER_GOLD,
                       TOWER_STONE, TOWER_WOOD);
  p.gold -= TOWER_GOLD, stone -= TOWER_STONE, wood -= TOWER_WOOD;
  towers.push_back(town);
  Town t = towns()[size_t(town)];
  level.add<Watchtower>(t.x + 72, t.y - 8);
  level.add<Shockwave>(t.x + 72, t.y - 8, 6, 60, 0.6f, col::gold);
  level.game.announce(fmt::format("A WATCHTOWER RISES OVER {}", ACTS[town].town));
  return {};
}

void Campaign::awaken(Level &level) {
  nightmare++, act = 1, quest = 0, progress = 0, qsite = false;
  stage = QuestStage::Offered;
  threatScale = float(1 + nightmare);
  delves.clear();
  staff(level);
  level.game.announce(fmt::format("NIGHTMARE {}  THE DARK RETURNS STRONGER", nightmare));
}

void Campaign::fall(Level &level) {
  delve.abort(level, *this);
  if (const QuestDef *q = quests::current(*this);
      q && q->goal == Objective::Fetch && stage == QuestStage::Ready) {
    stage = QuestStage::Active, progress = 0, cache = entt::null;
    level.game.announce("THE RELIQUARY SLIPS AWAY  IT WAITS IN THE RUINS");
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
  const char *where = fire ? "YOU WAKE BY YOUR CAMPFIRE" : "YOU WAKE BY THE WAYPOINT";
  level.game.announce(lost > 0 ? fmt::format("{}  -{} GOLD", where, lost) : std::string(where));
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
        level.game.announce(fmt::format("A SANCTUARY SEEKER JOINS THE BASTION  {} SHELTERED  THE MERCHANT IS {}% KINDER",
                                        rescued, int(discount() * 100 + 0.5f)));
        if (rescued >= HOUND_AFTER && !hound) {
          hound = true;
          level.game.announce("A GRATEFUL HUNTER GIVES YOU HIS HOUND");
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
    level.game.announce("SOMEONE CRIES FOR HELP OUT THERE");
    return;
  }
}

void Campaign::heel(Level &level) {
  if (!hound)
    return;
  Player &p = *level.player;
  if (companion != entt::null && !level.reg.valid(companion)) {
    companion = entt::null, houndDown = true;
    level.game.announce("YOUR HOUND LIMPS HOME  IT RETURNS AT DAWN");
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
  level.game.announce(fmt::format("THE BASTION HELD  {} FELL AT THE BRIDGE  +{} GOLD", fell, reward));
}

void Campaign::tick(Level &level) {
  bool dawn = level.night.dawn;
  if (dawn && !wasDay) {
    regrow(level.night.nights);
    if (hound && houndDown)
      houndDown = false, level.game.announce("YOUR HOUND IS BACK AT YOUR SIDE");
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
  if (world::edits == &edits)
    world::edits = nullptr;
}
