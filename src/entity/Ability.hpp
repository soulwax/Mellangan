#pragma once

#include "gfx/Screen.hpp"

class Mob;

namespace ability {

void blink(Mob &caster, float reach, float damage, Color tone);
void nova(Mob &caster, float radius, float damage, float heal, Color tone);

} // namespace ability
