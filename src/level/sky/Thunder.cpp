#include "level/sky/Thunder.hpp"

#include "audio/Sound.hpp"
#include "level/Level.hpp"

void Thunder::tick() {
  flash = std::max(0.0f, flash - DT * 3);
  if (flicker > 0 && (flicker -= DT) <= 0)
    flash = std::max(flash, 0.75f);
  if (in > 0 && (in -= DT) <= 0)
    Sound::thunder.play(volume, 0.04f, pitch);
}

void Thunder::storm(Level &level) {
  if (rng.chance(DT / 7)) {
    flash = std::max(flash, 0.3f);
    if (in <= 0)
      in = rng.range(1.5f, 3.5f), volume = 0.1f, pitch = rng.range(0.5f, 0.65f);
  }
  if ((strikeTime -= DT) <= 0) {
    strikeTime = rng.range(3, 8);
    float d = level.strike();
    if (d >= 0) {
      flash = 1, flicker = 0.13f;
      in = 0.1f + d / 340, volume = clampf(0.34f - d / 1100, 0.12f, 0.34f);
      pitch = clampf(0.95f - d / 1400, 0.65f, 0.95f);
    }
  }
}
