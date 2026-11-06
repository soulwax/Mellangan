#include "campaign/Townsfolk.hpp"

#include <cmath>

#include "gui/Lang.hpp"
#include "Game.hpp"
#include "campaign/Bastion.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "particle/Spark.hpp"
#include "level/Level.hpp"

namespace {

const char *const LINES[5][4] = {
    {"They took the mill. They will not take Hearthwick.", "Keep the fire high. We hold till dawn.",
     "The knights haven't slept. Neither have we.", "Every hand on the wall tonight."},
    {"The marsh can howl all it likes. Saltmoor stands.", "Salt on the threshold, steel at the bridge.",
     "We rebuilt that wall three times. We'll do it again.", "The drowned won't have this town."},
    {"Let the barrows freeze. Frosthold doesn't break.", "Warden Kell says no one sleeps until it's done.",
     "We keep the beacons lit, whatever it costs.", "Cold hands still hold a spear."},
    {"Dunewatch has outlasted worse storms than this.", "Every drop of water, we share. Every wall, we hold.",
     "Let them laugh out there. We're still here.", "The caravan comes back. It always does."},
    {"This is where the dark began. It ends here too.", "The ash falls. The fire doesn't.",
     "Keeper Ash says the last lantern will hold. So will we.", "If this fire goes out, none do. So it won't."},
};

} // namespace

void Townsfolk::onAdded() {
  level->tag(this);
  tx = x, ty = y;
}

const char *Townsfolk::line() const {
  if (mode == Mode::Lost)
    return lang.seekerLostLine;
  if (mode == Mode::Following)
    return lang.seekerFollowLine;
  return tr(LINES[town % 5][(voice + said) % 4]);
}

void Townsfolk::settle(double cx, double cy, int tn) {
  mode = Mode::Home, home = {cx, cy}, town = tn, wander = 0, hp = HP;
}

void Townsfolk::trail() {
  Player &p = *level->player;
  level->enemiesNear(x, y, 24, [&](Enemy &e) {
    if (e.buried || e.removed || std::hypot(e.x - x, e.y - y) > e.type.radius + 6)
      return;
    if (float bite = e.type.damage * e.damageMul * DT * 1.5f; bite > 0)
      hp -= bite, flash = 0.15f;
  });
  if (hp <= 0) {
    Spark::burst(*level, x, y - 8, col::white, 12, 60, true);
    level->game.announce(lang.seekerLost);
    return remove();
  }
  double xd = p.x - x, yd = p.y - y, d = std::hypot(xd, yd);
  if (mode == Mode::Lost) {
    if (d < CALL && !p.dead()) {
      mode = Mode::Following, talk = SAY, hush = SAY + 6, spoken = line();
      level->game.announce(lang.seekerFollows);
    }
    return;
  }
  if (d > LEASH || p.dead()) {
    mode = Mode::Lost;
    return;
  }
  facing = xd < 0 ? -1.0f : 1.0f;
  tx = p.x, ty = p.y;
  if (d > 20) {
    double v = std::min(TRAIL * 1.0, p.speed * 1.05), mx = xd / d * v, my = yd / d * v;
    bastion::route(x, y, p.x, p.y, float(v), mx, my);
    double ox = x, oy = y;
    x += mx * DT, y += my * DT;
    level->collide(x, y, 4);
    bastion::wall(ox, oy, x, y);
  }
}

void Townsfolk::tick() {
  t += DT;
  talk = std::max(0.0f, talk - DT), hush = std::max(0.0f, hush - DT);
  flash = std::max(0.0f, flash - DT);
  if (mode != Mode::Home)
    return trail();
  bool night = !level->night.dawn;
  if ((wander -= DT) <= 0) {
    wander = 3 + 3 * float(std::fmod(t * 7.3 + voice, 1.0));
    double a = std::fmod(t * 2.1 + voice * 1.7, TAU), r = night ? 22 : 30 + 20 * std::sin(t + voice);
    double cx = night ? home.x + (voice - 1) * 4 : home.x, cy = night ? home.y + 2 * 16 : home.y;
    tx = cx + std::cos(a) * r, ty = cy + std::sin(a) * r * 0.6;
  }
  Player &p = *level->player;
  double pd = std::hypot(p.x - x, p.y - y);
  bool quiet = true, nearest = true;
  level->each<Townsfolk>([&](Townsfolk &o) {
    if (&o == this)
      return;
    quiet &= o.talk <= 0;
    nearest &= std::hypot(p.x - o.x, p.y - o.y) >= pd;
  });
  if (pd < HEAR && hush <= 0 && quiet && nearest && !p.dead())
    talk = SAY, hush = SAY + 6, spoken = line(), said++;
  if (talk > 0 || pd < HEAR) {
    facing = p.x < x ? -1.0f : 1.0f;
    return;
  }
  double xd = tx - x, yd = ty - y, d = std::hypot(xd, yd);
  if (d < 2)
    return;
  x += xd / d * SPEED * DT, y += yd / d * SPEED * DT;
  facing = xd < 0 ? -1.0f : 1.0f;
  level->collide(x, y, 4);
}

void Townsfolk::render(Screen &s) {
  bool night = !level->night.dawn;
  float shiver = night ? std::sin(t * 30 + voice) * 0.5f : 0;
  float bob = std::hypot(tx - x, ty - y) > 2 ? std::abs(std::sin(t * 8 + voice)) : 0;
  s.color(flash > 0 ? Color{1, 0.5f, 0.5f, 1} : col::white);
  s.sprite(atlas::tile(tile), xp + shiver, yp - bob, 8, 16, facing, 1);
  s.color(col::white);
  if (mode == Mode::Home)
    return;
  float k = std::clamp(hp / HP, 0.0f, 1.0f);
  s.rect(xp - 7, yp - 20, 14, 2, Color{0.1f, 0.05f, 0.08f, 0.8f});
  s.rect(xp - 7, yp - 20, 14 * k, 2, Color{0.5f, 0.9f, 0.5f, 0.9f});
}

void Townsfolk::renderShadow(Screen &s) {
  s.sprite(atlas::Shadow, xp, yp + 1, 8, 4, 0.8f, 0.6f);
}
