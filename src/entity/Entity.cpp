#include "entity/Entity.hpp"

#include "level/Level.hpp"

void Entity::remove() {
  if (removed)
    return;
  removed = true;
  if (level && id != entt::null)
    level->retire(id);
}
