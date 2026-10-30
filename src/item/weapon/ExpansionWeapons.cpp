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

namespace {
const char *const NAMES[] = {"Ricochet Sling", "Harpoon", "Ground Sigil",
                            "Boomerang", "Mortar", "Thorn Lash", "Prism Beam",
                            "Spirit Swarm"};
const std::array<const char *, Upgrade::MAX_LEVEL> DESCRIPTIONS[] = {
    {"Stones bounce between two foes", "+8 stone damage", "+1 ricochet",
     "25% faster firing", "+12 damage, +1 ricochet"},
    {"Piercing harpoons pull and mark foes", "+12 damage", "+1 piercing hit",
     "25% faster firing", "+16 damage, stronger pull"},
    {"A ground rune pulses and chills foes", "+4 pulse damage", "+18 rune radius",
     "25% faster casting", "+6 damage, longer chill"},
    {"A curved blade returns for a second hit", "+10 damage", "+50 flight reach",
     "25% faster throwing", "+14 damage, +1 blade"},
    {"Lobs a delayed explosive at a foe", "+18 blast damage", "+18 blast radius",
     "25% faster loading", "+24 damage, +1 shell"},
    {"A thorn whip poisons foes in a line", "+10 lash damage", "+35 lash reach",
     "25% faster lashing", "+14 damage, stronger poison"},
    {"A fixed prism ray cuts through crowds", "+4 ray damage", "+60 beam reach",
     "25% faster channeling", "+6 damage, wider beam"},
    {"Two spirits seek and mark nearby foes", "+6 spirit damage", "+1 spirit",
     "25% faster summoning", "+10 damage, +1 spirit"},
};
const float DAMAGE[] = {16, 24, 8, 18, 36, 22, 7, 12};
const float DAMAGE2[] = {8, 12, 4, 10, 18, 10, 4, 6};
const float DAMAGE5[] = {12, 16, 6, 14, 24, 14, 6, 10};
const float COOLDOWN[] = {1.3f, 2.2f, 3.5f, 2.4f, 3.0f, 1.8f, 2.8f, 2.7f};
const Color TINTS[] = {{0.9f, 0.75f, 0.5f, 1}, {0.5f, 0.85f, 1, 1},
                       {0.55f, 0.65f, 1, 1}, {1, 0.8f, 0.45f, 1},
                       {1, 0.5f, 0.2f, 1}, {0.45f, 1, 0.4f, 1},
                       {0.8f, 0.5f, 1, 1}, {0.5f, 1, 0.9f, 1}};
}

ExpansionWeapon::ExpansionWeapon(int kind)
    : Upgrade(NAMES[std::clamp(kind, 0, 7)],
              atlas::expansion(14 + std::clamp(kind, 0, 7)),
              DESCRIPTIONS[std::clamp(kind, 0, 7)], true),
      kind(std::clamp(kind, 0, 7)) {}

void ExpansionWeapon::tick(Player &p) {
  if ((cd -= DT) > 0 || p.dead())
    return;
  cd = COOLDOWN[kind] * (level >= 4 ? 0.75f : 1) * p.haste * pace;
  float dmg = (DAMAGE[kind] + DAMAGE2[kind] * at(level, 2) +
               DAMAGE5[kind] * at(level, 5)) * p.might * power;
  float reach = kind == 2 || kind == 4 ? 38.0f + 18 * at(level, 3) :
                kind == 5 ? 80.0f + 35 * at(level, 3) :
                kind == 6 ? 160.0f + 60 * at(level, 3) :
                            130.0f + 50 * at(level, 3);
  auto targets = p.level->nearest(p.x, p.y, 240, 4);
  double dx = p.xAim, dy = p.yAim;
  if (!targets.empty()) {
    dx = targets.front()->x - p.x;
    dy = targets.front()->bodyY() - p.bodyY();
  }
  double len = std::hypot(dx, dy);
  if (len < 0.001)
    dx = p.facing, dy = 0, len = 1;
  dx /= len, dy /= len;
  int n = kind == 7 ? 2 + at(level, 3) + at(level, 5) :
          (kind == 3 || kind == 4) ? 1 + at(level, 5) : 1;
  for (int i = 0; i < n; i++) {
    double a = std::atan2(dy, dx) + (i - (n - 1) * 0.5) * 0.25;
    double x = p.x, y = p.bodyY();
    if (kind == 2 || kind == 4) {
      x += dx * std::min(len, 150.0);
      y += dy * std::min(len, 150.0);
      if (i > 0)
        x += std::cos(a + PI * 0.5) * 22, y += std::sin(a + PI * 0.5) * 22;
    }
    int count = kind == 0 ? 2 + at(level, 3) + at(level, 5) :
                kind == 1 ? 2 + at(level, 3) : 1 + at(level, 5);
    p.level->add<ExpansionAttack>(kind, x, y, std::cos(a), std::sin(a),
                                   dmg, reach, count)->source = this;
  }
  Sound::cast.play(0.25f);
}

ExpansionAttack::ExpansionAttack(int kind, double x, double y, double dx,
                                 double dy, float damage, float reach, int count)
    : kind(std::clamp(kind, 0, 7)), count(count), damage(damage), reach(reach),
      ox(x), oy(y), dx(dx), dy(dy) {
  this->x = x, this->y = y;
  double len = std::hypot(dx, dy);
  if (len > 0.001)
    this->dx /= len, this->dy /= len;
  else
    this->dx = 1, this->dy = 0;
  radius = kind == 1 ? 4 : 7;
}

bool ExpansionAttack::hitBefore(entt::entity id) const {
  return std::find(hits.begin(), hits.end(), id) != hits.end();
}

void ExpansionAttack::strike(Enemy &e) {
  hits.push_back(e.id);
  impact(e);
  if (kind == 1 || kind == 7)
    e.afflict(StatusKind::Marked, 2.0f, kind == 1 ? 0.2f : 0.1f, source);
  if (kind == 2)
    e.afflict(StatusKind::Chill, 0.8f + 0.4f * (count - 1), 0.25f, source);
  if (kind == 5)
    e.afflict(StatusKind::Poison, 2.0f, damage * 0.2f * count, source);
  if (kind == 4)
    e.afflict(StatusKind::Burn, 1.5f, damage * 0.12f, source);
  e.hurt(damage, dx, dy, kind == 1 ? -110.0f * count : 30.0f, source);
}

void ExpansionAttack::tick() {
  Player &p = *level->player;
  if (p.dead())
    return remove();
  if (age == 0)
    born();
  age += DT;
  float life = kind == 2 ? 3.0f : kind == 4 ? 0.7f :
               kind == 5 ? 0.3f : kind == 6 ? 0.65f : 2.5f;
  if (age >= life)
    return remove();
  trail();
  bool area = kind == 2 || kind == 4;
  bool line = kind == 5 || kind == 6;
  if (kind == 4 && age < 0.6f)
    return;
  if (kind == 3 && !returning && age >= reach / 240) {
    returning = true;
    hits.clear();
  }
  if (kind == 7 || (kind == 3 && returning)) {
    double tx = p.x, ty = p.bodyY();
    if (kind == 7) {
      auto nearby = level->nearest(x, y, 260, 24);
      for (Enemy *e : nearby)
        if (!hitBefore(e->id)) {
          tx = e->x, ty = e->bodyY();
          break;
        }
    }
    double len = std::hypot(tx - x, ty - y);
    if (kind == 3 && returning && len < 15)
      return remove();
    if (len > 0.001)
      dx = (tx - x) / len, dy = (ty - y) / len;
  }
  if (!area && !line) {
    float speed = kind == 7 ? 135.0f : kind == 1 ? 320.0f : 240.0f;
    x += dx * speed * DT;
    y += dy * speed * DT;
  }
  if (area || line) {
    if ((pulse -= DT) > 0)
      return;
    pulse = kind == 2 ? 0.6f : kind == 6 ? 0.2f : 10;
    hits.clear();
    if (kind == 2)
      level->add<Shockwave>(x, y, reach * 0.25f, reach, 0.4f,
                            Color{0.75f, 0.85f, 1, 0.7f}, false);
  }
  Enemy *bounced = nullptr;
  double cx = line ? ox + dx * reach * 0.5 : x;
  double cy = line ? oy + dy * reach * 0.5 : y;
  float scan = line ? reach * 0.5f + 40 : area ? reach + 32 : radius + 32;
  level->enemiesNear(cx, cy, scan, [&](Enemy &e) {
    if (removed || bounced || hitBefore(e.id))
      return;
    double ex = e.x - (line ? ox : x), ey = e.bodyY() - (line ? oy : y);
    float r = (area ? reach : line ? (kind == 6 ? 4.0f + 3 * (count - 1) : 12.0f) : radius) + e.radius;
    bool touch = ex * ex + ey * ey < r * r;
    if (line) {
      double along = ex * dx + ey * dy;
      double across = ex * dy - ey * dx;
      touch = along >= -e.radius && along <= reach + e.radius && std::abs(across) < r;
    }
    if (!touch)
      return;
    strike(e);
    if (kind == 0)
      bounced = &e;
    if ((kind == 0 || kind == 1 || kind == 7) && --count <= 0)
      remove();
  });
  if (kind == 0 && bounced && !removed) {
    auto nearby = level->nearest(x, y, 170, 24);
    Enemy *target = nullptr;
    for (Enemy *e : nearby)
      if (!hitBefore(e->id)) {
        target = e;
        break;
      }
    if (!target)
      return remove();
    double len = std::hypot(target->x - x, target->bodyY() - y);
    if (len > 0.001)
      dx = (target->x - x) / len, dy = (target->bodyY() - y) / len;
  }
  if (kind == 4) {
    blast();
    remove();
  }
}

void ExpansionAttack::born() {
  Color c = TINTS[kind];
  if (kind == 5) {
    for (int i = 0; i < 5 && Drift::room(); i++) {
      double k = rng.range(0.15f, 1.0f) * reach;
      Drift *d = level->add<Drift>(ox + dx * k, oy + dy * k, rng.range(-15, 15),
                                   rng.range(-25, -5), atlas::pt::Leaf,
                                   col::white, rng.range(0.5f, 0.8f));
      d->spin = rng.range(-4, 4), d->sway = 8, d->gravity = 30;
    }
  } else if (kind == 6) {
    for (int i = 0; i < 4; i++)
      level->add<Mark>(ox + dx * reach, oy + dy * reach, 0.5f,
                       atlas::pt::HolyGlint, i % 2 ? col::white : c,
                       1.0f + i * 0.3f, i * 0.4f);
  } else if (kind == 2) {
    level->add<Shockwave>(x, y, 2, reach, 0.3f, c.alpha(0.9f));
  }
}

void ExpansionAttack::trail() {
  Color c = TINTS[kind];
  if (kind == 2 && rng.chance(0.25f) && Drift::room()) {
    float a = rng.range(0, TAU), r = reach * rng.range(0.2f, 0.95f);
    Drift *d = level->add<Drift>(x + std::cos(a) * r, y + std::sin(a) * r, 0,
                                 -rng.range(4, 10), atlas::pt::Snowflake,
                                 c.alpha(0.8f), rng.range(0.5f, 0.9f));
    d->spin = 1.5f, d->scale = 0.7f;
  } else if (kind == 0 && rng.chance(0.3f) && Drift::room()) {
    Drift *d = level->add<Drift>(x, y + 2, 0, -4, atlas::pt::Puff,
                                 Color{0.8f, 0.7f, 0.55f, 0.35f}, 0.35f);
    d->scale = 0.5f, d->grow = 1.2f;
  } else if (kind == 3 && rng.chance(0.15f) && Drift::room()) {
    Drift *d = level->add<Drift>(x, y, -dy * 20, dx * 20, atlas::pt::Puff,
                                 Color{1, 0.95f, 0.85f, 0.3f}, 0.4f);
    d->scale = 0.6f, d->grow = 1.5f, d->spin = 3;
  } else if (kind == 7 && rng.chance(0.6f) && Spark::alive < 600) {
    level->add<Spark>(x, y, -dx * 20 + rng.range(-8, 8),
                      -dy * 20 - rng.range(4, 12), c, 0.35f, 0.4f, false);
  } else if (kind == 4 && age < 0.6f && rng.chance(0.6f) && Drift::room()) {
    float k = age / 0.6f;
    Drift *d = level->add<Drift>(x + 40 * (1 - k), y - 120 * (1 - k * k), 0, -6,
                                 atlas::pt::Puff, Color{0.6f, 0.55f, 0.5f, 0.45f},
                                 0.5f);
    d->scale = 0.5f, d->grow = 1;
  }
}

void ExpansionAttack::blast() {
  Color c = TINTS[kind];
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

void ExpansionAttack::impact(Enemy &e) {
  Color c = TINTS[kind];
  if (kind == 0) {
    for (int i = 0; i < 3 && Debris::room(); i++)
      level->add<Chip>(e.x, e.y, 6, atlas::pt::Pebble, col::white, 0.9);
  } else if (kind == 1) {
    for (int i = 0; i < 5 && Drift::room(); i++) {
      Drift *d = level->add<Drift>(e.x, e.bodyY(), rng.range(-40, 40),
                                   rng.range(-60, -20), atlas::pt::FrostGlint,
                                   c, rng.range(0.3f, 0.5f));
      d->gravity = 200, d->drag = 0.5f, d->scale = 0.6f;
    }
  } else if (kind == 7) {
    level->add<Mark>(e.x, e.bodyY(), 0.5f, atlas::pt::Wisp, c, 1, 0);
  }
}

void ExpansionAttack::render(Screen &s) {
  Color c = TINTS[kind];
  double ex = ox + dx * reach, ey = oy + dy * reach;
  if (kind == 2) {
    float fade = std::min({1.0f, age * 4, (3 - age) * 2});
    s.ring(xp, yp, reach, c.alpha(0.55f * fade));
    s.ring(xp, yp, reach * 0.62f, c.alpha(0.3f * fade));
    for (int i = 0; i < 6; i++) {
      float a = age * 0.8f + TAU * i / 6;
      s.particle(atlas::pt::Rune, xp + std::cos(a) * reach * 0.81f,
                 yp + std::sin(a) * reach * 0.81f, c.alpha(0.75f * fade), 1,
                 a + PI * 0.5f);
    }
    s.particle(atlas::pt::Rune, xp, yp, col::white.alpha(0.8f * fade), 2,
               -age * 0.5f);
  } else if (kind == 4) {
    float k = std::min(1.0f, age / 0.6f);
    s.ring(xp, yp, reach * (1.5f - 0.5f * k), col::gold.alpha(0.3f + 0.5f * k));
    if (k < 1) {
      s.sprite(atlas::Shadow, xp, yp + 1, 8, 4, 0.5f + k, 0.5f + 0.6f * k);
      s.particle(atlas::pt::Rock, xp + 40 * (1 - k), yp - 120 * (1 - k * k),
                 col::white, 1.5f, age * 10);
    }
  } else if (kind == 5) {
    float a = 1 - age / 0.3f;
    s.line(ox, oy, ex, ey, 3, Color{0.2f, 0.45f, 0.15f, 0.8f * a});
    s.line(ox, oy, ex, ey, 1, c.alpha(a));
    for (double k = 10; k < reach; k += 7) {
      double side = std::fmod(k, 14) < 7 ? 1 : -1;
      double tx = ox + dx * k, ty = oy + dy * k;
      s.line(tx, ty, tx - dy * 4 * side + dx * 2, ty + dx * 4 * side + dy * 2,
             1, Color{0.85f, 1, 0.6f, a});
    }
  } else if (kind == 6) {
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
  } else if (kind == 7) {
    s.color(c.alpha(0.4f));
    s.center(atlas::Glow, xp, yp, 16 / float(atlas::Glow.w));
    s.color(col::white);
    s.particle(atlas::pt::Wisp, xp, yp, Color{0.75f, 1, 0.95f, 1}, 1,
               float(std::atan2(-dx, dy)) + 0.2f * std::sin(age * 14));
  } else {
    if (kind == 1) {
      double rx = x - ox, ry = y - oy;
      for (int i = 0; i < 8; i += 2)
        s.line(ox + rx * i / 8, oy + ry * i / 8 + std::sin(age * 9 + i) * 1.5,
               ox + rx * (i + 1) / 8, oy + ry * (i + 1) / 8, 1,
               Color{0.8f, 0.7f, 0.5f, 0.7f});
    }
    if (kind == 3)
      for (int i = 2; i >= 1; i--) {
        s.color(c.alpha(0.2f * float(3 - i)));
        s.center(atlas::expansion(14 + kind), xp - dx * 6 * i, yp - dy * 6 * i,
                 1, (age - 0.04f * i) * 18);
      }
    s.color(kind == 0 ? col::white : c);
    s.center(atlas::expansion(14 + kind), xp, yp, 1,
             kind == 3 ? age * 18 : kind == 0 ? age * 12 : float(std::atan2(dy, dx)));
  }
  s.color(col::white);
}

void ExpansionAttack::renderLight(Screen &s) {
  s.glow(xp, yp, kind == 2 || kind == 4 ? reach * 2 : 30, TINTS[kind] * 0.12f);
}
