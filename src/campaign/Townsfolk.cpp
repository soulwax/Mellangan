#include "campaign/Townsfolk.hpp"

#include <cmath>

#include "entity/Player.hpp"
#include "level/Level.hpp"

namespace {

const char *const LINES[5][4] = {
    {"They took the mill first. Then the well.", "Keep the fire high tonight, please.",
     "I heard wolves inside the walls last night.", "My boy went for wood. He'll be back soon."},
    {"The lanterns sing in the marsh. Don't listen.", "Salt keeps the drowned out. Mostly.",
     "The abbey bells still ring, under the water.", "Spores in the bread again."},
    {"The cold is coming from the barrows, not the sky.", "Warden Kell hasn't slept in days.",
     "Something old hunts by the standing stones.", "Keep your hands near the fire."},
    {"The caravan is our only water.", "The sky splits over the steppe every night.",
     "Glass sings in the wind out there.", "Hyenas laugh just like people."},
    {"This is where the dark began.", "The ash never stops falling.",
     "Keeper Ash says the last lantern will hold.", "If the fire goes out here, it goes out everywhere."},
};

} // namespace

void Townsfolk::onAdded() {
  level->tag(this);
  tx = x, ty = y;
}

const char *Townsfolk::line() const { return LINES[town % 5][(voice + said) % 4]; }

void Townsfolk::tick() {
  t += DT;
  talk = std::max(0.0f, talk - DT), hush = std::max(0.0f, hush - DT);
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
  s.sprite(atlas::tile(tile), xp + shiver, yp - bob, 8, 16, facing, 1);
}

void Townsfolk::renderShadow(Screen &s) {
  s.sprite(atlas::Shadow, xp, yp + 1, 8, 4, 0.8f, 0.6f);
}
