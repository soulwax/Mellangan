#pragma once

#include <entt/entity/entity.hpp>

#include "SpriteObject.hpp"

class Upgrade;

class Entity : public SpriteObject {
public:
  float radius = 4;
  Upgrade *source = nullptr;
  entt::entity id = entt::null;

  virtual void onAdded() {}
  void remove();
};
