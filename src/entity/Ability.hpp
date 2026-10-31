#pragma once

#include "gfx/Screen.hpp"

class Mob;

namespace ability {

void blink(Mob &caster, float reach, float damage, Color tone);
void nova(Mob &caster, float radius, float damage, float heal, Color tone);
void volley(Mob &caster, int count, double spread, float speed, float damage,
            int pierce);
void rally(Mob &caster, float reach, float damage, float guard);
void reap(Mob &caster, float reach, float damage);
void bombs(Mob &caster, int count, float damage, float heal);
void dance(Mob &caster, float reach, float damage, Color tone);
void thunder(Mob &caster, int count, float damage);

} // namespace ability
