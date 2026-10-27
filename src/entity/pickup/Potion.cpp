#include "entity/pickup/Potion.hpp"

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"
#include "particle/debris/Debris.hpp"
#include "particle/debris/HealDebris.hpp"

void Potion::render(Screen &s) {
  s.center(atlas::tile(atlas::t::PotionRed), xp, yp, 0.8f);
}

void Potion::renderLight(Screen &s) {
  s.glow(xp, yp, 50, {0.35f, 0.06f, 0.08f, 1});
}

float Potion::reach() const { return level->player->magnet * 0.6f; }

void Potion::take(Player &p) {
  p.heal(30);
  Sound::potion.play(0.7f, 0);
  for (int i = 0; i < 10 && Debris::room(); i++)
    level->add<HealDebris>(p.x, p.y, 8, 0);
}
