#include "entity/Merchant.hpp"

#include "Game.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

namespace {

constexpr int TRADER = 86, BARREL = 82;
constexpr Color LAMP{0.95f, 0.7f, 0.35f, 1};

} // namespace

void Merchant::onAdded() { level->tag(this); }

void Merchant::tick() {
  t += DT;
  Player &p = *level->player;
  if (std::abs(p.x - x) > 2)
    facing = p.x < x ? -1.0f : 1.0f;
}

void Merchant::render(Screen &s) {
  float bob = 1 + 0.03f * std::sin(t * 2.2f);
  s.sprite(atlas::tile(BARREL), xp - facing * 11, yp, 8, 16, 0.8f, 0.8f);
  s.sprite(atlas::tile(TRADER), xp, yp, 8, 16, facing, bob);
  int lx = xp + int(facing * 9);
  s.rect(lx, yp - 13, 1, 13, {0.36f, 0.24f, 0.14f, 1});
  s.rect(lx - 1, yp - 16, 3, 3, LAMP * (0.85f + 0.15f * flicker(t, 7)));
}

void Merchant::renderShadow(Screen &s) {
  s.sprite(atlas::Shadow, xp, yp + 1, 8, 4, 0.9f, 0.7f);
}

void Merchant::renderLight(Screen &s) {
  float f = flicker(t, 7);
  s.glow(xp + facing * 9, yp - 14, 64 * (0.95f + 0.05f * f), LAMP * 0.35f);
  level->lights.add(x, y, 56, 0.6f * f);
}

Merchant *Merchant::camp(Level &level, double x, double y) {
  Vendor &v = level.vendor;
  if (Merchant *old = level.get<Merchant>(v.merchant)) {
    Spark::burst(level, old->x, old->y - 6, LAMP, 6, 30, true);
    old->remove();
  }
  level.collide(x, y, 5);
  Merchant *m = level.add<Merchant>(x, y);
  v.merchant = m->id;
  v.restock(level.monsterLevel(x, y), rng);
  Spark::burst(level, x, y - 6, LAMP, 10, 40, true);
  Player &p = *level.player;
  spdlog::info("merchant: camp {} at {:.0f} s, ilvl {}, hero level {} with {} gold, {} kills",
               v.restocks, level.time, v.ilvl, p.lvl, p.gold, level.kills);
  return m;
}

void Merchant::visit(Level &level) {
  Vendor &v = level.vendor;
  Night &n = level.night;
  if (const Structure *s = n.lastLit(); s && n.lit != v.camp) {
    v.camp = n.lit;
    camp(level, s->centerX() + 28, s->centerY() + 12);
    level.game.announce("A MERCHANT CAMPS BY THE FIRE");
  }
  Merchant *m = level.get<Merchant>(v.merchant);
  if (m && n.dawn && !v.day) {
    v.restock(level.monsterLevel(m->x, m->y), rng);
    level.game.announce("THE MERCHANT RESTOCKS AT DAWN");
  }
  v.day = n.dawn;
}

Merchant *Merchant::near(Level &level) {
  Merchant *m = level.get<Merchant>(level.vendor.merchant);
  Player &p = *level.player;
  if (!m || p.dead())
    return nullptr;
  double xd = m->x - p.x, yd = m->y - p.y;
  return xd * xd + yd * yd < REACH * REACH ? m : nullptr;
}
