#include "entity/effect/Aura.hpp"

#include <algorithm>

#include "audio/Sound.hpp"
#include "entity/Mob.hpp"
#include "level/Level.hpp"
#include "particle/Arc.hpp"
#include "particle/Drift.hpp"
#include "particle/Spark.hpp"

Aura::Aura(const Mob &owner, const AuraDef &def)
    : owner(owner.id), hostile(owner.hostile()), d(def), zap(def.zap) {
  x = owner.x, y = owner.y, radius = def.radius;
}

float Aura::fade() const { return std::min({1.0f, age * 4, (d.life - age) * 2}); }

void Aura::strikeOne() {
  std::vector<Mob *> near = level->nearestFoes(hostile, x, y, d.radius, 6);
  if (near.empty())
    return;
  Mob &e = *near[rng.irange(0, int(near.size()) - 1)];
  Mob *o = level->mob(owner);
  e.strike(d.bolt * (o ? o->arcs : 1), e.x - x, e.y - y, 50);
  e.afflict(StatusKind::Chill, 0.4f, 0.4f);
  level->add<Arc>(std::vector<ArcNode>{{e.x + rng.range(-10, 10), e.bodyY() - 70},
                                       {e.x, e.bodyY()}},
                  0.22f, d.tone);
  if (Spark::alive < 600)
    Spark::burst(*level, e.x, e.bodyY(), d.tone, 4, 60, true);
  if (zaps++ % 3 == 0)
    Sound::thunder.play(0.15f);
}

void Aura::tick() {
  Mob *o = level->mob(owner);
  if ((age += DT) >= d.life || (d.follow && (!o || o->dead())))
    return remove();
  if (d.follow && o)
    x = o->x, y = o->y;
  if (d.zap > 0 && (zap -= DT) <= 0)
    zap = d.zap, strikeOne();
  if ((pulse -= DT) > 0)
    return;
  pulse = 0.5f;
  level->foes(hostile, x, y, d.radius + 16, [&](Mob &e) {
    double ex = e.x - x, ey = e.y - y;
    float rr = d.radius + e.radius;
    if (ex * ex + ey * ey >= rr * rr)
      return;
    if (d.damage > 0)
      e.strike(d.damage * 0.5f, ex, ey, 0);
    if (d.chill > 0)
      e.afflict(StatusKind::Chill, 0.7f, d.chill);
    if (d.poison > 0)
      e.afflict(StatusKind::Poison, 1.5f, d.poison);
    if (d.burn > 0)
      e.afflict(StatusKind::Burn, 1.5f, d.burn);
    if (d.mark > 0)
      e.afflict(StatusKind::Marked, 1, d.mark);
  });
  if (d.heal > 0 && o && !o->dead()) {
    double ex = o->x - x, ey = o->y - y;
    if (ex * ex + ey * ey < d.radius * d.radius) {
      o->hp = std::min(o->maxHp, o->hp + d.heal * 0.5f);
      if (Spark::alive < 500)
        Spark::burst(*level, o->x, o->bodyY(), d.tone, 2, 25, false);
    }
  }
  if (Drift::room()) {
    float a = rng.range(0, TAU), r = rng.range(0.2f, 1) * d.radius;
    Drift *p = level->add<Drift>(x + std::cos(a) * r, y + std::sin(a) * r * 0.7f, 0,
                                 -rng.range(6, 14), d.particle, d.tone.alpha(0.7f),
                                 rng.range(0.8f, 1.2f));
    p->scale = 0.7f, p->spin = rng.range(-2, 2);
  }
}

void Aura::render(Screen &s) {
  float f = fade(), beat = 0.85f + 0.15f * std::sin(age * 5);
  s.ring(x, y, d.radius, d.tone.alpha(0.55f * f), 1);
  s.ring(x, y, d.radius * 0.7f * beat, d.tone.alpha(0.25f * f));
  for (int i = 0; i < 8; i++) {
    float t = age * 0.8f + TAU * i / 8;
    s.particle(d.particle, x + std::cos(t) * d.radius,
               y + std::sin(t) * d.radius * 0.7f, d.tone.alpha(0.7f * f), 0.7f, t);
  }
  s.color(d.tone.alpha(0.12f * f));
  s.center(atlas::Glow, x, y, d.radius * 2.2f / atlas::Glow.w);
  s.color(col::white);
}

void Aura::renderLight(Screen &s) { s.glow(x, y, d.radius * 1.6f, d.tone * (0.1f * fade())); }
