#pragma once

#include <entt/entity/entity.hpp>

#include "SpriteObject.hpp"

class Entity : public SpriteObject {
public:
    float radius = 4;
    entt::entity id = entt::null;

    virtual void onAdded() {}
    void remove();
};
