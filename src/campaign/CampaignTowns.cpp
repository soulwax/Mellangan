#include "campaign/Campaign.hpp"
#include "hero/Mutation.hpp"
#include "hero/Affliction.hpp"
#include "level/Noise.h"

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
  p.changeling = lineage == 1;
  for (int m = 0; m < int(Mutation::Count); m++)
    if (mutations >> m & 1)
      mutation::grant(p, Mutation(m));
  for (int a = 0; a < int(Affliction::Count); a++)
    if (afflictions >> a & 1)
      affliction::grant(p, Affliction(a));
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
    return lang.towerHas;
  Player &p = *level.player;
  int &stone = p.materials[int(Mat::Stone)], &wood = p.materials[int(Mat::Wood)];
  if (p.gold < TOWER_GOLD || stone < TOWER_STONE || wood < TOWER_WOOD)
    return say(lang.towerNeeds, TOWER_GOLD, TOWER_STONE, TOWER_WOOD);
  p.gold -= TOWER_GOLD, stone -= TOWER_STONE, wood -= TOWER_WOOD;
  towers.push_back(town);
  Town t = towns()[size_t(town)];
  level.add<Watchtower>(t.x + 72, t.y - 8);
  level.add<Shockwave>(t.x + 72, t.y - 8, 6, 60, 0.6f, col::gold);
  level.game.announce(say(lang.watchtowerRises, tr(ACTS[town].town)));
  return {};
}
