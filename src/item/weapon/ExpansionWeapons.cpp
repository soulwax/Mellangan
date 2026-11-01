#include "item/weapon/ExpansionWeapons.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "level/Level.hpp"
#include "particle/Drift.hpp"
#include "particle/Mark.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"
#include "particle/debris/Chip.hpp"

using namespace xw;

namespace {

enum Shape { Bolt, Area, Line };

struct Def {
  const char *name;
  std::array<const char *, Upgrade::MAX_LEVEL> desc;
  float damage, damage2, damage5, cooldown;
  Color tint;
  Shape shape;
  float reachBase, reachStep, life, speed, pulse;
};

const Def DEFS[Count] = {
    {"Ricochet Sling",
     {"Stones bounce between two foes", "+8 stone damage", "+1 ricochet",
      "25% faster firing", "+12 damage, +1 ricochet"},
     16, 8, 12, 1.3f, {0.9f, 0.75f, 0.5f, 1}, Bolt, 130, 50, 2.5f, 240, 10},
    {"Harpoon",
     {"Piercing harpoons pull and mark foes", "+12 damage", "+1 piercing hit",
      "25% faster firing", "+16 damage, stronger pull"},
     24, 12, 16, 2.2f, {0.5f, 0.85f, 1, 1}, Bolt, 130, 50, 2.5f, 320, 10},
    {"Ground Sigil",
     {"A ground rune pulses and chills foes", "+4 pulse damage", "+18 rune radius",
      "25% faster casting", "+6 damage, longer chill"},
     8, 4, 6, 3.5f, {0.55f, 0.65f, 1, 1}, Area, 38, 18, 3.0f, 0, 0.6f},
    {"Boomerang",
     {"A curved blade returns for a second hit", "+10 damage", "+50 flight reach",
      "25% faster throwing", "+14 damage, +1 blade"},
     18, 10, 14, 2.4f, {1, 0.8f, 0.45f, 1}, Bolt, 130, 50, 2.5f, 240, 10},
    {"Mortar",
     {"Lobs a delayed explosive at a foe", "+18 blast damage", "+18 blast radius",
      "25% faster loading", "+24 damage, +1 shell"},
     36, 18, 24, 3.0f, {1, 0.5f, 0.2f, 1}, Area, 38, 18, 0.7f, 0, 10},
    {"Thorn Lash",
     {"A thorn whip poisons foes in a line", "+10 lash damage", "+35 lash reach",
      "25% faster lashing", "+14 damage, stronger poison"},
     22, 10, 14, 1.8f, {0.45f, 1, 0.4f, 1}, Line, 80, 35, 0.3f, 0, 10},
    {"Prism Beam",
     {"A fixed prism ray cuts through crowds", "+4 ray damage", "+60 beam reach",
      "25% faster channeling", "+6 damage, wider beam"},
     7, 4, 6, 2.8f, {0.8f, 0.5f, 1, 1}, Line, 160, 60, 0.65f, 0, 0.2f},
    {"Spirit Swarm",
     {"Two spirits seek and mark nearby foes", "+6 spirit damage", "+1 spirit",
      "25% faster summoning", "+10 damage, +1 spirit"},
     12, 6, 10, 2.7f, {0.5f, 1, 0.9f, 1}, Bolt, 130, 50, 2.5f, 135, 10},
};

int clampKind(int kind) { return std::clamp(kind, 0, Count - 1); }

} // namespace

ExpansionWeapon::ExpansionWeapon(int kind)
    : Upgrade(DEFS[clampKind(kind)].name, atlas::expansion(14 + clampKind(kind)),
              DEFS[clampKind(kind)].desc, true),
      kind(clampKind(kind)) {}

void ExpansionWeapon::tick(Mob &p) {
  if ((cd -= DT) > 0 || p.dead())
    return;
  const Def &d = DEFS[kind];
  cd = d.cooldown * (level >= 4 ? 0.75f : 1) * p.haste * pace;
  float dmg = (d.damage + d.damage2 * at(level, 2) + d.damage5 * at(level, 5)) *
              p.might * power;
  float reach = d.reachBase + d.reachStep * at(level, 3);
  auto targets = p.level->nearestFoes(p, p.x, p.y, 240, 4);
  double dx = p.xAim, dy = p.yAim;
  if (!targets.empty()) {
    dx = targets.front()->x - p.x;
    dy = targets.front()->bodyY() - p.bodyY();
  }
  double len = std::hypot(dx, dy);
  if (len < 0.001)
    dx = p.facing, dy = 0, len = 1;
  dx /= len, dy /= len;
  int n = kind == Swarm                          ? 2 + at(level, 3) + at(level, 5)
          : kind == Boomerang || kind == Mortar ? 1 + at(level, 5)
                                                : 1;
  int count = kind == Sling     ? 2 + at(level, 3) + at(level, 5)
              : kind == Harpoon ? 2 + at(level, 3)
                                : 1 + at(level, 5);
  for (int i = 0; i < n; i++) {
    double a = std::atan2(dy, dx) + (i - (n - 1) * 0.5) * 0.25;
    double x = p.x, y = p.bodyY();
    if (d.shape == Area) {
      x += dx * std::min(len, 150.0);
      y += dy * std::min(len, 150.0);
      if (i > 0)
        x += std::cos(a + PI * 0.5) * 22, y += std::sin(a + PI * 0.5) * 22;
    }
    p.level->add<ExpansionAttack>(kind, x, y, std::cos(a), std::sin(a), dmg,
                                  reach, count)->source = this;
  }
  Sound::cast.play(0.25f);
}

ExpansionAttack::ExpansionAttack(int kind, double x, double y, double dx,
                                 double dy, float damage, float reach, int count)
    : kind(clampKind(kind)), count(count), damage(damage), reach(reach),
      ox(x), oy(y), dx(dx), dy(dy) {
  this->x = x, this->y = y;
  double len = std::hypot(dx, dy);
  if (len > 0.001)
    this->dx /= len, this->dy /= len;
  else
    this->dx = 1, this->dy = 0;
  radius = this->kind == Harpoon ? 4 : 7;
}

bool ExpansionAttack::hitBefore(entt::entity id) const {
  return std::find(hits.begin(), hits.end(), id) != hits.end();
}

Mob *ExpansionAttack::freshFoe(double range) const {
  for (Mob *e : level->nearestFoes(source && source->hostile(), x, y, range, 24))
    if (!hitBefore(e->id))
      return e;
  return nullptr;
}

void ExpansionAttack::steer(double tx, double ty) {
  double len = std::hypot(tx - x, ty - y);
  if (len > 0.001)
    dx = (tx - x) / len, dy = (ty - y) / len;
}

void ExpansionAttack::strike(Mob &e) {
  hits.push_back(e.id);
  impact(e);
  if (kind == Harpoon || kind == Swarm)
    e.afflict(StatusKind::Marked, 2.0f, kind == Harpoon ? 0.2f : 0.1f, source);
  if (kind == Sigil)
    e.afflict(StatusKind::Chill, 0.8f + 0.4f * (count - 1), 0.25f, source);
  if (kind == Lash)
    e.afflict(StatusKind::Poison, 2.0f, damage * 0.2f * count, source);
  if (kind == Mortar)
    e.afflict(StatusKind::Burn, 1.5f, damage * 0.12f, source);
  e.strike(damage, dx, dy, kind == Harpoon ? -110.0f * count : 30.0f, source);
}

void ExpansionAttack::sweep(bool area, bool line, Mob *&bounced) {
  double cx = line ? ox + dx * reach * 0.5 : x;
  double cy = line ? oy + dy * reach * 0.5 : y;
  float scan = line ? reach * 0.5f + 40 : area ? reach + 32 : radius + 32;
  float width = kind == Prism ? 4.0f + 3 * (count - 1) : 12.0f;
  level->foes(source && source->hostile(), cx, cy, scan, [&](Mob &e) {
    if (removed || bounced || hitBefore(e.id))
      return;
    double ex = e.x - (line ? ox : x), ey = e.bodyY() - (line ? oy : y);
    float r = (area ? reach : line ? width : radius) + e.radius;
    bool touch = ex * ex + ey * ey < r * r;
    if (line) {
      double along = ex * dx + ey * dy;
      double across = ex * dy - ey * dx;
      touch = along >= -e.radius && along <= reach + e.radius && std::abs(across) < r;
    }
    if (!touch)
      return;
    strike(e);
    if (kind == Sling)
      bounced = &e;
    if ((kind == Sling || kind == Harpoon || kind == Swarm) && --count <= 0)
      remove();
  });
}

void ExpansionAttack::tick() {
  Mob &p = source && source->holder ? *source->holder : *level->player;
  if (p.dead())
    return remove();
  if (age == 0)
    born();
  age += DT;
  const Def &d = DEFS[kind];
  if (age >= d.life)
    return remove();
  trail();
  bool area = d.shape == Area, line = d.shape == Line;
  if (kind == Mortar && age < 0.6f)
    return;
  if (kind == Boomerang && !returning && age >= reach / 240) {
    returning = true;
    hits.clear();
  }
  if (kind == Boomerang && returning) {
    if (std::hypot(p.x - x, p.bodyY() - y) < 15)
      return remove();
    steer(p.x, p.bodyY());
  } else if (kind == Swarm) {
    Mob *e = freshFoe(260);
    steer(e ? e->x : p.x, e ? e->bodyY() : p.bodyY());
  }
  if (d.shape == Bolt) {
    x += dx * d.speed * DT;
    y += dy * d.speed * DT;
  } else {
    if ((pulse -= DT) > 0)
      return;
    pulse = d.pulse;
    hits.clear();
    if (kind == Sigil)
      level->add<Shockwave>(x, y, reach * 0.25f, reach, 0.4f,
                            Color{0.75f, 0.85f, 1, 0.7f}, false);
  }
  Mob *bounced = nullptr;
  sweep(area, line, bounced);
  if (kind == Sling && bounced && !removed) {
    Mob *target = freshFoe(170);
    if (!target)
      return remove();
    steer(target->x, target->bodyY());
  }
  if (kind == Mortar) {
    blast();
    remove();
  }
}

void ExpansionAttack::born() {
  Color c = DEFS[kind].tint;
  if (kind == Lash) {
    for (int i = 0; i < 5 && Drift::room(); i++) {
      double k = rng.range(0.15f, 1.0f) * reach;
      Drift *d = level->add<Drift>(ox + dx * k, oy + dy * k, rng.range(-15, 15),
                                   rng.range(-25, -5), atlas::pt::Leaf,
                                   col::white, rng.range(0.5f, 0.8f));
      d->spin = rng.range(-4, 4), d->sway = 8, d->gravity = 30;
    }
  } else if (kind == Prism) {
    for (int i = 0; i < 4; i++)
      level->add<Mark>(ox + dx * reach, oy + dy * reach, 0.5f,
                       atlas::pt::HolyGlint, i % 2 ? col::white : c,
                       1.0f + i * 0.3f, i * 0.4f);
  } else if (kind == Sigil) {
    level->add<Shockwave>(x, y, 2, reach, 0.3f, c.alpha(0.9f));
  }
}

void ExpansionAttack::trail() {
  Color c = DEFS[kind].tint;
  if (kind == Sigil && rng.chance(0.25f) && Drift::room()) {
    float a = rng.range(0, TAU), r = reach * rng.range(0.2f, 0.95f);
    Drift *d = level->add<Drift>(x + std::cos(a) * r, y + std::sin(a) * r, 0,
                                 -rng.range(4, 10), atlas::pt::Snowflake,
                                 c.alpha(0.8f), rng.range(0.5f, 0.9f));
    d->spin = 1.5f, d->scale = 0.7f;
  } else if (kind == Sling && rng.chance(0.3f) && Drift::room()) {
    Drift *d = level->add<Drift>(x, y + 2, 0, -4, atlas::pt::Puff,
                                 Color{0.8f, 0.7f, 0.55f, 0.35f}, 0.35f);
    d->scale = 0.5f, d->grow = 1.2f;
  } else if (kind == Boomerang && rng.chance(0.15f) && Drift::room()) {
    Drift *d = level->add<Drift>(x, y, -dy * 20, dx * 20, atlas::pt::Puff,
                                 Color{1, 0.95f, 0.85f, 0.3f}, 0.4f);
    d->scale = 0.6f, d->grow = 1.5f, d->spin = 3;
  } else if (kind == Swarm && rng.chance(0.6f) && Spark::alive < 600) {
    level->add<Spark>(x, y, -dx * 20 + rng.range(-8, 8),
                      -dy * 20 - rng.range(4, 12), c, 0.35f, 0.4f, false);
  } else if (kind == Mortar && age < 0.6f && rng.chance(0.6f) && Drift::room()) {
    float k = age / 0.6f;
    Drift *d = level->add<Drift>(x + 40 * (1 - k), y - 120 * (1 - k * k), 0, -6,
                                 atlas::pt::Puff, Color{0.6f, 0.55f, 0.5f, 0.45f},
                                 0.5f);
    d->scale = 0.5f, d->grow = 1;
  }
}

void ExpansionAttack::blast() {
  Color c = DEFS[kind].tint;
  level->add<Shockwave>(x, y, 4, reach * 1.3f, 0.35f, c);
  level->add<Mark>(x, y, 2.5f, atlas::Glow, Color{0.06f, 0.04f, 0.03f, 0.45f},
                   reach * 2.2f / atlas::Glow.w);
  for (int i = 0; i < 6 && Debris::room(); i++)
    level->add<Chip>(x, y, 2, i % 2 ? atlas::pt::Rock : atlas::pt::Pebble,
                     col::white, 1.4);
  for (int i = 0; i < 4 && Drift::room(); i++) {
    float a = rng.range(0, TAU);
    Drift *d = level->add<Drift>(x, y, std::cos(a) * 30, std::sin(a) * 20 - 10,
                                 atlas::pt::Puff, Color{0.5f, 0.45f, 0.42f, 0.5f},
                                 rng.range(0.6f, 0.9f));
    d->grow = 1.6f;
  }
  Spark::burst(*level, x, y, c, 8, reach * 2, true);
}

void ExpansionAttack::impact(Mob &e) {
  Color c = DEFS[kind].tint;
  if (kind == Sling) {
    for (int i = 0; i < 3 && Debris::room(); i++)
      level->add<Chip>(e.x, e.y, 6, atlas::pt::Pebble, col::white, 0.9);
  } else if (kind == Harpoon) {
    for (int i = 0; i < 5 && Drift::room(); i++) {
      Drift *d = level->add<Drift>(e.x, e.bodyY(), rng.range(-40, 40),
                                   rng.range(-60, -20), atlas::pt::FrostGlint,
                                   c, rng.range(0.3f, 0.5f));
      d->gravity = 200, d->drag = 0.5f, d->scale = 0.6f;
    }
  } else if (kind == Swarm) {
    level->add<Mark>(e.x, e.bodyY(), 0.5f, atlas::pt::Wisp, c, 1, 0);
  }
}

void ExpansionAttack::renderBolt(Screen &s, Color c) const {
  Sprite art = atlas::expansion(14 + kind);
  if (kind == Harpoon) {
    double rx = x - ox, ry = y - oy;
    for (int i = 0; i < 8; i += 2)
      s.line(ox + rx * i / 8, oy + ry * i / 8 + std::sin(age * 9 + i) * 1.5,
             ox + rx * (i + 1) / 8, oy + ry * (i + 1) / 8, 1,
             Color{0.8f, 0.7f, 0.5f, 0.7f});
  }
  if (kind == Boomerang)
    for (int i = 2; i >= 1; i--) {
      s.color(c.alpha(0.2f * float(3 - i)));
      s.center(art, xp - dx * 6 * i, yp - dy * 6 * i, 1, (age - 0.04f * i) * 18);
    }
  s.color(kind == Sling ? col::white : c);
  s.center(art, xp, yp, 1,
           kind == Boomerang ? age * 18 : kind == Sling ? age * 12 : float(std::atan2(dy, dx)));
}

void ExpansionAttack::render(Screen &s) {
  Color c = DEFS[kind].tint;
  double ex = ox + dx * reach, ey = oy + dy * reach;
  switch (kind) {
  case Sigil: {
    float fade = std::min({1.0f, age * 4, (3 - age) * 2});
    s.ring(xp, yp, reach, c.alpha(0.55f * fade));
    s.ring(xp, yp, reach * 0.62f, c.alpha(0.3f * fade));
    for (int i = 0; i < 6; i++) {
      float a = age * 0.8f + TAU * i / 6;
      s.particle(atlas::pt::Rune, xp + std::cos(a) * reach * 0.81f,
                 yp + std::sin(a) * reach * 0.81f, c.alpha(0.75f * fade), 1,
                 a + PI * 0.5f);
    }
    s.particle(atlas::pt::Rune, xp, yp, col::white.alpha(0.8f * fade), 2, -age * 0.5f);
    break;
  }
  case Mortar: {
    float k = std::min(1.0f, age / 0.6f);
    s.ring(xp, yp, reach * (1.5f - 0.5f * k), col::gold.alpha(0.3f + 0.5f * k));
    if (k < 1) {
      s.sprite(atlas::Shadow, xp, yp + 1, 8, 4, 0.5f + k, 0.5f + 0.6f * k);
      s.particle(atlas::pt::Rock, xp + 40 * (1 - k), yp - 120 * (1 - k * k),
                 col::white, 1.5f, age * 10);
    }
    break;
  }
  case Lash: {
    float a = 1 - age / 0.3f;
    s.line(ox, oy, ex, ey, 3, Color{0.2f, 0.45f, 0.15f, 0.8f * a});
    s.line(ox, oy, ex, ey, 1, c.alpha(a));
    for (double k = 10; k < reach; k += 7) {
      double side = std::fmod(k, 14) < 7 ? 1 : -1;
      double tx = ox + dx * k, ty = oy + dy * k;
      s.line(tx, ty, tx - dy * 4 * side + dx * 2, ty + dx * 4 * side + dy * 2,
             1, Color{0.85f, 1, 0.6f, a});
    }
    break;
  }
  case Prism: {
    float a = std::min(1.0f, (0.65f - age) * 4), w = 2.0f + (count - 1);
    double sx = -dy, sy = dx, shift = std::sin(age * 30) * 1.5;
    const Color hues[] = {{1, 0.4f, 0.55f, 0.5f}, {0.4f, 1, 0.6f, 0.5f},
                          {0.45f, 0.6f, 1, 0.5f}};
    for (int i = 0; i < 3; i++) {
      double o = (i - 1) * (w + 1) + shift * (i - 1);
      s.line(ox + sx * o, oy + sy * o, ex + sx * o, ey + sy * o, w,
             hues[i].alpha(0.5f * a));
    }
    s.line(ox, oy, ex, ey, std::max(1.0f, w - 1), col::white.alpha(0.9f * a));
    break;
  }
  case Swarm:
    s.color(c.alpha(0.4f));
    s.center(atlas::Glow, xp, yp, 16 / float(atlas::Glow.w));
    s.color(col::white);
    s.particle(atlas::pt::Wisp, xp, yp, Color{0.75f, 1, 0.95f, 1}, 1,
               float(std::atan2(-dx, dy)) + 0.2f * std::sin(age * 14));
    break;
  default:
    renderBolt(s, c);
  }
  s.color(col::white);
}

void ExpansionAttack::renderLight(Screen &s) {
  s.glow(xp, yp, DEFS[kind].shape == Area ? reach * 2 : 30, DEFS[kind].tint * 0.12f);
}
