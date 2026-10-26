#pragma once

#include <entt/entity/entity.hpp>

#include "Math.hpp"
#include "gfx/Screen.hpp"

class Level;

class Entity {
public:
    Vec2 pos;
    float radius = 4;
    bool removed = false;
    Level *level = nullptr;
    entt::entity id = entt::null;

    virtual ~Entity() = default;
    virtual void onAdded() {}
    virtual void tick() {}
    virtual void render(Screen &) {}
    virtual void renderShadow(Screen &) {}
    virtual void renderLight(Screen &) {}
    // 0: on the ground, 1: y-sorted actors, 2: in the air
    virtual int layer() const { return 1; }
    void remove() { removed = true; }
};

class Mob : public Entity {
public:
    float hp = 1, maxHp = 1, flash = 0, animT = 0, facing = 1, scale = 1;
    Vec2 knock;

    Vec2 body() const { return pos + Vec2{0, -7 * scale}; }

protected:
    void drawBody(Screen &s, int tile, bool floats, float move, Color tint, float flashAmount, Color flashColor) const;
    void drawShadow(Screen &s, bool floats) const;
};
