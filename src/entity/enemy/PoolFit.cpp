#include "entity/Pool.h"
#include "entity/enemy/AlphaWolf.hpp"
#include "entity/enemy/Burrower.hpp"
#include "entity/enemy/Cyclops.hpp"
#include "entity/enemy/ExpansionEnemy.hpp"
#include "entity/enemy/Ghost.hpp"
#include "entity/enemy/Mimic.hpp"
#include "entity/enemy/Necromancer.hpp"
#include "entity/enemy/Renegade.hpp"
#include "entity/enemy/SupplyBarrel.hpp"
#include "entity/enemy/Thief.hpp"
#include "entity/enemy/WildFoe.hpp"
#include "entity/enemy/Wolf.hpp"

namespace {
template <class T, size_t Size = sizeof(T)> constexpr bool pooled() {
  static_assert(Size <= POOL_MAX, "foe outgrew the entity pool: crowds fall back to malloc");
  return true;
}
static_assert(pooled<Enemy>() && pooled<AlphaWolf>() && pooled<Burrower>() &&
              pooled<Cyclops>() && pooled<ExpansionEnemy>() && pooled<Ghost>() &&
              pooled<Mimic>() && pooled<Necromancer>() && pooled<Renegade>() &&
              pooled<SupplyBarrel>() && pooled<Thief>() && pooled<WildFoe>() &&
              pooled<Wolf>());
} // namespace
