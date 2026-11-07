#undef NDEBUG
#include <cassert>

#include <iostream>
#include <sstream>

#include "test/Tests.hpp"

#include "Game.hpp"
#include "campaign/Campaign.hpp"
#include "campaign/Stash.hpp"
#include "campaign/Townsfolk.hpp"
#include "entity/Merchant.hpp"
#include "entity/Player.hpp"
#include "entity/Workbench.hpp"
#include "entity/enemy/Enemy.hpp"
#include "entity/pickup/Chest.hpp"
#include "entity/pickup/Gem.hpp"
#include "particle/Particle.hpp"
#include "entity/enemy/Hazard.hpp"
#include "item/Upgrade.hpp"
#include "item/loot/Loot.hpp"
#include "level/Edits.hpp"
#include "level/Level.hpp"
#include "level/Prop.hpp"
#include "level/World.hpp"
#include "level/Noise.h"

namespace {

bool sameProp(int x, int y, Edits *a, Edits *b) {
  Prop pa, pb;
  world::edits = a;
  bool ha = world::propAt(x, y, pa);
  world::edits = b;
  bool hb = world::propAt(x, y, pb);
  world::edits = nullptr;
  if (ha != hb)
    return false;
  if (!ha)
    return true;
  return pa.count == pb.count && pa.solid == pb.solid &&
         (pa.count == 0 || pa.parts[0].sprite.x == pb.parts[0].sprite.x);
}

void play(Game &game, Campaign &c) {
  game.level = std::make_unique<Level>(game, c.rules());
  c.enter(*game.level);
  game.level->tick();
}

} // namespace

void testCampaign() {
  {
    Edits e;
    e.place(3, -4, Piece::Torch);
    e.harvest(-5, 6, 12.5f);
    Edits back;
    std::istringstream lines(e.save());
    for (std::string line; std::getline(lines, line);)
      assert(back.load(line));
    assert(back.changes.size() == 2 && back.at(3, -4)->placed &&
           back.at(3, -4)->piece == Piece::Torch && !back.at(-5, 6)->placed &&
           back.at(-5, 6)->regrow == 12.5f && !back.at(0, 0));
    assert(!back.load("edit q 1 2 3") && !back.load("nonsense"));
  }
  {
    int tx = 0, ty = 0;
    Prop p;
    bool found = false;
    for (int r = 0; r < 60 && !found; r++)
      for (int i = -r; i <= r && !found; i++)
        if (world::propAt(i, r, p) && p.count > 0)
          tx = i, ty = r, found = true;
    assert(found);
    Edits e;
    world::edits = &e;
    e.harvest(tx, ty);
    assert(!world::propAt(tx, ty, p));
    e.place(tx, ty, Piece::Campfire);
    assert(world::propAt(tx, ty, p) && p.count == 1 && p.light.r > 0 && p.solid > 0);
    e.clear(tx, ty);
    assert(world::propAt(tx, ty, p) && p.count > 0);
    world::edits = nullptr;
  }

  Game game;
  Campaign first;
  first.world = "TESTWRLD", first.hero = 0;
  play(game, first);
  Level &level = *game.level;
  Player &p = *level.player;
  assert(first.founded && world::edits == &first.edits && !level.rules.endsAtDawn);
  assert(level.rules.safeAt(first.xTown, first.yTown) && level.night.dawn);
  int cx = int(std::floor(first.xTown / 16)), cy = int(std::floor(first.yTown / 16));
  Prop prop;
  assert(world::propAt(cx, cy + 2, prop) && prop.light.r > 0);
  assert(!world::propAt(cx + 1, cy, prop));
  assert(level.rules.threat(first.xTown, first.yTown) < 1 &&
         level.rules.threat(first.xTown + 1600, first.yTown) > 150);
  std::vector<Townsfolk *> folk;
  level.each<Townsfolk>([&](Townsfolk &f) { folk.push_back(&f); });
  assert(folk.size() == 3 && folk[0]->line() && *folk[0]->line());
  p.x = folk[0]->x + 10, p.y = folk[0]->y;
  level.tick();
  int talking = 0;
  for (Townsfolk *f : folk)
    talking += f->talk > 0;
  assert(folk[0]->talk > 0 && talking == 1 && *folk[0]->spoken);
  p.x = first.xTown + 600, p.y = first.yTown;
  level.night.dawn = false;
  for (int i = 0; i < 60 * 12; i++)
    level.tick();
  for (Townsfolk *f : folk)
    assert(std::hypot(f->x - first.xTown, f->y - (first.yTown + 32)) < 45);
  level.night.dawn = true;
  p.x = first.xTown, p.y = first.yTown + 2;
  Merchant *trader = level.get<Merchant>(level.vendor.merchant);
  assert(trader && level.get<Workbench>(level.vendor.bench) && level.vendor.restocks == 1);
  assert(level.rules.safeAt(trader->x, trader->y) && level.rules.dayRate > 0);
  double tx = trader->x;
  level.night.lit = 1;
  Merchant::visit(level);
  assert(level.get<Merchant>(level.vendor.merchant)->x == tx);
  level.night.lit = 0;
  p.god = true;
  for (int i = 0; i < 900 && level.count<Enemy>() == 0; i++)
    level.tick();
  assert(level.night.dawn && level.count<Enemy>() > 0);
  p.god = false;

  p.gold = 500, p.materials[int(Mat::Rune)] = 4;
  Rng r;
  r.state = 3;
  for (int i = 0; i < 3; i++)
    p.bag.push_back(rollItem(30, 0, false, r, Rarity::Magic));
  Item armour;
  for (int i = 0; i < 200 && armour.gearSlot() != GearArmour; i++)
    armour = rollItem(20, 0, false, r, Rarity::Rare);
  assert(armour.gearSlot() == GearArmour);
  p.bag.push_back(armour);
  assert(p.equip(p.bag.size() - 1) && p.worn[GearArmour]);
  Upgrade *learned = nullptr;
  for (auto &up : p.upgrades)
    if (up->level == 0 && !up->skill && !up->maxed()) {
      learned = up.get();
      break;
    }
  assert(learned);
  learned->levelUp(p), learned->levelUp(p);
  first.stash.push_back(rollItem(10, 0, false, r, Rarity::Rare));
  first.edits.place(cx + 20, cy + 20, Piece::Torch);
  first.edits.harvest(cx - 20, cy - 20, 30);
  p.x += 37, p.y -= 11;
  first.capture(level);
  std::string text = first.save();

  Campaign second;
  assert(Campaign::load(text, second));
  assert(second.world == "TESTWRLD" && second.gold == 500 && second.founded &&
         second.materials[int(Mat::Rune)] == 4 && second.bag.size() == 3 &&
         second.stash.size() == 1 && second.worn[GearArmour] &&
         second.gear[GearArmour].name() == armour.name());
  assert(second.edits.changes.size() == first.edits.changes.size());
  for (const auto &[k, c] : first.edits.changes) {
    const Edits::Change &d = second.edits.changes.at(k);
    assert(d.placed == c.placed && d.piece == c.piece && std::abs(d.regrow - c.regrow) < 0.06f);
  }
  for (int y = cy - 22; y <= cy + 22; y++)
    for (int x = cx - 22; x <= cx + 22; x++)
      assert(sameProp(x, y, &first.edits, &second.edits));
  std::string name = learned->name;
  int levels = learned->level;
  first.leave();

  Game again;
  play(again, second);
  Player &q = *again.level->player;
  assert(q.gold == 500 && q.materials[int(Mat::Rune)] == 4 && q.bag.size() == 3);
  assert(q.worn[GearArmour] && q.gear[GearArmour].name() == armour.name());
  assert(q.upgrade(name)->level == levels);
  assert(std::abs(q.x - p.x) < 0.2 && std::abs(q.y - p.y) < 0.2);
  assert(world::edits == &second.edits);
  assert(Campaign::load(second.save(), first) && first.edits.changes.size() == second.edits.changes.size());

  q.x = second.xTown + 40, q.y = second.yTown;
  again.campaign = std::make_unique<Campaign>(second);
  again.campaign->chests = second.chests;
  assert(Stash::near(*again.level));
  q.hp = 0;
  second.fall(*again.level);
  assert(q.gold == 450 && q.hp == q.maxHp && std::abs(q.x - second.xTown) < 1);
  again.campaign.reset();
  second.leave();
  assert(!world::edits);
  std::cout << "[TEST] Campaign: edits save, load and reshape props; a founded town has its "
               "fence, fire and safe zone; hero, gear, bag, stash, upgrades and every edited "
               "prop survive a save and reload; falling wakes you by the waypoint for a tenth "
               "of your gold: PASS"
            << std::endl;
}

void testCaveTravel() {
  Rng kept = rng;
  uint32_t salt = noise_salt;
  Game game;
  game.campaign = std::make_unique<Campaign>();
  Campaign &c = *game.campaign;
  c.world = "CLASSIC", c.hero = 0;
  game.worldMode = WorldMode::Classic, game.seedCode.clear();
  noise_reseed(0);
  world::openCaves(true);
  world::enter(world::Layer::Surface);
  game.level = std::make_unique<Level>(game, c.rules());
  c.enter(*game.level);
  world::CaveCell cell{};
  bool found = false;
  for (int r = 0; r < 12 && !found; r++)
    for (int cy = -r; cy <= r && !found; cy++)
      for (int cx = -r; cx <= r && !found; cx++)
        found = (std::abs(cx) == r || std::abs(cy) == r) && world::caveCell(cx, cy, cell) &&
                cell.kind == world::CaveKind::Through;
  assert(found);
  int mx = cell.mx[0], my = cell.my[0];
  Prop prop;
  assert(world::propAt(mx, my, prop) && prop.count == 1);
  Player *p = game.level->player;
  p->x = mx * 16 + 8.0, p->y = my * 16 + 12.0;
  int tx = 0, ty = 0;
  assert(world::caveMouthNear(p->x, p->y, tx, ty) && tx == mx && ty == my);
  int lvl = p->lvl = 7;
  c.capture(*game.level);
  game.travel(world::Layer::Caves, mx * 16 + 8.0, my * 16 + 12.0);
  Level &below = *game.level;
  p = below.player;
  assert(world::layer() == world::Layer::Caves && c.layer == 1 && world::edits == &c.below[0]);
  assert(!below.rules.sky && !below.rules.endsAtDawn && !below.night.dawn && p->lvl == lvl);
  assert(std::abs(p->x - (mx * 16 + 8.0)) < 1 && !below.rockAt(p->x, p->y));
  assert(world::propAt(mx, my, prop) && prop.light.r > 0);
  below.tick();
  assert(below.chart.charted > 0 && below.chart.at(Chart::cellOf(p->x), Chart::cellOf(p->y)));
  int bx = cell.mx[1], by = cell.my[1];
  below.quarry();
  for (int i = 0; i < 60 * 30 && !(world::caveMouthNear(p->x, p->y, tx, ty) && tx == bx && ty == by); i++) {
    double dx = bx * 16 + 8.0 - p->x, dy = by * 16 + 12.0 - p->y, d = std::sqrt(dx * dx + dy * dy);
    p->x += dx / d * 3, p->y += dy / d * 3;
    below.collide(p->x, p->y, p->radius);
    below.quarry();
  }
  int far = 0;
  for (int y = -40; y <= 40; y++)
    for (int x = -40; x <= 40; x++)
      far += below.rockAt(p->x + x * 16, p->y + y * 16) == world::isRock(int(std::floor(p->x / 16)) + x,
                                                                         int(std::floor(p->y / 16)) + y);
  assert(far == 81 * 81);
  double wx = p->x, wy = p->y;
  for (int i = 0; i < 120; i++) {
    p->x += 4;
    below.collide(p->x, p->y, p->radius);
  }
  assert(!world::isRock(int(std::floor(p->x / 16)), int(std::floor(p->y / 16))));
  p->x = wx, p->y = wy;
  Enemy *in = below.spawner.spawn(EnemyKind::Rat, cell.hx * 16 + 8.0, cell.hy * 16 + 8.0);
  double rx = 0, ry = 0;
  for (int y = 0; y < 64 && !rx; y++)
    for (int x = 0; x < 64 && !rx; x++)
      if (world::isRock(cell.cx * 64 + x, cell.cy * 64 + y))
        rx = (cell.cx * 64 + x) * 16 + 8.0, ry = (cell.cy * 64 + y) * 16 + 8.0;
  p->x = rx, p->y = ry;
  below.quarry();
  assert(below.rockAt(rx, ry) && !below.spawner.spawn(EnemyKind::Rat, rx, ry));
  p->x = wx, p->y = wy;
  below.quarry();
  assert(in);
  p->god = true;
  for (auto &u : p->upgrades)
    u->level = 0;
  p->x = mx * 16 + 8.0, p->y = my * 16 + 12.0;
  below.quarry();
  Enemy *rat = below.spawner.spawn(EnemyKind::Rat, cell.mx[1] * 16 + 8.0, cell.my[1] * 16 + 12.0);
  entt::entity ratId = rat->id;
  double gap = 1e9;
  int steps = 0;
  for (; steps < 60 * 40 && gap > 40; steps++) {
    p->x = mx * 16 + 8.0, p->y = my * 16 + 12.0;
    below.tick();
    Mob *m = below.mob(ratId);
    assert(m && !below.lost(m->x, m->y));
    gap = std::hypot(m->x - p->x, m->y - p->y);
  }
  assert(gap <= 40);
  below.quarry();
  int blockedTiles = 0, routed = 0;
  for (int dy = -30; dy <= 30; dy++)
    for (int dx = -30; dx <= 30; dx++) {
      double qx = std::floor(p->x / 16 + dx) * 16 + 8, qy = std::floor(p->y / 16 + dy) * 16 + 10;
      int here = below.flowAt(qx, qy);
      if (here < 3 || here == UINT16_MAX)
        continue;
      double xw = p->x - qx, yw = p->y - qy, len = std::hypot(xw, yw);
      xw = xw / len * 40, yw = yw / len * 40;
      if (!below.rockAt(qx + xw / 40 * 12, qy + yw / 40 * 12 - 2))
        continue;
      blockedTiles++;
      below.steer(qx, qy, 40, xw, yw);
      double nx = qx + xw / 40 * 12, ny = qy + yw / 40 * 12;
      int next = below.flowAt(nx, ny);
      routed += !below.rockAt(nx, ny - 2) && next >= 0 && next < here && std::abs(std::hypot(xw, yw) - 40) < 0.01;
    }
  assert(blockedTiles >= 5 && routed == blockedTiles);
  c.below[0].place(cell.hx, cell.hy, Piece::Torch);
  game.travel(world::Layer::Surface, mx * 16 + 8.0, my * 16 + 12.0);
  assert(world::layer() == world::Layer::Surface && c.layer == 0 && world::edits == &c.edits);
  assert(game.level->rules.sky && game.level->player->lvl == lvl && !game.level->rockAt(rx, ry));
  std::string text = c.save();
  Campaign back;
  assert(Campaign::load(text, back) && back.layer == 0 && back.below[0].at(cell.hx, cell.hy));
  c.leave();
  game.campaign.reset();
  game.level.reset();
  world::openCaves(false);
  world::enter(world::Layer::Surface);
  noise_reseed(salt);
  rng = kept;
  std::cout << "[TEST] Cave travel: down through a mouth at (" << mx << ", " << my
            << "), rock blocks and refuses spawns, a rat finds the hero through the cave in "
            << steps / 60.0 << " s, all " << blockedTiles
            << " tiles whose straight line meets rock steer downhill, the hero keeps level " << lvl
            << ", and climbs back out with the cave's edits kept: PASS" << std::endl;
}

namespace {

bool fresh(const Particle *p) { return p->maxLife - p->life <= DT * 1.01f; }

bool caveOf(CaveBiome want, world::CaveCell &out) {
  for (int r = 0; r < 40; r++)
    for (int cy = -r; cy <= r; cy++)
      for (int cx = -r; cx <= r; cx++)
        if ((std::abs(cx) == r || std::abs(cy) == r) && world::caveCell(cx, cy, out) &&
            out.biome == want)
          return true;
  return false;
}

} // namespace

void testCaveRules() {
  Rng kept = rng;
  uint32_t salt = noise_salt;
  Game game;
  game.campaign = std::make_unique<Campaign>();
  Campaign &c = *game.campaign;
  c.world = "CLASSIC", c.hero = 0;
  game.worldMode = WorldMode::Classic, game.seedCode.clear();
  noise_reseed(0);
  world::openCaves(true);
  world::enter(world::Layer::Surface);
  game.level = std::make_unique<Level>(game, c.rules());
  c.enter(*game.level);
  world::CaveCell vents{}, crypt{};
  assert(caveOf(CaveBiome::Vents, vents) && caveOf(CaveBiome::Catacombs, crypt));

  game.travel(world::Layer::Caves, vents.mx[0] * 16 + 8.0, vents.my[0] * 16 + 12.0);
  int lx = 0, ly = 0;
  bool hot = false;
  for (int y = vents.cy * world::CAVE_CELL; y < (vents.cy + 1) * world::CAVE_CELL && !hot; y++)
    for (int x = vents.cx * world::CAVE_CELL; x < (vents.cx + 1) * world::CAVE_CELL && !hot; x++)
      if (world::isLava(x, y))
        lx = x, ly = y, hot = true;
  assert(hot);
  Level *below = game.level.get();
  below->rules.spawnRate = 0;
  Player *p = below->player;
  p->god = true;
  for (auto &u : p->upgrades)
    u->level = 0;
  int blasts = 0, near = 0;
  for (int i = 0; i < 60 * 20; i++) {
    p->x = lx * 16 + 8.0, p->y = ly * 16 + 40.0;
    below->tick();
    for (size_t k = 0; k < below->particles.size(); k++)
      if (auto *b = dynamic_cast<Blast *>(below->particles[k].get()); b && fresh(b) && b->by &&
          std::string_view(b->by) == "an erupting vent") {
        blasts++;
        int bx = world::tileOf(b->x), by = world::tileOf(b->y);
        assert(world::isLava(bx, by));
        near += std::abs(bx - lx) <= 10 && std::abs(by - ly) <= 10;
      }
  }
  assert(blasts >= 4 && blasts <= 9 && near == blasts);

  game.travel(world::Layer::Surface, vents.mx[0] * 16 + 8.0, vents.my[0] * 16 + 12.0);
  game.travel(world::Layer::Caves, crypt.mx[0] * 16 + 8.0, crypt.my[0] * 16 + 12.0);
  below = game.level.get();
  below->rules.spawnRate = 0;
  p = below->player;
  p->god = true;
  for (auto &u : p->upgrades)
    u->level = 0;
  int risen = 0, quiet = 0;
  for (int i = 0; i < 60 * 30; i++) {
    p->x = crypt.hx * 16 + 8.0, p->y = crypt.hy * 16 + 8.0;
    int skeletons = 0;
    below->each<Enemy>([&](Enemy &e) { skeletons += e.is(EnemyKind::Skeleton); });
    below->tick();
    int after = 0;
    below->each<Enemy>([&](Enemy &e) { after += e.is(EnemyKind::Skeleton); });
    risen += std::max(0, after - skeletons);
  }
  game.travel(world::Layer::Surface, crypt.mx[0] * 16 + 8.0, crypt.my[0] * 16 + 12.0);
  int made[int(CaveBiome::Count)]{}, healed = 0;
  for (CaveBiome want : {CaveBiome::Roots, CaveBiome::Warrens, CaveBiome::Spores, CaveBiome::Geodes}) {
    world::CaveCell cell{};
    assert(caveOf(want, cell));
    game.travel(world::Layer::Caves, cell.mx[0] * 16 + 8.0, cell.my[0] * 16 + 12.0);
    Level &l = *game.level;
    l.rules.spawnRate = 0;
    Player &h = *l.player;
    h.god = true;
    for (auto &u : h.upgrades)
      u->level = 0;
    int sx = cell.hx, sy = cell.hy;
    Prop pp;
    for (int y = cell.cy * 64; y < cell.cy * 64 + 64; y++)
      for (int x = cell.cx * 64; x < cell.cx * 64 + 64; x++)
        if (world::propAt(x, y, pp) && pp.kind &&
            (want == CaveBiome::Spores ? std::string_view(pp.kind) == "glowshrooms" || std::string_view(pp.kind) == "toadstools"
             : want == CaveBiome::Geodes ? std::string_view(pp.kind) == "crystal" || std::string_view(pp.kind) == "amethyst"
                                         : false) &&
            !world::isRock(x, y + 1))
          sx = x, sy = y + 1;
    for (int i = 0; i < 60 * 20; i++) {
      h.x = sx * 16 + 8.0, h.y = sy * 16 + 8.0, h.xa = 60, h.ya = 0;
      if (want == CaveBiome::Geodes)
        h.hp = h.maxHp - 30;
      float hp = h.hp;
      l.tick();
      healed += want == CaveBiome::Geodes && h.hp > hp;
      for (size_t k = 0; k < l.particles.size(); k++) {
        SpriteObject *o = l.particles[k].get();
        if (auto *pt = dynamic_cast<Particle *>(o); !pt || !fresh(pt))
          continue;
        if (auto *br = dynamic_cast<Bramble *>(o)) {
          assert(want == CaveBiome::Roots && br->x > h.x + 30 && std::abs(br->y - h.y) < 40);
          made[int(want)]++;
        } else if (auto *f = dynamic_cast<FrostPatch *>(o); f && f->kind == StatusKind::Poison) {
          assert(want == CaveBiome::Spores);
          made[int(want)]++;
        } else if (auto *b = dynamic_cast<Blast *>(o); b && b->by && std::string_view(b->by) == "falling rock") {
          assert(want == CaveBiome::Warrens && std::hypot(b->x - h.x, b->y - h.y) < 72);
          made[int(want)]++;
        }
      }
    }
    game.travel(world::Layer::Surface, cell.mx[0] * 16 + 8.0, cell.my[0] * 16 + 12.0);
  }
  assert(made[int(CaveBiome::Roots)] >= 5 && made[int(CaveBiome::Warrens)] >= 2 &&
         made[int(CaveBiome::Spores)] >= 7 && healed >= 5);
  for (int i = 0; i < 60 * 20; i++) {
    game.level->tick();
    for (size_t k = 0; k < game.level->particles.size(); k++)
      if (auto *b = dynamic_cast<Blast *>(game.level->particles[k].get()); b && fresh(b) && b->by &&
          std::string_view(b->by) == "an erupting vent")
        quiet++;
  }
  assert(risen >= 2 && risen <= 8 && quiet == 0);
  c.leave();
  game.campaign.reset();
  game.level.reset();
  world::openCaves(false);
  world::enter(world::Layer::Surface);
  noise_reseed(salt);
  rng = kept;
  std::cout << "[TEST] Cave rules: " << blasts << " vent eruptions in 20 s, all on lava within reach, and "
            << risen << " skeletons rose from the catacomb graves in 30 s; roots " << made[int(CaveBiome::Roots)]
            << " thorns, warrens " << made[int(CaveBiome::Warrens)] << " rockfalls, spores " << made[int(CaveBiome::Spores)]
            << " clouds, geodes healed " << healed << " times: PASS" << std::endl;
}

namespace {

int chestsIn(Level &level) {
  int n = 0;
  for (auto [id, o] : level.reg.view<Owned>().each())
    n += !o.self->removed && dynamic_cast<Chest *>(o.self.get()) != nullptr;
  return n;
}

} // namespace

void testDawnCaves() {
  Rng kept = rng;
  uint32_t salt = noise_salt;
  noise_reseed(0);
  world::openCaves(true);
  world::enter(world::Layer::Surface);
  Game game;
  game.level = std::make_unique<Level>(game, RunRules{});
  Level &level = *game.level;
  Player &p = *level.player;
  p.god = true;
  world::CaveCell cell{};
  assert(caveOf(CaveBiome::Roots, cell));
  int mx = cell.mx[0], my = cell.my[0];
  Prop prop;
  assert(world::propAt(mx, my, prop));
  for (int i = 0; i < 30; i++)
    level.spawner.spawn(EnemyKind::Rat, p.x + 200 + i, p.y);
  level.add<Gem>(float(p.x + 300), float(p.y), 5);
  level.tick();
  int foes = int(level.count<Enemy>());
  level.time = 200;
  int lit = level.night.lit, kills = level.kills = 37, lvl = p.lvl = 6;
  level.chart.unveil(p.x, p.y, 12);
  int surfaceCells = level.chart.charted;
  assert(foes >= 30 && surfaceCells > 0);

  level.shift(world::Layer::Caves, mx * 16 + 8.0, my * 16 + 12.0);
  assert(world::layer() == world::Layer::Caves && level.depth == world::Layer::Caves && !level.rules.sky);
  int alive = 0;
  level.each<Enemy>([&](Enemy &e) { alive += !e.removed; });
  assert(alive == 0);
  assert(level.time == 200 && level.kills == kills && p.lvl == lvl && level.night.lit == lit);
  assert(std::abs(p.x - (mx * 16 + 8.0)) < 1 && !level.rockAt(p.x, p.y));
  assert(level.chart.charted == 0 && level.surfaceChart().charted == surfaceCells);
  int magnet = 0;
  level.each<Gem>([&](Gem &g) { magnet += g.magnet; });
  assert(magnet >= 1);
  level.tick();
  assert(chestsIn(level) == 1 && level.chart.charted > 0);

  for (int i = 0; i < 60 * 10; i++)
    level.tick();
  int walkers = 0;
  level.each<Enemy>([&](Enemy &e) {
    assert(e.type.floats || !world::isRock(world::tileOf(e.x), world::tileOf(e.y - 2)));
    walkers += !e.type.floats;
  });
  float clock = level.time;
  assert(clock > 200 + 9);

  level.shift(world::Layer::Surface, mx * 16 + 8.0, my * 16 + 12.0);
  assert(world::layer() == world::Layer::Surface && level.rules.sky && level.chart.charted == surfaceCells);
  level.tick();
  assert(chestsIn(level) == 0);
  level.shift(world::Layer::Caves, mx * 16 + 8.0, my * 16 + 12.0);
  level.tick();
  assert(chestsIn(level) == 1);
  for (auto [id, o] : level.reg.view<Owned>().each())
    if (dynamic_cast<Chest *>(o.self.get()))
      o.self->remove();
  level.tick();
  level.shift(world::Layer::Surface, mx * 16 + 8.0, my * 16 + 12.0);
  level.shift(world::Layer::Caves, mx * 16 + 8.0, my * 16 + 12.0);
  level.tick();
  assert(chestsIn(level) == 0);
  level.shift(world::Layer::Surface, mx * 16 + 8.0, my * 16 + 12.0);

  game.level.reset();
  world::openCaves(false);
  world::enter(world::Layer::Surface);
  noise_reseed(salt);
  rng = kept;
  std::cout << "[TEST] Dawn-run caves: the hero goes down at (" << mx << ", " << my
            << ") inside the same run (clock, kills, level and shrines kept), the surface foes stay behind, gems follow, "
               "one chest waits at the heart (left below while unopened, never twice once taken), and the surface map comes back: PASS"
            << std::endl;
}

void testDeepFall() {
  Rng kept = rng;
  uint32_t salt = noise_salt;
  noise_reseed(0);
  world::openCaves(true);
  world::enter(world::Layer::Surface);
  world::CaveCell pit{};
  bool found = false;
  for (int r = 0; r < 12 && !found; r++)
    for (int cy = -r; cy <= r && !found; cy++)
      for (int cx = -r; cx <= r && !found; cx++)
        found = world::caveCell(cx, cy, pit) && pit.kind == world::CaveKind::Descent;
  assert(found);
  Game game;
  game.level = std::make_unique<Level>(game, RunRules{});
  Level &level = *game.level;
  Player &p = *level.player;
  level.shift(world::Layer::Caves, pit.mx[0] * 16 + 8.0, pit.my[0] * 16 + 12.0);
  level.rules.spawnRate = 0;
  level.tick();
  assert(level.chasmAt(pit.px * 16 + 8.0, pit.py * 16 + 8.0) && level.lost(pit.px * 16 + 8.0, pit.py * 16 + 10.0));
  auto rat = Enemy::create(EnemyKind::Rat, 1, 1);
  rat->x = pit.px * 16 + 8.0, rat->y = pit.py * 16 + 10.0;
  level.add(std::move(rat));
  level.tick();
  level.tick();
  int rats = 0;
  level.each<Enemy>([&](Enemy &e) { rats += !e.removed; });
  assert(rats == 0);

  float full = p.maxHp;
  p.hp = full;
  p.x = pit.px * 16 + 8.0, p.y = pit.py * 16 + 10.0;
  game.fall();
  assert(world::layer() == world::Layer::Deep && level.depth == world::Layer::Deep && !level.rules.sky);
  assert(p.hp < full && p.hp >= full * 0.7f && !level.rockAt(p.x, p.y - 2));
  level.tick();
  assert(chestsIn(level) == 1);

  level.shift(world::Layer::Caves, pit.mx[0] * 16 + 8.0, pit.my[0] * 16 + 12.0);
  p.hp = 3;
  p.x = pit.px * 16 + 8.0, p.y = pit.py * 16 + 10.0;
  game.fall();
  assert(world::layer() == world::Layer::Deep && p.hp >= 1 && !p.dead());
  game.fall();
  assert(world::layer() == world::Layer::Deep);

  world::DeepCell d{};
  found = false;
  for (int r = 0; r < 12 && !found; r++)
    for (int cy = pit.cy - r; cy <= pit.cy + r && !found; cy++)
      for (int cx = pit.cx - r; cx <= pit.cx + r && !found; cx++)
        found = world::deepCell(cx, cy, d) && d.way != world::Way::None;
  assert(found);
  world::Passage w;
  assert(world::passageNear(d.wx * 16 + 8.0, d.wy * 16 + 12.0, w));
  level.shift(w.to, w.x * 16 + 8.0, w.y * 16 + 12.0);
  assert(world::layer() == w.to && !level.rockAt(p.x, p.y - 2));
  level.shift(world::Layer::Surface, pit.mx[0] * 16 + 8.0, pit.my[0] * 16 + 12.0);

  game.level.reset();
  world::openCaves(false);
  world::enter(world::Layer::Surface);
  noise_reseed(salt);
  rng = kept;
  std::cout << "[TEST] Falling into the Sundark: a rat on the chasm at (" << pit.px << ", " << pit.py
            << ") tumbles in, the hero drops to open deep floor for a quarter of their health and never below 1, "
               "a chest waits below, and a way up (kind "
            << int(w.way) << ") leads out: PASS" << std::endl;
}
