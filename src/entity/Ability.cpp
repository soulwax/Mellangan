#include "entity/Ability.hpp"

#include "audio/Sound.hpp"
#include "entity/Mob.hpp"
#include "level/Level.hpp"
#include "Game.hpp"
#include "entity/effect/FirePool.hpp"
#include "entity/effect/GlaiveThrust.hpp"
#include "entity/effect/Smite.hpp"
#include "entity/effect/Strike.hpp"
#include "entity/effect/Sweep.hpp"
#include "entity/projectile/Slash.hpp"
#include "entity/effect/Ward.hpp"
#include "entity/projectile/Arrow.hpp"
#include "entity/projectile/Bomb.hpp"
#include "entity/projectile/MeteorRock.hpp"
#include "entity/pickup/Gem.hpp"
#include "particle/Arc.hpp"
#include "particle/debris/Chip.hpp"
#include "particle/Mote.hpp"
#include "particle/Drift.hpp"
#include "particle/Flourish.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"

namespace ability {

namespace {
template <class F> void within(Mob &caster, float radius, F &&fn) {
  caster.level->foesOf(caster, caster.x, caster.y, radius + 20, [&](Mob &foe) {
    double xd = foe.x - caster.x, yd = foe.y - caster.y;
    if (xd * xd + yd * yd < radius * radius)
      fn(foe, xd, yd);
  });
}

void lob(Mob &caster, double x, double y, float damage, float pool,
         float burn, float time) {
  float power = caster.power();
  caster.level->add<Bomb>(caster.x, caster.bodyY(), x, y, damage * power, pool,
                          burn * power, time)->hostile = caster.hostile();
}

void burst(Mob &caster, float radius, float damage, Color tone) {
  caster.level->foesOf(caster, caster.x, caster.y, radius + 20, [&](Mob &foe) {
    double xd = foe.x - caster.x, yd = foe.y - caster.y;
    if (xd * xd + yd * yd < radius * radius)
      foe.strike(damage * caster.power(), xd, yd, 140);
  });
  Spark::burst(*caster.level, caster.x, caster.bodyY(), tone, 18, 140, true);
}
} // namespace

void blink(Mob &caster, float reach, float damage, Color tone) {
  double x0 = caster.x, y0 = caster.bodyY(), ax, ay;
  caster.aim(ax, ay);
  burst(caster, 60, damage, tone);
  flourish::echo(*caster.level, caster.x, caster.y, caster.body(), tone);
  caster.x += ax * reach, caster.y += ay * reach;
  caster.level->collide(caster.x, caster.y, caster.radius);
  flourish::streak(*caster.level, x0, y0, caster.x, caster.bodyY(),
                   atlas::pt::ArcaneStar, tone, 7);
  flourish::sigil(*caster.level, caster.x, caster.bodyY(), 60, tone, 6);
  burst(caster, 60, damage, tone);
  Sound::cast.play(0.6f, 0);
}

void nova(Mob &caster, float radius, float damage, float heal, Color tone) {
  caster.mend(heal);
  caster.level->foesOf(caster, caster.x, caster.y, radius + 20, [&](Mob &foe) {
    double xd = foe.x - caster.x, yd = foe.y - caster.y;
    if (xd * xd + yd * yd < radius * radius)
      foe.strike(damage * caster.power(), xd, yd, 220);
  });
  flourish::sigil(*caster.level, caster.x, caster.bodyY(), radius, tone, 16);
  flourish::scatter(*caster.level, caster.x, caster.bodyY(), atlas::pt::HolyGlint,
                    tone, 12, 40, -50);
  Spark::burst(*caster.level, caster.x, caster.bodyY(), tone, 12, 170, true);
  Sound::nova.play(0.5f, 0);
}

void volley(Mob &caster, int count, double spread, float speed, float damage,
            int pierce) {
  double ax, ay;
  caster.aim(ax, ay);
  double aim = std::atan2(ay, ax);
  Level &level = *caster.level;
  for (int i = 0; i < count; i++) {
    double dir = aim + (i - (count - 1) / 2.0) * spread;
    Arrow *a = level.add<Arrow>(caster.x, caster.bodyY(), std::cos(dir) * speed,
                                std::sin(dir) * speed, damage * caster.power(),
                                pierce);
    a->hostile = caster.hostile();
    if (i % 2 == 0 && Drift::room()) {
      Drift *d = level.add<Drift>(caster.x, caster.bodyY(), std::cos(dir) * 110,
                                  std::sin(dir) * 110, atlas::pt::Leaf, col::white,
                                  rng.range(0.5f, 0.8f));
      d->spin = rng.range(-4, 4), d->sway = 10, d->gravity = 20;
    }
  }
  level.add<Shockwave>(caster.x, caster.bodyY(), 8, 40, 0.3f,
                       Color{0.6f, 0.95f, 0.5f, 0.7f}, false);
  Sound::bow.play(0.7f, 0);
}

void rally(Mob &caster, float reach, float damage, float guard) {
  Level &level = *caster.level;
  level.add<Sweep>(caster, reach, damage * caster.power(), SweepStyle::Rally);
  caster.shield(guard);
  level.add<Ward>(caster, guard);
  flourish::sigil(level, caster.x, caster.bodyY(), 70, col::gold, 12);
  Spark::burst(level, caster.x, caster.bodyY(), col::gold, 10, 120, true);
  Sound::scythe.play(0.6f, 0);
}

void reap(Mob &caster, float reach, float damage) {
  Level &level = *caster.level;
  level.add<Sweep>(caster, reach, damage * caster.power());
  int caught = 0;
  level.foesOf(caster, caster.x, caster.y, reach + 8, [&](Mob &foe) {
    if (caught++ < 10)
      level.add<Mote>(foe.x, foe.bodyY(), atlas::pt::Wisp,
                      Color{1, 0.55f, 0.62f, 0.95f}, rng.range(0.5f, 0.8f));
  });
  caster.mend(2.0f * caught);
  flourish::sigil(level, caster.x, caster.bodyY(), reach, Color{0.9f, 0.25f, 0.32f, 1}, 0);
  flourish::scatter(level, caster.x, caster.bodyY(), atlas::pt::Puff,
                    Color{0.22f, 0.08f, 0.14f, 0.6f}, 8, 70);
  Spark::burst(level, caster.x, caster.bodyY(), {0.9f, 0.3f, 0.35f, 1}, 10, 150, true);
  Sound::scythe.play(0.7f, 0);
}

void bombs(Mob &caster, int count, float damage, float heal) {
  Level &level = *caster.level;
  caster.mend(heal);
  flourish::scatter(level, caster.x, caster.bodyY(), atlas::pt::Bubble, col::white,
                    10, 50, -60);
  level.add<Shockwave>(caster.x, caster.bodyY(), 4, 36, 0.35f,
                       Color{0.6f, 1, 0.4f, 0.8f}, false);
  double ax, ay;
  caster.aim(ax, ay);
  double aim = std::atan2(ay, ax);
  for (int i = 0; i < count; i++) {
    double dir = caster.hostile() ? aim + (i - (count - 1) / 2.0) * 0.5
                                  : TAU * i / count + rng.range(0, 1),
           r = rng.range(50, 90);
    float power = caster.power();
    Bomb *b = level.add<Bomb>(caster.x, caster.bodyY(), caster.x + std::cos(dir) * r,
                              caster.y + std::sin(dir) * r, damage * power, 28,
                              damage * 0.35f * power, 3);
    b->hostile = caster.hostile();
  }
  Sound::axe.play(0.6f, 0);
}

void dance(Mob &caster, float reach, float damage, Color tone) {
  Level &level = *caster.level;
  auto cut = [&] {
    for (float side : {1.0f, -1.0f})
      level.add<Slash>(caster, side, damage * caster.power(), 1.2f,
                       SlashStyle::Flurry);
  };
  double x0 = caster.x, y0 = caster.bodyY(), ax, ay;
  caster.aim(ax, ay);
  cut();
  flourish::echo(level, caster.x, caster.y, caster.body(), tone);
  caster.x += ax * reach, caster.y += ay * reach;
  level.collide(caster.x, caster.y, caster.radius);
  caster.shield(0.5f);
  flourish::streak(level, x0, y0, caster.x, caster.bodyY(), atlas::pt::Petal,
                   col::white, 9);
  cut();
  Spark::burst(level, caster.x, caster.bodyY(), tone, 14, 120, true);
  Sound::slash.play(0.7f, 0);
}

void thunder(Mob &caster, int count, float damage) {
  Level &level = *caster.level;
  std::vector<Mob *> near;
  level.foesOf(caster, caster.x, caster.y, 240, [&](Mob &foe) {
    if (near.size() < 48)
      near.push_back(&foe);
  });
  for (int i = 0; i < count; i++) {
    double x = caster.x + rng.range(-120, 120), y = caster.y + rng.range(-90, 90);
    if (!near.empty()) {
      Mob &t = *near[i % near.size()];
      x = t.x + (caster.hostile() ? rng.range(-30, 30) : 0);
      y = t.y + (caster.hostile() ? rng.range(-20, 20) : 0);
    }
    level.add<Strike>(x, y, damage * caster.power(), 24)->hostile = caster.hostile();
  }
  Sound::thunder.play(0.5f);
}

void charge(Mob &caster, float reach, float damage) {
  Level &level = *caster.level;
  double x0 = caster.x, y0 = caster.y, ax, ay;
  caster.aim(ax, ay);
  flourish::echo(level, caster.x, caster.y, caster.body(), col::gold);
  caster.x += ax * reach, caster.y += ay * reach;
  level.collide(caster.x, caster.y, caster.radius);
  double vx = caster.x - x0, vy = caster.y - y0,
         len = std::max(1.0, vx * vx + vy * vy);
  level.foesOf(caster, (x0 + caster.x) / 2, (y0 + caster.y) / 2, reach / 2 + 30,
               [&](Mob &foe) {
                 double b = std::clamp(((foe.x - x0) * vx + (foe.y - y0) * vy) / len, 0.0, 1.0);
                 double xd = foe.x - x0 - vx * b, yd = foe.y - y0 - vy * b;
                 float r = 14 + foe.radius;
                 if (xd * xd + yd * yd < r * r)
                   foe.strike(damage * caster.power(), ax + xd * 0.05, ay + yd * 0.05, 260);
               });
  caster.shield(0.4f);
  flourish::streak(level, x0, y0 - 6, caster.x, caster.bodyY(), atlas::pt::Pebble,
                   Color{0.85f, 0.88f, 0.95f, 1}, 8);
  level.add<Shockwave>(caster.x, caster.bodyY(), 6, 46, 0.35f, col::gold.alpha(0.8f));
  Spark::burst(level, caster.x, caster.bodyY(), col::gold, 10, 110, true);
  Sound::scythe.play(0.6f, 0, 0.8f);
}

void miasma(Mob &caster, float radius, float poison, float seconds) {
  Level &level = *caster.level;
  constexpr Color TOXIC{0.55f, 0.95f, 0.35f, 1};
  within(caster, radius, [&](Mob &foe, double, double) {
    foe.afflict(StatusKind::Poison, seconds, poison * caster.power());
  });
  for (int i = 0; i < 14 && Drift::room(); i++) {
    float a = rng.range(0, TAU), r = rng.range(10, radius);
    Drift *d = level.add<Drift>(caster.x + std::cos(a) * r,
                                caster.y + std::sin(a) * r * 0.7f, rng.range(-6, 6),
                                -rng.range(4, 12), atlas::pt::Puff, TOXIC.alpha(0.4f),
                                rng.range(1.2f, 2.0f));
    d->grow = 1.6f, d->scale = 0.8f, d->spin = rng.range(-1, 1);
  }
  level.add<Shockwave>(caster.x, caster.bodyY(), 6, radius, 0.5f, TOXIC.alpha(0.7f), false);
  flourish::scatter(level, caster.x, caster.bodyY(), atlas::pt::Bubble, TOXIC, 10, 50, -40);
  Sound::nova.play(0.4f, 0, 0.8f);
}

void chain(Mob &caster, int jumps, float reach, float damage) {
  Level &level = *caster.level;
  constexpr Color STORM{0.55f, 0.75f, 1, 1};
  auto targets = level.nearestFoes(caster, caster.x, caster.y, reach, jumps);
  if (targets.empty())
    return;
  std::vector<ArcNode> path{{caster.x, caster.bodyY()}};
  while (!targets.empty()) {
    auto next = std::min_element(targets.begin(), targets.end(), [&](Mob *a, Mob *b) {
      auto d = [&](Mob *m) {
        double xd = m->x - path.back().x, yd = m->bodyY() - path.back().y;
        return xd * xd + yd * yd;
      };
      return d(a) < d(b);
    });
    Mob &foe = **next;
    targets.erase(next);
    path.push_back({foe.x, foe.bodyY()});
    foe.strike(damage * caster.power(), foe.x - caster.x, foe.y - caster.y, 60);
    Spark::burst(level, foe.x, foe.bodyY(), STORM, 5, 70, false);
  }
  level.add<Arc>(std::move(path), 0.3f, STORM);
  Sound::thunder.play(0.4f);
}

void grasp(Mob &caster, float radius, float damage) {
  Level &level = *caster.level;
  constexpr Color DUSK{0.62f, 0.22f, 0.45f, 1};
  int n = 0;
  within(caster, radius, [&](Mob &foe, double xd, double yd) {
    foe.strike(damage * caster.power(), -xd, -yd, 480);
    foe.afflict(StatusKind::Chill, 2, 0.45f);
    if (n++ < 8)
      flourish::streak(level, foe.x, foe.bodyY(), caster.x, caster.bodyY(),
                       atlas::pt::Wisp, DUSK, 4);
  });
  flourish::sigil(level, caster.x, caster.bodyY(), radius, DUSK, 10);
  level.add<Shockwave>(caster.x, caster.bodyY(), radius, 10, 0.45f, DUSK.alpha(0.8f), false);
  Sound::scythe.play(0.6f, 0, 0.7f);
}

void smite(Mob &caster, int count, float damage) {
  Level &level = *caster.level;
  bool h = caster.hostile();
  auto targets = level.nearestFoes(caster, caster.x, caster.y, 240, count);
  for (int i = 0; i < count; i++) {
    double x = caster.x + rng.range(-90, 90), y = caster.y + rng.range(-70, 70);
    if (!targets.empty()) {
      Mob &t = *targets[i % targets.size()];
      x = t.x + (h ? rng.range(-34, 34) : 0), y = t.y + (h ? rng.range(-24, 24) : 0);
    }
    level.add<Smite>(x, y, damage * caster.power(), 22)->hostile = h;
  }
  flourish::sigil(level, caster.x, caster.bodyY(), 50, Color{1, 0.85f, 0.4f, 1}, 8);
  Sound::nova.play(0.45f, 0, 1.2f);
}

void lunge(Mob &caster, float reach, float damage) {
  Level &level = *caster.level;
  double x0 = caster.x, y0 = caster.bodyY(), ax, ay;
  caster.aim(ax, ay);
  flourish::echo(level, caster.x, caster.y, caster.body(), col::white);
  caster.x += ax * reach * 0.4, caster.y += ay * reach * 0.4;
  level.collide(caster.x, caster.y, caster.radius);
  level.add<GlaiveThrust>(caster, ax, ay, reach, damage * caster.power(), 180, false);
  caster.shield(0.3f);
  flourish::streak(level, x0, y0, caster.x, caster.bodyY(), atlas::pt::Petal,
                   col::white, 5);
  Sound::slash.play(0.7f, 0, 0.85f);
}

void entangle(Mob &caster, float radius, float damage) {
  Level &level = *caster.level;
  constexpr Color THORN{0.45f, 0.8f, 0.35f, 1};
  int n = 0;
  within(caster, radius, [&](Mob &foe, double xd, double yd) {
    foe.strike(damage * caster.power(), xd, yd, 30);
    foe.afflict(StatusKind::Chill, 2.5f, 0.65f);
    if (n++ < 12)
      flourish::scatter(level, foe.x, foe.y, atlas::pt::Leaf, THORN, 3, 30, -20);
  });
  flourish::sigil(level, caster.x, caster.bodyY(), radius, THORN, 12);
  level.add<Shockwave>(caster.x, caster.y, 4, radius, 0.5f, THORN.alpha(0.7f), false);
  Sound::erupt.play(0.5f, 0);
}

void mines(Mob &caster, int count, float damage) {
  for (int i = 0; i < count; i++) {
    double dir = TAU * i / count + rng.range(-0.2f, 0.2f), r = rng.range(60, 95);
    lob(caster, caster.x + std::cos(dir) * r, caster.y + std::sin(dir) * r, damage,
        22, damage * 0.25f, 2.5f);
  }
  caster.level->add<Shockwave>(caster.x, caster.bodyY(), 4, 30, 0.3f,
                               Color{0.95f, 0.7f, 0.35f, 0.8f}, false);
  Sound::axe.play(0.6f, 0, 0.8f);
}

void comet(Mob &caster, int count, float damage) {
  Level &level = *caster.level;
  bool h = caster.hostile();
  auto targets = level.nearestFoes(caster, caster.x, caster.y, 260, count);
  for (int i = 0; i < count; i++) {
    double x = caster.x + rng.range(-110, 110), y = caster.y + rng.range(-80, 80);
    if (!targets.empty()) {
      Mob &t = *targets[i % targets.size()];
      x = t.x + (h ? rng.range(-40, 40) : 0), y = t.y + (h ? rng.range(-28, 28) : 0);
    }
    float power = caster.power();
    level.add<MeteorRock>(x, y, damage * power, 30, damage * 0.2f * power)->hostile = h;
  }
  flourish::scatter(level, caster.x, caster.bodyY(), atlas::pt::FallingStar,
                    Color{0.85f, 0.75f, 1, 1}, 6, 60, -40);
  Sound::cast.play(0.6f, 0, 0.8f);
}

void keg(Mob &caster, float damage) {
  double ax, ay;
  caster.aim(ax, ay);
  double x = caster.x + ax * 110, y = caster.y + ay * 110;
  auto near = caster.level->nearestFoes(caster, caster.x, caster.y, 170, 1);
  if (caster.hostile() && !near.empty())
    x = near[0]->x, y = near[0]->y;
  lob(caster, x, y, damage, 46, damage * 0.15f, 3.5f);
  Sound::axe.play(0.7f, 0, 0.7f);
}

void warcry(Mob &caster, float radius, float seconds) {
  Level &level = *caster.level;
  constexpr Color RAGE{1, 0.4f, 0.3f, 1};
  within(caster, radius, [&](Mob &foe, double xd, double yd) {
    foe.strike(6 * caster.power(), xd, yd, 260);
    foe.afflict(StatusKind::Marked, seconds, 0.3f);
  });
  level.add<Shockwave>(caster.x, caster.bodyY(), 8, radius, 0.4f, RAGE.alpha(0.85f));
  level.add<Shockwave>(caster.x, caster.bodyY(), 4, radius * 0.7f, 0.55f,
                       RAGE.alpha(0.5f), false);
  flourish::sigil(level, caster.x, caster.bodyY(), 40, RAGE, 10);
  Sound::howl.play(0.6f, 0, 1.15f);
}

void bandage(Mob &caster, float amount) {
  caster.mend(amount);
  for (StatusKind k : {StatusKind::Burn, StatusKind::Poison, StatusKind::Chill})
    caster.status.effects[int(k)] = {};
  flourish::scatter(*caster.level, caster.x, caster.bodyY(), atlas::pt::HolyGlint,
                    Color{0.5f, 1, 0.6f, 1}, 10, 40, -50);
  Spark::burst(*caster.level, caster.x, caster.bodyY(), Color{0.5f, 1, 0.6f, 1}, 8,
               60, true);
  Sound::potion.play(0.6f, 0);
}

void tumble(Mob &caster, float reach) {
  Level &level = *caster.level;
  double x0 = caster.x, y0 = caster.bodyY(), ax, ay;
  caster.aim(ax, ay);
  flourish::echo(level, caster.x, caster.y, caster.body(), Color{0.8f, 0.85f, 1, 1});
  caster.x += ax * reach, caster.y += ay * reach;
  level.collide(caster.x, caster.y, caster.radius);
  caster.shield(0.6f);
  flourish::streak(level, x0, y0, caster.x, caster.bodyY(), atlas::pt::Puff,
                   Color{0.85f, 0.8f, 0.7f, 0.6f}, 5);
  Sound::slash.play(0.4f, 0, 1.4f);
}

void whirlwind(Mob &caster, float reach, float damage) {
  Level &level = *caster.level;
  level.add<Sweep>(caster, reach, damage * caster.power());
  level.add<Shockwave>(caster.x, caster.bodyY(), reach * 0.5f, reach, 0.3f,
                       Color{0.85f, 0.9f, 1, 0.7f}, false);
  flourish::scatter(level, caster.x, caster.bodyY(), atlas::pt::Gust, col::white, 6,
                    90);
  Sound::scythe.play(0.6f, 0, 1.2f);
}

void quake(Mob &caster, float radius, float damage) {
  Level &level = *caster.level;
  within(caster, radius, [&](Mob &foe, double xd, double yd) {
    foe.strike(damage * caster.power(), xd, yd, 300);
    foe.afflict(StatusKind::Chill, 1.2f, 0.65f);
  });
  for (int i = 0; i < 10 && Debris::room(); i++) {
    float a = rng.range(0, TAU), r = rng.range(10, radius);
    level.add<Chip>(caster.x + std::cos(a) * r, caster.y + std::sin(a) * r * 0.7f, 2,
                    i % 3 ? atlas::pt::Pebble : atlas::pt::Rock, col::white, 1.2);
  }
  level.add<Shockwave>(caster.x, caster.y, 6, radius, 0.45f,
                       Color{0.75f, 0.6f, 0.4f, 0.9f});
  level.game.onExplosion();
  Sound::explosion.play(0.5f, 0, 0.7f);
}

void firering(Mob &caster, int count, float damage) {
  Level &level = *caster.level;
  for (int i = 0; i < count; i++) {
    double a = TAU * i / count;
    level.add<FirePool>(caster.x + std::cos(a) * 55, caster.y + std::sin(a) * 40, 20,
                        damage * caster.power(), 4)->hostile = caster.hostile();
  }
  Spark::burst(level, caster.x, caster.bodyY(), Color{1, 0.5f, 0.15f, 1}, 14, 110, true);
  Sound::explosion.play(0.35f, 0, 1.3f);
}

void lodestar(Mob &caster, float radius) {
  Level &level = *caster.level;
  if (caster.asPlayer())
    level.each<Gem>([&](Gem &g) {
      double xd = g.x - caster.x, yd = g.y - caster.y;
      if (!g.magnet && xd * xd + yd * yd < radius * radius)
        g.magnet = true, g.speed = -60, g.swirl = rng.range(-1, 1);
    });
  level.add<Shockwave>(caster.x, caster.bodyY(), radius * 0.4f, 6, 0.6f,
                       Color{0.5f, 0.85f, 1, 0.7f}, false);
  flourish::sigil(level, caster.x, caster.bodyY(), 36, Color{0.5f, 0.85f, 1, 1}, 6);
  Sound::magnet.play(0.6f, 0);
}

void snare(Mob &caster, float reach, float damage) {
  Level &level = *caster.level;
  double ax, ay;
  caster.aim(ax, ay);
  int n = 0;
  within(caster, reach, [&](Mob &foe, double xd, double yd) {
    double d = std::max(1.0, std::sqrt(xd * xd + yd * yd));
    if ((xd * ax + yd * ay) / d < 0.5)
      return;
    foe.strike(damage * caster.power(), xd, yd, 20);
    foe.afflict(StatusKind::Chill, 3, 0.65f);
    if (n++ < 10)
      flourish::scatter(level, foe.x, foe.bodyY(), atlas::pt::Fluff,
                        Color{0.85f, 0.78f, 0.6f, 1}, 2, 20);
  });
  for (int i = -2; i <= 2; i++) {
    double a = std::atan2(ay, ax) + i * 0.25;
    flourish::streak(level, caster.x, caster.bodyY(), caster.x + std::cos(a) * reach,
                     caster.bodyY() + std::sin(a) * reach, atlas::pt::Fluff,
                     Color{0.85f, 0.78f, 0.6f, 0.8f}, 4);
  }
  Sound::bow.play(0.6f, 0, 0.8f);
}

} // namespace ability
