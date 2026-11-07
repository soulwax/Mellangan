#undef NDEBUG
#include <cassert>

#include <cstring>
#include <iostream>

#include "test/Tests.hpp"

#include "Game.hpp"
#include "campaign/Campaign.hpp"
#include "campaign/Harvest.hpp"
#include "entity/Player.hpp"
#include "entity/pickup/ItemDrop.hpp"
#include "level/Level.hpp"
#include "level/Prop.hpp"
#include "level/World.hpp"
#include "level/Noise.h"

namespace {

void look(Game &game) {
  Player &p = *game.level->player;
  Screen &view = game.screen;
  view.xOffset = int(p.x) - view.vw / 2, view.yOffset = int(p.y) - view.vh / 2;
  game.level->scatter(view);
}

int drops(Level &level, Mat m) {
  int n = 0;
  for (auto [id, o] : level.reg.view<Owned>().each())
    if (auto *d = dynamic_cast<ItemDrop *>(o.self.get());
        d && !d->removed && d->drop.kind == DropKind::Material && d->drop.material == int(m))
      n++;
  return n;
}

bool open(Level &level, Campaign &c, int piece, int &tx, int &ty) {
  Player &p = *level.player;
  for (int r = 0; r < 40; r++)
    for (int k = 0; k < 16; k++) {
      p.x = c.xTown + 420 + std::cos(TAU * k / 16) * r * 8;
      p.y = c.yTown + std::sin(TAU * k / 16) * r * 8;
      harvest::aim(p, tx, ty);
      if (harvest::blocked(level, c, piece, tx, ty).empty())
        return true;
    }
  return false;
}

} // namespace

void testGathering() {
  using harvest::yieldOf;
  assert(yieldOf("oak")->mat == Mat::Wood && yieldOf("snowy pine")->mat == Mat::Wood);
  assert(yieldOf("boulder")->mat == Mat::Stone && yieldOf("lichen rock")->mat == Mat::Stone);
  assert(yieldOf("rune stone")->mat == Mat::Rune && yieldOf("amethyst")->mat == Mat::Shard);
  assert(yieldOf("skull")->mat == Mat::Bone && yieldOf("lavender")->mat == Mat::Herb);
  assert(yieldOf("mushroomTree")->mat == Mat::Wood && yieldOf("crystalSpire")->mat == Mat::Shard);
  assert(!yieldOf("tallGrass") && !yieldOf(nullptr));

  Game game;
  Campaign c;
  c.world = "GATHER01";
  game.level = std::make_unique<Level>(game, c.rules());
  Level &level = *game.level;
  c.enter(level);
  level.tick();
  Player &p = *level.player;
  p.god = true;

  bool chopped = false;
  for (int step = 0; step < 30 && !chopped; step++) {
    p.x = c.xTown + 300 + step * 90, p.y = c.yTown + 60;
    look(game);
    for (const Prop &prop : level.visibleProps()) {
      const Yield *y = yieldOf(prop.kind);
      if (!y || y->mat != Mat::Wood)
        continue;
      p.x = prop.x + 8, p.y = prop.y + 2;
      harvest::Target t;
      assert(harvest::target(level, c, t) && t.yield);
      int tx = t.tx, ty = t.ty, wood = drops(level, t.yield->mat);
      for (int i = 1; i < t.yield->hits; i++) {
        assert(harvest::strike(level, c) && !c.edits.at(tx, ty));
        assert(c.chops.at(Edits::key(tx, ty)) == i);
      }
      assert(harvest::strike(level, c));
      const Edits::Change *cut = c.edits.at(tx, ty);
      assert(cut && !cut->placed && cut->regrow == float(level.night.nights + t.yield->days));
      Prop gone;
      world::edits = &c.edits;
      assert(!world::propAt(tx, ty, gone));
      level.tick();
      assert(drops(level, t.yield->mat) == wood + t.yield->amount);
      c.regrow(int(cut->regrow));
      assert(!c.edits.at(tx, ty) && world::propAt(tx, ty, gone) && gone.count > 0);
      chopped = true;
      break;
    }
  }
  assert(chopped);

  p.materials[int(Mat::Wood)] = 3, p.materials[int(Mat::Stone)] = 1;
  int tx, ty;
  assert(open(level, c, 0, tx, ty));
  size_t zones = level.rules.safe.size();
  assert(harvest::build(level, c, 0, tx, ty));
  assert(p.materials[int(Mat::Wood)] == 0 && p.materials[int(Mat::Stone)] == 0);
  assert(c.edits.at(tx, ty)->placed && c.edits.at(tx, ty)->piece == Piece::Campfire);
  assert(level.rules.safe.size() == zones + 1 && level.rules.safeAt(tx * 16 + 8.0, ty * 16 + 12.0));
  assert(harvest::blocked(level, c, 0, tx, ty).rfind("needs", 0) == 0);
  p.materials[int(Mat::Wood)] = 3, p.materials[int(Mat::Stone)] = 1;
  assert(harvest::blocked(level, c, 0, tx, ty) == "something is already built here");
  assert(!harvest::build(level, c, 0, tx, ty));

  look(game);
  p.x = tx * 16 + 8.0 + 10, p.y = ty * 16 + 16.0;
  harvest::Target mine;
  assert(harvest::target(level, c, mine) && mine.placed && mine.tx == tx && mine.ty == ty);
  int wood = drops(level, Mat::Wood);
  assert(harvest::strike(level, c) && !c.edits.at(tx, ty)->placed && c.edits.at(tx, ty)->regrow == 0);
  level.tick();
  assert(drops(level, Mat::Wood) == wood + 1);

  p.materials[int(Mat::Wood)] = 5, p.materials[int(Mat::Stone)] = 2, p.materials[int(Mat::Ore)] = 1;
  assert(open(level, c, 4, tx, ty) && harvest::build(level, c, 4, tx, ty));
  p.x = tx * 16 + 8.0, p.y = ty * 16 + 30.0;
  assert(harvest::benchNear(level, c));
  p.x += 200;
  assert(!harvest::benchNear(level, c));

  p.x = c.xTown, p.y = c.yTown + 2;
  look(game);
  int cx = int(std::floor(c.xTown / 16)), cy = int(std::floor(c.yTown / 16));
  p.x = (cx + 4) * 16 + 8.0 + 6, p.y = (cy + 3) * 16 + 16.0;
  harvest::Target town;
  assert(c.founding(cx + 4, cy + 3) && c.founding(cx, cy + 2) && !c.founding(cx + 1, cy));
  assert(!harvest::target(level, c, town) || !(town.tx == cx + 4 && town.ty == cy + 3));
  p.materials[int(Mat::Wood)] = 2;
  assert(harvest::blocked(level, c, 1, cx + 2, cy).empty() && harvest::build(level, c, 1, cx + 2, cy));
  look(game);
  p.x = (cx + 2) * 16 + 8.0 + 6, p.y = cy * 16 + 16.0;
  assert(harvest::target(level, c, town) && town.placed && town.tx == cx + 2);

  c.edits.place(cx + 40, cy, Piece::Campfire);
  p.x = (cx + 44) * 16, p.y = cy * 16;
  c.fall(level);
  assert(std::abs(p.x - ((cx + 40) * 16 + 8.0)) < 1 && std::abs(p.y - (cy * 16 + 30.0)) < 1);
  p.x = c.xTown + 30, p.y = c.yTown;
  c.fall(level);
  assert(std::abs(p.x - c.xTown) < 1);
  c.capture(level);
  Campaign back;
  assert(Campaign::load(c.save(), back));
  c.leave();
  Game again;
  again.level = std::make_unique<Level>(again, back.rules());
  back.enter(*again.level);
  assert(again.level->rules.safeAt((cx + 40) * 16 + 8.0, cy * 16 + 12.0));
  assert(back.edits.at(cx + 40, cy)->piece == Piece::Campfire);
  back.leave();
  std::cout << "[TEST] Gathering and building: props yield by kind, a tree falls after its hits "
               "and drops wood, then regrows; pieces cost materials, refuse taken spots, take "
               "down for a refund, a campfire guards its ground across a reload, a placed "
               "workbench is found, and the town's own pieces stay: PASS"
            << std::endl;
}

void testVeins() {
  using harvest::yieldOf;
  assert(yieldOf("ore vein")->mat == Mat::Ore && yieldOf("rockPile")->mat == Mat::Stone);
  Rng kept = rng;
  uint32_t salt = noise_salt;
  noise_reseed(0);
  world::openCaves(true);
  world::enter(world::Layer::Caves);
  int veins = 0, vx = 0, vy = 0;
  for (int cy = -3; cy < 3; cy++)
    for (int cx = -3; cx < 3; cx++)
      for (int y = cy * 64; y < cy * 64 + 64; y++)
        for (int x = cx * 64; x < cx * 64 + 64; x++)
          if (Prop v; world::propAt(x, y, v) && v.kind && std::strcmp(v.kind, "ore vein") == 0) {
            assert(!world::isRock(x, y) && (world::inVault(x, y) || world::isRock(x - 1, y) ||
                                            world::isRock(x + 1, y) || world::isRock(x, y - 1)));
            if (!veins++ || (std::abs(x) + std::abs(y) < std::abs(vx) + std::abs(vy)))
              vx = x, vy = y;
          }
  assert(veins > 0);
  world::enter(world::Layer::Surface);
  Prop above;
  bool had = world::propAt(vx, vy, above);

  Game game;
  Campaign c;
  c.world = "CLASSIC", c.layer = 1;
  world::enter(world::Layer::Caves);
  game.level = std::make_unique<Level>(game, c.rules());
  Level &level = *game.level;
  c.enter(level);
  Player &p = *level.player;
  p.god = true;
  p.x = vx * 16 + 8.0, p.y = vy * 16 + 12.0;
  level.tick();
  p.x = vx * 16 + 8.0, p.y = vy * 16 + 12.0;
  look(game);
  harvest::Target t;
  assert(harvest::target(level, c, t) && t.yield && t.yield->mat == Mat::Ore && t.tx == vx && t.ty == vy);
  int ore = drops(level, Mat::Ore);
  for (int i = 0; i < t.yield->hits; i++)
    assert(harvest::strike(level, c));
  level.tick();
  assert(drops(level, Mat::Ore) == ore + t.yield->amount);
  assert(c.below[0].at(vx, vy) && !c.below[0].at(vx, vy)->placed && !c.edits.at(vx, vy));
  Prop gone;
  assert(!world::propAt(vx, vy, gone));

  int tx = 0, ty = 0;
  p.materials[int(Mat::Wood)] = 10;
  bool spot = false;
  for (int r = 1; r < 8 && !spot; r++)
    for (int k = 0; k < 16 && !spot; k++) {
      p.x = vx * 16 + 8.0 + std::cos(TAU * k / 16) * r * 16, p.y = vy * 16 + 12.0 + std::sin(TAU * k / 16) * r * 16;
      harvest::aim(p, tx, ty);
      spot = !world::isRock(tx, ty) && !world::isRock(tx, ty + 1) && harvest::blocked(level, c, 1, tx, ty).empty();
    }
  assert(spot && harvest::build(level, c, 1, tx, ty));
  assert(c.below[0].at(tx, ty) && c.below[0].at(tx, ty)->placed && !c.edits.at(tx, ty));
  p.x = tx * 16 + 8.0, p.y = ty * 16 + 30.0;
  level.rules.spawnRate = 0, level.dread = 0;
  for (int i = 0; i < 60 * 8; i++)
    level.tick(), p.x = tx * 16 + 8.0, p.y = ty * 16 + 30.0;
  float lit = level.dread;
  assert(lit > 1.5f && lit < 2.5f);

  world::enter(world::Layer::Surface);
  world::edits = &c.edits;
  Prop still;
  assert(world::propAt(vx, vy, still) == had);
  game.level.reset();
  world::edits = nullptr;
  world::openCaves(false);
  noise_reseed(salt);
  rng = kept;
  std::cout << "[TEST] Veins: " << veins << " ore veins line the walls of 36 cave cells; one yields " << t.yield->amount
            << " ore in " << t.yield->hits << " strikes and the cut is kept below, not on the surface; a torch built "
            << "below is kept below too and its light holds the dread to " << lit << " in 8 s: PASS" << std::endl;
}
