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
#include "entity/Faction.hpp"
#include "particle/Arc.hpp"
#include "entity/enemy/SynodFoe.hpp"
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

bool deepSpot(CaveBiome want, std::initializer_list<const char *> kinds, int off, double &hx, double &hy) {
  for (int r = 0; r < 30; r++)
    for (int cy = -r; cy <= r; cy++)
      for (int cx = -r; cx <= r; cx++) {
        world::DeepCell d;
        if ((std::abs(cx) != r && std::abs(cy) != r) || !world::deepCell(cx, cy, d) || d.biome != want)
          continue;
        if (!kinds.size())
          return hx = d.hx * 16 + 8.0, hy = d.hy * 16 + 12.0, true;
        for (int y = cy * 64 + 4; y < cy * 64 + 60; y++)
          for (int x = cx * 64 + 4; x < cx * 64 + 60; x++) {
            Prop p;
            bool hit = false;
            for (const char *k : kinds)
              hit |= *k == '~'   ? world::isWater(x, y)
                     : *k == '^' ? world::isLava(x, y)
                                 : world::propAt(x, y, p) && p.kind && std::string_view(p.kind) == k;
            int fy = y + off;
            if (hit && !world::isRock(x, fy) && !world::isWater(x, fy) && !world::isLava(x, fy) &&
                !world::isRock(x, fy - 1) && !world::isRock(x - 1, fy) && !world::isRock(x + 1, fy))
              return hx = x * 16 + 8.0, hy = fy * 16 + 12.0, true;
          }
      }
  return false;
}

void testDeepRules() {
  Rng kept = rng;
  uint32_t salt = noise_salt;
  noise_reseed(0);
  world::openCaves(true);
  world::enter(world::Layer::Deep);
  Game game;
  game.level = std::make_unique<Level>(game, RunRules{});
  Level &level = *game.level;
  Player &p = *level.player;
  p.god = true;
  for (auto &u : p.upgrades)
    u->level = 0;
  double sx, sy;
  assert(deepSpot(CaveBiome::Glowcap, {}, 0, sx, sy));
  level.shift(world::Layer::Deep, sx, sy);
  level.rules.spawnRate = 0;
  auto stay = [&](CaveBiome b, std::initializer_list<const char *> kinds, int off, float seconds) {
    assert(deepSpot(b, kinds, off, sx, sy));
    level.each<Enemy>([](Enemy &e) { e.remove(); });
    p.x = sx, p.y = sy, p.xa = p.ya = 0;
    level.caveTime = 0.5f;
    level.tick();
    assert(world::caveBiomeAt(world::tileOf(p.x), world::tileOf(p.y)) == b);
    int blasts = 0;
    float most = 0;
    for (int i = 0; i < int(seconds * 60); i++) {
      level.tick();
      most = std::max(most, float(std::abs(p.x - sx)));
      for (const auto &q : level.particles)
        if (auto *k = dynamic_cast<Blast *>(q.get()); k && fresh(k))
          blasts++;
    }
    return std::pair{blasts, most};
  };
  auto foes = [&](EnemyKind k) {
    int n = 0;
    level.each<Enemy>([&](Enemy &e) { n += e.is(k); });
    return n;
  };
  stay(CaveBiome::Glowcap, {}, 0, 5);
  float glow = level.gloom;
  assert(glow < 0.25f);
  stay(CaveBiome::Mere, {"~"}, 3, 20);
  int stirred = foes(EnemyKind::Drowned) + foes(EnemyKind::Snapjaw);
  assert(stirred >= 1);
  int vents = stay(CaveBiome::Magma, {"^"}, 3, 20).first;
  assert(vents >= 3);
  stay(CaveBiome::Ossuary, {"ribs", "skull"}, 5, 20);
  int risen = foes(EnemyKind::Skeleton);
  assert(risen >= 3);
  stay(CaveBiome::Hive, {"beehive"}, 5, 20);
  int brood = foes(EnemyKind::Spider);
  assert(brood >= 3);
  float blown = stay(CaveBiome::Abyss, {}, 0, 20).second;
  assert(blown > 8);
  float dark = level.gloom;
  assert(dark > 0.5f);
  assert(deepSpot(CaveBiome::Halls, {"rune stone"}, 1, sx, sy));
  p.x = sx, p.y = sy;
  level.tick();
  for (int i = 0; i < 12; i++)
    level.spawner.spawn(EnemyKind::Rat, sx + std::cos(i * 0.52) * 40, sy + std::sin(i * 0.52) * 40, true);
  level.caveTime = 0.1f;
  int feared = 0;
  for (int i = 0; i < 60 * 4; i++) {
    level.tick();
    p.x = sx, p.y = sy;
    int now = 0;
    level.each<Enemy>([&](Enemy &e) { now += e.status.effect(StatusKind::Fear).time > 0; });
    feared = std::max(feared, now);
  }
  assert(feared >= 1);
  level.shift(world::Layer::Surface, sx, sy);
  game.level.reset();
  world::openCaves(false);
  world::enter(world::Layer::Surface);
  noise_reseed(salt);
  rng = kept;
  std::cout << "[TEST] Sundark rules: the Glowcap Forest lifts the gloom to " << glow << ", the Black Mere stirs up "
            << stirred << " drowned and snapjaws, the Magma Halls erupt " << vents << " times, the Ossuary raises "
            << risen << " skeletons round a lingering hero, the Hive hatches " << brood << " spiders, the Chasm's wind "
            << "shoves the hero " << blown << " px and darkens to " << dark << ", and the Lost Halls ward frightens "
            << feared << " rats: PASS" << std::endl;
}

void testDread() {
  Rng kept = rng;
  uint32_t salt = noise_salt;
  noise_reseed(0);
  world::openCaves(true);
  world::enter(world::Layer::Surface);
  Game game;
  RunRules held;
  held.endsAtDawn = false, held.spawnRate = 0;
  game.level = std::make_unique<Level>(game, held);
  Level &level = *game.level;
  Player &p = *level.player;
  p.god = true;
  float up = level.threatAt(p.x, p.y), xpUp = (level.tick(), level.xpMul);
  world::enter(world::Layer::Deep);
  double sx, sy;
  assert(deepSpot(CaveBiome::Glowcap, {}, 0, sx, sy));
  level.shift(world::Layer::Deep, sx, sy);
  level.rules.spawnRate = 0;
  level.tick();
  float down = level.threatAt(p.x, p.y), xpDown = level.xpMul;
  assert(std::abs(down - up - Level::DEPTH_THREAT[2]) < 1 && xpDown > xpUp * 1.3f);
  for (int i = 0; i < 60 * 30; i++)
    level.tick();
  float grown = level.dread;
  assert(grown > 29 && grown < 31 && level.pressure() > 1.09f);
  level.dread = Level::DREAD_MAX / 3 + 1;
  level.tick();
  assert(level.dreadSaid == 1 && level.threatAt(p.x, p.y) > down + 90);

  assert(deepSpot(CaveBiome::Halls, {}, 0, sx, sy));
  p.x = sx, p.y = sy;
  assert(level.sanctum(p.x, p.y) && !level.sanctum(p.x + Level::SANCTUM + 40, p.y));
  assert(!level.spawner.spawn(EnemyKind::Rat, sx + 20, sy, true));
  Prop lamp;
  assert(world::propAt(world::tileOf(sx), world::tileOf(sy) - 1, lamp) && lamp.light.a > 0);
  p.hp = p.maxHp / 2;
  float hurt = p.hp;
  for (int i = 0; i < 60 * 4; i++)
    level.tick(), p.x = sx, p.y = sy;
  assert(level.dread == 0 && p.hp > hurt);

  level.shift(world::Layer::Surface, sx, sy);
  game.level = std::make_unique<Level>(game, RunRules{});
  Level &dawn = *game.level;
  dawn.shift(world::Layer::Deep, sx, sy);
  dawn.rules.spawnRate = 0;
  for (int i = 0; i < 60 * 5; i++)
    dawn.tick();
  assert(dawn.dread == 0);
  dawn.shift(world::Layer::Surface, sx, sy);
  game.level.reset();
  world::openCaves(false);
  world::enter(world::Layer::Surface);
  noise_reseed(salt);
  rng = kept;
  std::cout << "[TEST] Dread: the Sundark adds " << down - up << " threat and pays " << xpDown / xpUp
            << "x the xp; in a campaign the dark gathers " << grown << " dread in 30 s and speaks at a third; a Lost "
            << "Halls sanctuary refuses spawns, burns a lamp, mends the hero and empties the dread; the dawn run "
               "feels none: PASS"
            << std::endl;
}

void testCracks() {
  Rng kept = rng;
  uint32_t salt = noise_salt;
  noise_reseed(0);
  world::openCaves(true);
  world::enter(world::Layer::Surface);
  world::CaveCell cell{};
  bool found = false;
  for (int r = 0; r < 12 && !found; r++)
    for (int cy = -r; cy <= r && !found; cy++)
      for (int cx = -r; cx <= r && !found; cx++)
        found = (std::abs(cx) == r || std::abs(cy) == r) && world::caveCell(cx, cy, cell) && cell.vault;
  assert(found);
  Game game;
  game.level = std::make_unique<Level>(game, RunRules{});
  Level &level = *game.level;
  Player &p = *level.player;
  p.god = true;
  level.shift(world::Layer::Caves, cell.mx[0] * 16 + 8.0, cell.my[0] * 16 + 12.0);
  level.rules.spawnRate = 0;
  assert(world::edits == &level.breaks && world::crackAt(cell.kx, cell.ky));
  int fx = 0, fy = 0;
  for (int d = 0; d < 4; d++) {
    int x = cell.kx + (d == 0) - (d == 1), y = cell.ky + (d == 2) - (d == 3);
    if (!world::isRock(x, y) && !world::inVault(x, y))
      fx = x, fy = y;
  }
  p.x = fx * 16 + 8.0, p.y = fy * 16 + 12.0;
  level.tick();
  assert(level.rockAt(cell.kx * 16 + 8.0, cell.ky * 16 + 8.0) && level.lost(cell.vx * 16 + 8.0, cell.vy * 16 + 10.0));
  Prop mark;
  assert(world::propAt(cell.kx, cell.ky, mark) && mark.kind && std::string_view(mark.kind) == "cracked wall");
  int chests = chestsIn(level);
  game.input.gather.clicked = true;
  assert(game.crack());
  game.input.gather.clicked = false;
  level.tick();
  assert(!world::isRock(cell.kx, cell.ky) && !world::crackAt(cell.kx, cell.ky));
  assert(!level.rockAt(cell.kx * 16 + 8.0, cell.ky * 16 + 8.0) && !level.lost(cell.vx * 16 + 8.0, cell.vy * 16 + 10.0));
  assert(chestsIn(level) == chests + 1);
  int veins = 0;
  for (int y = cell.vy - 2; y <= cell.vy + 2; y++)
    for (int x = cell.vx - 2; x <= cell.vx + 2; x++)
      if (Prop v; world::inVault(x, y) && world::propAt(x, y, v) && v.kind && std::string_view(v.kind) == "ore vein")
        veins++;
  game.input.gather.clicked = true;
  assert(!game.crack());
  game.input.gather.clicked = false;

  level.shift(world::Layer::Surface, cell.mx[0] * 16 + 8.0, cell.my[0] * 16 + 12.0);
  assert(!world::edits);
  level.shift(world::Layer::Caves, cell.mx[0] * 16 + 8.0, cell.my[0] * 16 + 12.0);
  assert(!world::isRock(cell.kx, cell.ky));
  level.shift(world::Layer::Surface, cell.mx[0] * 16 + 8.0, cell.my[0] * 16 + 12.0);
  game.level.reset();
  assert(!world::edits);
  world::openCaves(false);
  world::enter(world::Layer::Surface);
  noise_reseed(salt);
  rng = kept;
  std::cout << "[TEST] Cracked walls: the wall at (" << cell.kx << ", " << cell.ky << ") is marked, blocks, and seals a "
            << "vault no foe can reach; E breaks it once, the rock map opens, a chest waits inside beside " << veins
            << " ore veins, and the break holds for the rest of the run: PASS" << std::endl;
}

void testGuardians() {
  Rng kept = rng;
  uint32_t salt = noise_salt;
  noise_reseed(0);
  world::openCaves(true);
  world::enter(world::Layer::Deep);
  int lairs[int(CaveBiome::Count)]{};
  world::DeepCell lair{};
  bool found = false;
  for (int cy = -14; cy < 14; cy++)
    for (int cx = -14; cx < 14; cx++)
      if (world::DeepCell d; world::deepCell(cx, cy, d) && d.lair) {
        lairs[int(d.biome)]++;
        assert(CAVES[int(d.biome)].guardian && d.biome != CaveBiome::Halls);
        if (!found)
          lair = d, found = true;
      }
  assert(found && !lairs[int(CaveBiome::Halls)]);
  Game game;
  game.level = std::make_unique<Level>(game, RunRules{});
  Level &level = *game.level;
  Player &p = *level.player;
  p.god = true;
  double hx = lair.hx * 16 + 8.0, hy = lair.hy * 16 + 12.0;
  level.shift(world::Layer::Deep, hx, hy + 200);
  level.rules.spawnRate = 0;
  auto guardians = [&] {
    int n = 0;
    level.each<Enemy>([&](Enemy &e) { n += e.boss && e.title && !e.def; });
    return n;
  };
  for (int i = 0; i < 30; i++)
    level.tick(), p.x = hx, p.y = hy + 200;
  assert(guardians() == 0);
  p.x = hx, p.y = hy + 32;
  level.tick();
  level.tick();
  assert(guardians() == 1 && level.woken.size() == 1);
  Enemy *g = nullptr;
  level.each<Enemy>([&](Enemy &e) {
    if (e.boss && e.title)
      g = &e;
  });
  const CaveDef &def = CAVES[int(lair.biome)];
  assert(g && std::string_view(g->title) == def.guardian && g->is(def.keeper) &&
         g->maxHp > Enemy::typeOf(def.keeper).hp * 20);
  bool barred = false;
  for (entt::entity id : level.bossIds)
    barred |= id == g->id;
  assert(barred);
  int chests = chestsIn(level);
  g->strike(1e9f, 0, 1, 0);
  level.tick();
  assert(guardians() == 0 && chestsIn(level) == chests + 1);
  for (int i = 0; i < 60; i++)
    level.tick(), p.x = hx, p.y = hy + 40;
  assert(guardians() == 0);
  level.shift(world::Layer::Surface, hx, hy);
  game.level.reset();
  world::openCaves(false);
  world::enter(world::Layer::Surface);
  noise_reseed(salt);
  rng = kept;
  std::cout << "[TEST] Guardians:";
  for (int b = CAVE_SHALLOW; b < int(CaveBiome::Count); b++)
    std::cout << " " << CAVES[b].name << " " << lairs[b];
  std::cout << " lairs in 784 deep cells; " << def.guardian << " wakes when the hero comes near its lair, carries the "
            << "boss bar, falls with one chest and never wakes twice: PASS" << std::endl;
}

void testDepthMemory() {
  Campaign probe;
  probe.world = "CLASSIC", probe.layer = 2, probe.dread = 123.4f;
  probe.slain = {uint64_t(uint32_t(3)) << 32 | uint32_t(-2), uint64_t(uint32_t(-7)) << 32 | uint32_t(11)};
  Campaign back;
  assert(Campaign::load(probe.save(), back) && back.layer == 2 && std::abs(back.dread - 123.4f) < 0.06f &&
         back.slain == probe.slain);
  assert(Campaign::load(probe.save() + "dread 99999\n", back) && back.dread == Level::DREAD_MAX);
  assert(Campaign::load(probe.save() + "dread nan\n", back) && back.dread == 0);

  Rng kept = rng;
  uint32_t salt = noise_salt;
  Game game;
  game.campaign = std::make_unique<Campaign>();
  Campaign &c = *game.campaign;
  c.world = "CLASSIC", c.hero = 0;
  game.worldMode = WorldMode::Classic, game.seedCode.clear();
  noise_reseed(0);
  world::openCaves(true);
  world::enter(world::Layer::Deep);
  world::DeepCell lair{};
  bool found = false;
  for (int r = 0; r < 14 && !found; r++)
    for (int cy = -r; cy <= r && !found; cy++)
      for (int cx = -r; cx <= r && !found; cx++)
        found = world::deepCell(cx, cy, lair) && lair.lair;
  assert(found);
  world::enter(world::Layer::Surface);
  game.level = std::make_unique<Level>(game, c.rules());
  c.enter(*game.level);
  int surfaceLevel = game.level->monsterLevel(game.level->player->x, game.level->player->y);
  double hx = lair.hx * 16 + 8.0, hy = lair.hy * 16 + 12.0;
  game.travel(world::Layer::Deep, hx, hy + 32);
  int deepLevel = game.level->monsterLevel(hx, hy + 32);
  game.level->player->god = true;
  game.level->rules.spawnRate = 0;
  game.level->tick();
  game.level->tick();
  Enemy *g = nullptr;
  game.level->each<Enemy>([&](Enemy &e) {
    if (e.boss && e.title)
      g = &e;
  });
  assert(g);
  g->strike(1e9f, 0, 1, 0);
  game.level->tick();
  game.level->tick();
  uint64_t key = uint64_t(uint32_t(lair.cx)) << 32 | uint32_t(lair.cy);
  assert(game.level->felled.count(key));
  game.level->dread = 200;
  game.travel(world::Layer::Caves, lair.hx * 16 + 8.0, lair.hy * 16 + 12.0);
  assert(c.slain.count(key) && std::abs(game.level->dread - 200) < 1);
  game.travel(world::Layer::Deep, hx, hy + 32);
  game.level->rules.spawnRate = 0;
  for (int i = 0; i < 30; i++)
    game.level->tick();
  int again = 0;
  game.level->each<Enemy>([&](Enemy &e) { again += e.boss && e.title; });
  assert(again == 0 && game.level->felled.count(key));
  Campaign reloaded;
  c.capture(*game.level);
  assert(Campaign::load(c.save(), reloaded) && reloaded.slain.count(key) && reloaded.dread > 199);
  game.travel(world::Layer::Surface, hx, hy);
  assert(game.level->dread == 0 && c.dread == 0 && c.slain.count(key));
  game.level.reset();
  game.campaign.reset();
  world::edits = nullptr;
  world::openCaves(false);
  world::enter(world::Layer::Surface);
  noise_reseed(salt);
  rng = kept;
  std::cout << "[TEST] Depth memory: dread and slain guardians round-trip the save (bad values clamp); monster level "
            << surfaceLevel << " on the surface is " << deepLevel << " in the Sundark; a slain guardian stays slain "
            << "across travel and reload, dread follows the hero between the layers below and daylight clears it: PASS"
            << std::endl;
}

void testFeuds() {
  assert(factionOf(EnemyKind::Wolf) == Faction::Wildkin && factionOf(EnemyKind::Sporeling) == Faction::Mycel &&
         factionOf(EnemyKind::Skeleton) == Faction::BarrowHost && factionOf(EnemyKind(200)) == Faction::Wildkin);
  int fired = 0, again = 0;
  for (int slot = 0; slot < 2000; slot++)
    fired += Level::flares(3, -2, 1, slot), again += Level::flares(3, -2, 1, slot);
  assert(fired == again && fired > 2000 * 0.05 && fired < 2000 * 0.14);
  Grudges g = Grudges::roll(1234), same = Grudges::roll(1234);
  assert(g.count == Grudges::MAX && same.count == g.count);
  bool differs = false;
  for (int i = 0; i < g.count; i++) {
    assert(g.pair[i][0] == same.pair[i][0] && g.pair[i][1] == same.pair[i][1] && g.pair[i][0] != g.pair[i][1]);
    assert(int(g.pair[i][0]) < FAMILIES && int(g.pair[i][1]) < FAMILIES);
  }
  for (uint32_t s = 1; s < 8 && !differs; s++) {
    Grudges o = Grudges::roll(s);
    differs = o.pair[0][0] != g.pair[0][0] || o.pair[0][1] != g.pair[0][1];
  }
  assert(differs);

  Rng kept = rng;
  uint32_t salt = noise_salt;
  noise_reseed(0);
  world::enter(world::Layer::Surface);
  Game game;
  RunRules rules;
  rules.feuds = true, rules.spawnRate = 0, rules.waves = false, rules.bosses = false;
  game.level = std::make_unique<Level>(game, rules);
  Level &level = *game.level;
  Player &p = *level.player;
  p.god = true;
  for (auto &u : p.upgrades)
    u->level = 0;
  level.grudges.count = 1, level.grudges.pair[0][0] = Faction::Wildkin, level.grudges.pair[0][1] = Faction::Mycel;
  level.grudged = true;
  std::vector<entt::entity> wolves, spores;
  for (int i = 0; i < 6; i++) {
    if (Enemy *w = level.spawner.spawn(EnemyKind::Wolf, p.x + 160 + i * 6, p.y - 20 + i * 8))
      wolves.push_back(w->id);
    if (Enemy *s = level.spawner.spawn(EnemyKind::Sporeling, p.x + 220 + i * 6, p.y - 20 + i * 8))
      spores.push_back(s->id);
  }
  level.tick();
  assert(wolves.size() == 6 && spores.size() == 6);
  assert(!RunRules{}.feuds);
  level.tick();
  int brawlers = level.ignite();
  assert(brawlers == 12 && level.ignite() == 0);
  for (int i = 0; i < 60 * 30; i++)
    level.tick();
  auto fallen = [&](const std::vector<entt::entity> &ids) {
    int n = 0;
    for (entt::entity id : ids)
      n += !level.get<Enemy>(id);
    return n;
  };
  int deadWolves = fallen(wolves), deadSpores = fallen(spores);
  assert(deadWolves >= 1 && deadSpores >= 1);
  level.brawlers.clear(), level.night.nights = 1;
  for (int i = 0; i < 300; i++)
    level.flareTime = 0, level.time += Level::FLARE_EVERY, level.feuds();
  assert(level.brawlers.empty());
  game.level.reset();
  noise_reseed(salt);
  rng = kept;
  std::cout << "[TEST] Feuds: factions per kind, grudges rolled from the seed, flare-ups " << fired
            << "/2000 and the same every time; a forced Wildkin-Mycel flare sets " << brawlers << " brawlers on each "
            << "other and in 30 s " << deadWolves << " wolves and " << deadSpores
            << " sporelings fall to the other side; the dawn run has no feuds: PASS" << std::endl;
}

void testReputation() {
  Campaign probe;
  probe.world = "CLASSIC";
  probe.rep = {12, -40, 600, 0, 0, 0, 0, 0, -1000};
  Campaign back;
  assert(Campaign::load(probe.save(), back) && back.rep == probe.rep);
  assert(Campaign::load(probe.save() + "rep 5000 -9000\n", back) && back.rep[0] == Level::REP_MAX &&
         back.rep[1] == -Level::REP_MAX && back.rep[2] == 600);
  Campaign blank;
  blank.world = "CLASSIC";
  assert(blank.save().find("\nrep") == std::string::npos);

  Rng kept = rng;
  uint32_t salt = noise_salt;
  noise_reseed(0);
  world::enter(world::Layer::Surface);
  Game game;
  RunRules rules;
  rules.feuds = true, rules.spawnRate = 0, rules.waves = false, rules.bosses = false;
  game.level = std::make_unique<Level>(game, rules);
  Level &level = *game.level;
  Player &p = *level.player;
  p.god = true;
  level.grudges.count = 1, level.grudges.pair[0][0] = Faction::Wildkin, level.grudges.pair[0][1] = Faction::Mycel;
  level.grudged = true;
  level.rep[int(Faction::Mycel)] = Level::FRIENDLY - 1;
  Enemy *wolf = level.spawner.spawn(EnemyKind::Wolf, p.x + 120, p.y);
  level.tick();
  assert(wolf && !level.friendly(Faction::Mycel));
  wolf->strike(1e9f, 0, 1, 0);
  level.tick();
  assert(level.rep[int(Faction::Wildkin)] == -Level::REP_KILL && level.friendly(Faction::Mycel));
  Enemy *spore = level.spawner.spawn(EnemyKind::Sporeling, p.x + 120, p.y);
  level.tick();
  spore->strike(1e9f, 0, 1, 0);
  level.tick();
  assert(!level.friendly(Faction::Mycel) && level.rep[int(Faction::Wildkin)] == -Level::REP_KILL + Level::REP_RIVAL);
  game.level.reset();

  RunRules plain;
  plain.spawnRate = 0;
  game.level = std::make_unique<Level>(game, plain);
  Level &dawn = *game.level;
  Enemy *rat = dawn.spawner.spawn(EnemyKind::Rat, dawn.player->x + 120, dawn.player->y);
  dawn.tick();
  rat->strike(1e9f, 0, 1, 0);
  dawn.tick();
  assert(dawn.rep[int(Faction::Wildkin)] == 0);
  game.level.reset();
  noise_reseed(salt);
  rng = kept;
  std::cout << "[TEST] Reputation: the nine standings round-trip the save and clamp; a slain wolf costs the wildkin "
            << Level::REP_KILL << " and earns their mycel rivals " << Level::REP_RIVAL
            << ", tipping the mycel to friendly and back when a sporeling falls; the dawn run keeps no standing: PASS"
            << std::endl;
}

namespace {

constexpr int STRIP_Y = 4000, GAP = 10;

bool strip(int x, int y) { return y == STRIP_Y && x >= 0 && x <= 20 && x != GAP; }

} // namespace

void testMatter() {
  Rng kept = rng;
  uint32_t salt = noise_salt;
  noise_reseed(0);
  world::enter(world::Layer::Surface);
  Game game;
  RunRules rules;
  rules.matter = true, rules.spawnRate = 0, rules.waves = false, rules.bosses = false;
  game.level = std::make_unique<Level>(game, rules);
  Level &level = *game.level;
  Player &p = *level.player;
  p.god = true;
  for (auto &u : p.upgrades)
    u->level = 0;

  int tx = world::tileOf(p.x), ty = world::tileOf(p.y - 2);
  level.matter.set(tx + 1, ty, Stuff::Wet, 60);
  assert(level.stuffAt(tx + 1, ty) == Stuff::Wet && level.matter.pageCount() == 1);
  assert(level.react(tx + 1, ty, Cause::Chill) && level.stuffAt(tx + 1, ty) == Stuff::Frozen);
  p.x = (tx + 1) * 16 + 8.0, p.y = ty * 16 + 10.0;
  assert(level.footing(p.x, p.y) == Level::ICE_PACE);
  float hp = p.hp;
  for (int i = 0; i < 60 * 11; i++)
    level.tick();
  assert(level.stuffAt(tx + 1, ty) == Stuff::Frozen && p.hp == hp);
  for (int i = 0; i < 60 * 2; i++)
    level.tick();
  assert(level.stuffAt(tx + 1, ty) == Stuff::Wet);
  for (int i = 0; i < 60 * 7; i++)
    level.tick();
  assert(level.stuffAt(tx + 1, ty) == Stuff::None && level.matter.empty());

  Enemy *rat = level.spawner.spawn(EnemyKind::Rat, p.x + 80, p.y);
  level.tick();
  int rx = world::tileOf(rat->x), ry = world::tileOf(rat->y - 2);
  level.matter.set(rx, ry, Stuff::Wet, 30);
  rat->afflict(StatusKind::Chill, 2, 1);
  bool froze = level.stuffAt(rx, ry) == Stuff::Frozen;
  rat->remove();

  level.matter.fuel = strip;
  assert(level.react(0, STRIP_Y, Cause::Fire) && level.stuffAt(0, STRIP_Y) == Stuff::Burning);
  assert(!level.react(GAP, STRIP_Y + 5, Cause::Fire));
  int reached = 0;
  for (int i = 0; i < 60 * 30; i++) {
    level.tick();
    for (int x = 0; x <= 20; x++)
      if (level.stuffAt(x, STRIP_Y) != Stuff::None)
        reached = std::max(reached, x);
  }
  assert(reached == GAP - 1);
  for (int x = GAP; x <= 20; x++)
    assert(level.stuffAt(x, STRIP_Y) == Stuff::None);
  level.matter.set(3, STRIP_Y, Stuff::Burning, 5);
  p.x = 3 * 16 + 8.0, p.y = STRIP_Y * 16 + 10.0;
  p.god = false, p.invuln = 0;
  float before = p.hp;
  level.matterTick();
  assert(p.status.effect(StatusKind::Burn).time > 0);
  level.matter.set(3, STRIP_Y, Stuff::Wet, 5);
  level.matterTick();
  assert(p.status.effect(StatusKind::Burn).time <= 0 && p.hp <= before);
  p.god = true;
  for (int i = 0; i < 60 * 40; i++)
    level.tick();
  assert(level.matter.empty());
  game.level.reset();

  RunRules plain;
  game.level = std::make_unique<Level>(game, plain);
  assert(!game.level->react(0, 0, Cause::Fire) && game.level->matter.empty());
  game.level.reset();
  noise_reseed(salt);
  rng = kept;
  std::cout << "[TEST] Matter: a wet tile freezes for 12 s, carries the hero at ice pace, thaws to wet and dries; "
            << (froze ? "a chilled rat freezes the puddle under it; " : "")
            << "fire runs along the fuel to tile " << reached
            << " and stops at the gap; a burning tile sets the hero alight and water quenches it; every page is freed; "
               "the dawn run has no matter: PASS"
            << std::endl;
  assert(froze);
}

void testMatterTable() {
  Rng kept = rng;
  uint32_t salt = noise_salt;
  noise_reseed(0);
  world::enter(world::Layer::Surface);
  Game game;
  RunRules rules;
  rules.matter = true, rules.spawnRate = 0, rules.waves = false, rules.bosses = false;
  game.level = std::make_unique<Level>(game, rules);
  Level &level = *game.level;
  Player &p = *level.player;
  p.god = true;
  for (auto &u : p.upgrades)
    u->level = 0;
  constexpr int X = 3000, Y = 3000;
  level.matter.fuel = [](int, int) { return false; };
  for (int y = 0; y < 5; y++)
    for (int x = 0; x < 5; x++)
      level.matter.set(X + x, Y + y, Stuff::Wet, 60);
  level.add<Arc>(std::vector<ArcNode>{{X * 16.0 - 40, Y * 16.0}, {X * 16.0 + 40, Y * 16.0 + 34}}, 0.3f, Color{1, 1, 1, 1});
  level.tick();
  int charged = 0;
  for (int y = -1; y < 6; y++)
    for (int x = -1; x < 6; x++)
      charged += level.stuffAt(X + x, Y + y) == Stuff::Charged;
  assert(charged == 25);

  level.matter.set(X + 20, Y, Stuff::Oiled, 60);
  assert(level.react(X + 20, Y, Cause::Fire));
  int lit = 0;
  for (int y = -1; y <= 1; y++)
    for (int x = 19; x <= 21; x++)
      lit += level.stuffAt(X + x, Y + y) == Stuff::Burning;
  assert(lit == 9);

  level.matter.set(X + 30, Y, Stuff::Wet, 60);
  assert(level.react(X + 30, Y, Cause::Fire) && level.stuffAt(X + 30, Y) == Stuff::None);
  level.matter.set(X + 32, Y, Stuff::Limed, 60);
  assert(!level.react(X + 32, Y, Cause::Fire) && !level.react(X + 32, Y, Cause::Spore));
  p.x = (X + 32) * 16 + 8.0, p.y = Y * 16 + 10.0;
  p.status.apply(StatusKind::Burn, 5, 3);
  level.matterTick();
  assert(p.status.effect(StatusKind::Burn).time <= 0);

  p.x = (X + 60) * 16 + 8.0, p.y = Y * 16 + 10.0;
  level.tick();
  Enemy *spore = level.spawner.spawn(EnemyKind::Sporeling, p.x + 80, p.y);
  level.tick();
  int sx = world::tileOf(spore->x), sy = world::tileOf(spore->y - 2);
  spore->strike(1e9f, 0, 1, 0);
  level.tick();
  assert(level.stuffAt(sx, sy) == Stuff::Spored && level.stuffAt(sx + 1, sy + 1) == Stuff::Spored);
  Enemy *rat = level.spawner.spawn(EnemyKind::Rat, sx * 16 + 8.0, sy * 16 + 10.0);
  level.tick();
  rat->x = sx * 16 + 8.0, rat->y = sy * 16 + 10.0;
  rat->strike(1e9f, 0, 1, 0);
  level.tick();
  assert(level.sprouts.size() == 1);
  int before = 0, after = 0;
  level.each<Enemy>([&](Enemy &e) { before += e.is(EnemyKind::Sporeling); });
  for (int i = 0; i < 60 * 7; i++)
    level.tick();
  level.each<Enemy>([&](Enemy &e) { after += e.is(EnemyKind::Sporeling); });
  assert(level.sprouts.empty() && after == before + 1);
  game.level.reset();
  noise_reseed(salt);
  rng = kept;
  std::cout << "[TEST] Matter table: a bolt over a wet pool charges all " << charged
            << " tiles of it and nothing beyond; burning oil bursts into its 8 neighbours; fire steams a wet tile "
               "dry; lime refuses fire and spores and cleanses a burning hero; a dying sporeling spreads spores and a "
               "rat slain on them rises as a sporeling 6 s later: PASS"
            << std::endl;
}

void testSynod() {
  for (EnemyKind k : {EnemyKind::Lamplighter, EnemyKind::Penitent, EnemyKind::Censer, EnemyKind::Limer,
                      EnemyKind::Confessor, EnemyKind::Prioress})
    assert(factionOf(k) == Faction::Synod && Enemy::typeOf(k).unlockAt > 1e8f);
  Grudges none;
  assert(none.between(Faction::Synod, Faction::Mycel) && none.between(Faction::Starborn, Faction::Synod) &&
         !none.between(Faction::Synod, Faction::Wildkin) && !none.between(Faction::Synod, Faction::Freeblades));
  Campaign probe;
  probe.world = "CLASSIC", probe.scrutiny = 42.5f;
  Campaign back;
  assert(Campaign::load(probe.save(), back) && std::abs(back.scrutiny - 42.5f) < 0.06f);
  assert(Campaign::load(probe.save() + "scrutiny 900\n", back) && back.scrutiny == Level::SCRUTINY_MAX);

  Rng kept = rng;
  uint32_t salt = noise_salt;
  noise_reseed(0);
  world::enter(world::Layer::Surface);
  Game game;
  RunRules rules;
  rules.synod = true, rules.matter = true, rules.feuds = true, rules.spawnRate = 0, rules.waves = false;
  rules.bosses = false;
  game.level = std::make_unique<Level>(game, rules);
  Level &level = *game.level;
  Player &p = *level.player;
  p.god = true;
  for (auto &u : p.upgrades)
    u->level = 0;
  auto synods = [&] {
    int n = 0;
    level.each<Enemy>([&](Enemy &e) { n += factionOf(e) == Faction::Synod; });
    return n;
  };

  Enemy *limer = level.spawner.spawn(EnemyKind::Limer, p.x + 200, p.y);
  Enemy *penitent = level.spawner.spawn(EnemyKind::Penitent, p.x + 210, p.y + 20);
  Enemy *confessor = level.spawner.spawn(EnemyKind::Confessor, p.x + 190, p.y + 30);
  Enemy *censer = level.spawner.spawn(EnemyKind::Censer, p.x + 30, p.y);
  level.tick();
  assert(dynamic_cast<SynodFoe *>(limer) && limer->name() == std::string("a limer"));
  float sharp = penitent->damageMul;
  penitent->hp = penitent->maxHp * 0.4f;
  float wounded = penitent->hp;
  for (int i = 0; i < 60 * 3; i++)
    level.tick();
  int limed = 0;
  for (int dy = -8; dy <= 8; dy++)
    for (int dx = -8; dx <= 8; dx++)
      limed += level.stuffAt(world::tileOf(limer->x) + dx, world::tileOf(limer->y - 2) + dy) == Stuff::Limed;
  assert(limed >= 9 && penitent->damageMul > sharp * 1.3f && penitent->hp > wounded && confessor->hp > 0);
  assert(p.status.effect(StatusKind::Weaken).time > 0);
  (void)censer;

  float before = level.scrutiny;
  limer->strike(1e9f, 0, 1, 0);
  level.tick();
  assert(std::abs(level.scrutiny - before - Level::SUSPECT_KILL) < 0.01f);
  level.each<Enemy>([](Enemy &e) { e.remove(); });
  level.tick();

  level.scrutiny = Level::PATROLS + 1, level.patrolTime = 0;
  level.feuds();
  level.tick();
  int patrol = synods();
  assert(level.scrutinySaid == 1 && patrol >= 3);
  level.each<Enemy>([](Enemy &e) { e.remove(); });
  level.tick();
  level.scrutiny = Level::INQUISITION + 1;
  level.spawner.horde(level.time);
  level.tick();
  int host = synods(), confessors = 0;
  level.each<Enemy>([&](Enemy &e) { confessors += e.is(EnemyKind::Confessor); });
  assert(host >= 4 && confessors == 1);
  game.level.reset();

  RunRules plain;
  plain.spawnRate = 0;
  game.level = std::make_unique<Level>(game, plain);
  game.level->suspect(50);
  assert(game.level->scrutiny == 0);
  game.level.reset();
  noise_reseed(salt);
  rng = kept;
  std::cout << "[TEST] Synod: six members of their own family at war with the unclean; a limer limes " << limed
            << " tiles, a wounded penitent hits harder and a confessor mends it, censer smoke weakens the hero; "
               "scrutiny saves and clamps, a slain member raises it by "
            << Level::SUSPECT_KILL << ", at 50 a patrol of " << patrol << " seeks the hero and at 75 an inquisition of "
            << host << " replaces the horde; the dawn run keeps no scrutiny: PASS" << std::endl;
}
