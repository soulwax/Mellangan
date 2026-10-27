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

class Mob : public Entity {
public:
    float hp = 1, maxHp = 1, flash = 0, animT = 0, facing = 1, scale = 1;

    double bodyY() const { return y - 7 * scale; }
    float occlusion(float &width) const override { return width = 0.8f * scale, 14 * scale; }

protected:
    void drawBody(Screen &s, int px, int py, int tile, bool floats, float move, Color tint, float flashAmount, Color flashColor) const;
    void drawShadow(Screen &s, int px, int py, bool floats) const;
};
