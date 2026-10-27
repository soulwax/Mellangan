#include "particle/debris/MeatDebris.hpp"

#include "level/Level.hpp"
#include "particle/debris/BloodDebris.hpp"
#include "particle/debris/Debris.hpp"

MeatDebris::MeatDebris(double x, double y, double z, Color c)
    : Debris(x, y, z) {
  color = c;
}

void MeatDebris::tick() {
  Debris::tick();
  if (!removed)
    shed(std::make_unique<BloodDebris>(x, y, z, color), 0.05, 0.5);
}

void MeatDebris::collide(double xxa, double yya, double zza) {
  if (za < -0.5) {
    for (int i = 0; i < 20 && room(); i++) {
      auto blood = std::make_unique<BloodDebris>(x, y, 0, color);
      blood->xa = blood->xa * 0.4 + xa * 0.5;
      blood->ya = blood->ya * 0.4 + ya * 0.5;
      blood->za = -za * 0.5;
      level->spawn(std::move(blood));
    }
  }
  Debris::collide(xxa, yya, zza);
}
