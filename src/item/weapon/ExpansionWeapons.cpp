#include "item/weapon/ExpansionWeapons.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

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
  age += DT;
  float life = kind == 2 ? 3.0f : kind == 4 ? 0.7f :
               kind == 5 ? 0.3f : kind == 6 ? 0.65f : 2.5f;
  if (age >= life)
    return remove();
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
    Spark::burst(*level, x, y, TINTS[kind], 12, reach * 2, true);
    remove();
  }
}

void ExpansionAttack::render(Screen &s) {
  Color c = TINTS[kind];
  if (kind == 2 || kind == 4) {
    s.color(c.alpha(kind == 4 ? 0.25f + age * 0.6f : 0.5f));
    s.center(atlas::Ring, xp, yp, reach * 2 / atlas::Ring.w, age);
  } else if (kind == 5 || kind == 6) {
    float width = kind == 6 ? 3.0f + 2 * (count - 1) : 2.0f;
    s.line(ox, oy, ox + dx * reach, oy + dy * reach, width, c.alpha(0.65f));
    s.line(ox, oy, ox + dx * reach, oy + dy * reach, 1, col::white.alpha(0.8f));
  } else {
    if (kind == 1)
      s.line(ox, oy, x, y, 1, c.alpha(0.4f));
    s.color(c);
    s.center(atlas::expansion(14 + kind), xp, yp, 1,
             kind == 3 ? age * 18 : float(std::atan2(dy, dx)));
  }
  s.color(col::white);
}

void ExpansionAttack::renderLight(Screen &s) {
  s.glow(xp, yp, kind == 2 || kind == 4 ? reach * 2 : 30, TINTS[kind] * 0.12f);
}
