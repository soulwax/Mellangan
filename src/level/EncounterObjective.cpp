#include "level/EncounterObjective.hpp"

#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "level/Level.hpp"

EncounterObjective::EncounterObjective(uint32_t seed)
    : kind(int((seed >> 4) & 3)), heading((seed & 255) / 256.0 * TAU) {}

bool EncounterObjective::tick(Level &level, Player &p, double &x, double &y,
                              double d2) {
  if (kind == 1) {
    progress = d2 < 68 * 68 ? progress + DT : std::max(0.0f, progress - DT * 0.5f);
  } else if (kind == 2) {
    if (d2 < 110 * 110 && distance < 120) {
      for (int i = 0; i < 8; i++) {
        double a = heading + i * PI * 0.25;
        double nx = x + std::cos(a) * 10 * DT;
        double ny = y + std::sin(a) * 10 * DT;
        if (level.blocks(nx + std::cos(a) * 8, ny + std::sin(a) * 8))
          continue;
        distance += std::hypot(nx - x, ny - y);
        x = nx, y = ny, heading = a;
        break;
      }
    }
    int pressure = 0;
    level.enemiesNear(x, y, 40, [&](Enemy &e) {
      double dx = e.x - x, dy = e.y - y;
      if (dx * dx + dy * dy < 28 * 28)
        pressure++;
    });
    integrity -= std::min(pressure, 4) * 7 * DT;
    if (integrity <= 0)
      return false;
  } else if (kind == 3 && progress < 10 && d2 < 70 * 70) {
    progress += DT;
    p.afflict(StatusKind::Poison, 0.55f, 3);
  }
  return true;
}

bool EncounterObjective::complete() const {
  return kind == 0 || (kind == 1 && progress >= 12) ||
         (kind == 2 && distance >= 120) || (kind == 3 && progress >= 10);
}

float EncounterObjective::fraction(int defeated, int defenders) const {
  return kind == 0 ? (defenders > 0 ? float(defeated) / defenders : 0) :
         kind == 1 ? progress / 12 : kind == 2 ? float(distance / 120) : progress / 10;
}
