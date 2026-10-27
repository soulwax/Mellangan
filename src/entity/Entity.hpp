#pragma once

#include <entt/entity/entity.hpp>

#include "Math.hpp"
#include "entity/Pool.h"
#include "gfx/Screen.hpp"

class Level;

class Entity {
public:
    double x = 0, y = 0, z = 0;
    double xa = 0, ya = 0, za = 0;
    float radius = 4;
    bool removed = false;
    Level *level = nullptr;
    entt::entity id = entt::null;

    virtual ~Entity() = default;
    static void *operator new(size_t size) { return pool_alloc(size); }
    static void operator delete(void *p, size_t size) { pool_free(p, size); }
    virtual void onAdded() {}
    virtual void tick() {}

    void render(Screen &s) {
        int xp = int(std::floor(x));
        int yp = int(std::floor(y - z));
        render(s, xp, yp);
    }

    virtual void render(Screen &, int, int) {}

    void renderShadow(Screen &s) {
        int xp = int(std::floor(x));
        int yp = int(std::floor(y));
        renderShadow(s, xp, yp);
    }

    virtual void renderShadow(Screen &, int, int) {}

    void renderLight(Screen &s) {
        int xp = int(std::floor(x));
        int yp = int(std::floor(y - z));
        renderLight(s, xp, yp);
    }

    virtual void renderLight(Screen &, int, int) {}

    // 0: on the ground, 1: y-sorted actors, 2: in the air
    virtual int layer() const { return 1; }
    void remove() { removed = true; }
};

class Mob : public Entity {
public:
    float hp = 1, maxHp = 1, flash = 0, animT = 0, facing = 1, scale = 1;

    double bodyY() const { return y - 7 * scale; }

protected:
    void drawBody(Screen &s, int xp, int yp, int tile, bool floats, float move, Color tint, float flashAmount, Color flashColor) const;
    void drawShadow(Screen &s, int xp, int yp, bool floats) const;
};
