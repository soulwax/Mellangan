#include "campaign/Harvest.hpp"

#include <spdlog/fmt/fmt.h>

#include <cmath>
#include <cstring>

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "campaign/Campaign.hpp"
#include "entity/Player.hpp"
#include "entity/pickup/ItemDrop.hpp"
#include "level/Level.hpp"
#include "level/Prop.hpp"
#include "level/World.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"

namespace {

const Yield YIELDS[] = {
    {{"rune"}, Mat::Rune, 6, 1, 3},
    {{"crystal", "amethyst", "iceChunk", "Spire"}, Mat::Shard, 4, 2, 2},
    {{"skull", "ribs"}, Mat::Bone, 2, 2, 2},
    {{"oak", "Oak", "pine", "birch", "Birch", "cypress", "acacia", "baobab", "Tree", "fir",
      "Fir", "palm", "willow", "Willow", "bamboo", "redwood", "pruce", "juniper", "larch",
      "stump", "log", "Log", "driftwood", "bush"},
     Mat::Wood, 3, 2, 2},
    {{"mushroom", "Mushroom", "shroom", "toadstool", "sage", "lavender", "heather", "fern",
      "tulips", "poppies", "daisies", "clover", "bluebells", "cornflowers", "roses", "snowdrops",
      "Lilies", "berr", "sunflower", "hydrangea", "ivy", "bracken", "holly", "pumpkins"},
     Mat::Herb, 1, 1, 1},
    {{"oulder", "rock", "Rock", "sandstone", "obsidian", "menhir", "mesa", "hoodoo", "stone",
      "Stone", "pebbles"},
     Mat::Stone, 4, 2, 3},
};

void spill(Level &level, double x, double y, Mat m, int amount) {
  for (int i = 0; i < amount; i++) {
    float a = rng.range(0, TAU), r = rng.range(6, 16);
    level.add<ItemDrop>(x, y - 4, Drop{DropKind::Material, 1, {}, int(m)},
                        x + std::cos(a) * r, y + std::sin(a) * r * 0.7f);
  }
}

const PieceDef *defOf(Piece piece) {
  for (const PieceDef &d : PIECES)
    if (d.piece == piece)
      return &d;
  return nullptr;
}

} // namespace

const PieceDef PIECES[BUILDABLE] = {
    {Piece::Campfire, "Campfire", {0, 0, 0, 0, 0, 0, 3, 1}},
    {Piece::Torch, "Torch", {0, 0, 0, 0, 0, 0, 2, 0}},
    {Piece::Rail, "Fence", {0, 0, 0, 0, 0, 0, 2, 0}},
    {Piece::Wall, "Stone Wall", {0, 0, 0, 0, 0, 0, 0, 3}},
    {Piece::Bench, "Workbench", {0, 1, 0, 0, 0, 0, 5, 2}},
};

namespace harvest {

const Yield *yieldOf(const char *kind) {
  if (!kind)
    return nullptr;
  for (const Yield &y : YIELDS)
    for (const char *needle : y.needles)
      if (needle && std::strstr(kind, needle))
        return &y;
  return nullptr;
}

bool target(const Level &level, const Campaign &c, Target &out) {
  const Player &p = *level.player;
  double best = REACH * REACH;
  bool found = false;
  for (const Prop &prop : level.visibleProps()) {
    double xd = prop.x - p.x, yd = prop.y - p.y, d2 = xd * xd + yd * yd;
    if (d2 >= best)
      continue;
    int tx = int(std::floor(prop.x / 16)), ty = int(std::floor((prop.y - 1) / 16));
    const Edits::Change *edit = c.edits.at(tx, ty);
    bool placed = edit && edit->placed && !c.founding(tx, ty);
    const Yield *y = edit ? nullptr : yieldOf(prop.kind);
    if (!placed && !y)
      continue;
    best = d2, found = true;
    out = {tx, ty, y, placed, prop.x, prop.y};
  }
  return found;
}

bool strike(Level &level, Campaign &c) {
  Target t;
  if (!target(level, c, t))
    return false;
  uint64_t key = Edits::key(t.tx, t.ty);
  if (t.placed) {
    if (const PieceDef *d = defOf(c.edits.at(t.tx, t.ty)->piece))
      for (int m = 0; m < MATS; m++)
        spill(level, t.x, t.y, Mat(m), d->cost[m] / 2);
    c.edits.harvest(t.tx, t.ty);
    Spark::burst(level, t.x, t.y - 6, col::white, 8, 50, true);
    Sound::hurt.play(0.25f, 0.05f, 1.4f);
    return true;
  }
  int &hits = c.chops[key];
  Color tone = MATERIALS[int(t.yield->mat)].color;
  Spark::burst(level, t.x, t.y - 6, tone, 4, 40, false);
  if (++hits < t.yield->hits) {
    Sound::hurt.play(0.18f, 0.08f, 1.6f);
    return true;
  }
  c.chops.erase(key);
  c.edits.harvest(t.tx, t.ty, float(level.night.nights + t.yield->days));
  spill(level, t.x, t.y, t.yield->mat, t.yield->amount);
  level.add<Shockwave>(t.x, t.y - 4, 3, 18, 0.3f, tone, false);
  Sound::gem.play(0.3f, 0.05f, 0.8f);
  return true;
}

void aim(const Player &p, int &tx, int &ty) {
  tx = int(std::floor((p.x + p.facing * 16) / 16));
  ty = int(std::floor((p.y - 2) / 16));
}

std::string blocked(const Level &level, const Campaign &c, int piece, int tx, int ty) {
  const Player &p = *level.player;
  const PieceDef &d = PIECES[piece];
  for (int m = 0; m < MATS; m++)
    if (p.materials[m] < d.cost[m])
      return fmt::format("needs {} {}", d.cost[m],
                         d.cost[m] == 1 ? MATERIALS[m].name : MATERIALS[m].plural);
  Structure site;
  Prop prop;
  if (world::isWater(tx, ty) || world::isRamp(tx, ty))
    return "cannot build on water or stairs";
  if (world::structureAt(tx, ty, site, 1))
    return "too close to a ruin or house";
  if (c.edits.at(tx, ty) && c.edits.at(tx, ty)->placed)
    return "something is already built here";
  if (world::propAt(tx, ty, prop) && prop.count > 0)
    return "clear this spot first (E)";
  return {};
}

bool build(Level &level, Campaign &c, int piece, int tx, int ty) {
  if (piece < 0 || piece >= BUILDABLE || !blocked(level, c, piece, tx, ty).empty())
    return false;
  Player &p = *level.player;
  const PieceDef &d = PIECES[piece];
  for (int m = 0; m < MATS; m++)
    p.materials[m] -= d.cost[m];
  c.edits.place(tx, ty, d.piece);
  double x = tx * 16 + 8.0, y = ty * 16 + 16.0;
  if (d.piece == Piece::Campfire)
    level.rules.safe.push_back({x, y, CAMPFIRE_SAFE});
  level.add<Shockwave>(x, y - 4, 3, 22, 0.35f, col::gold, false);
  Sound::confirm.play(0.4f, 0, 0.9f);
  return true;
}

bool affordable(const Player &p, int piece) {
  for (int m = 0; m < MATS; m++)
    if (p.materials[m] < PIECES[piece].cost[m])
      return false;
  return true;
}

bool benchNear(const Level &level, const Campaign &c) {
  const Player &p = *level.player;
  int px = int(std::floor(p.x / 16)), py = int(std::floor(p.y / 16));
  for (int ty = py - 2; ty <= py + 2; ty++)
    for (int tx = px - 2; tx <= px + 2; tx++)
      if (const Edits::Change *e = c.edits.at(tx, ty); e && e->placed && e->piece == Piece::Bench) {
        double xd = tx * 16 + 8.0 - p.x, yd = ty * 16 + 16.0 - p.y;
        if (xd * xd + yd * yd < 30 * 30)
          return true;
      }
  return false;
}

} // namespace harvest
