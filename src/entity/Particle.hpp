#pragma once

#include "entity/Entity.hpp"

class Particle : public Entity {
public:
    float life, maxLife;

    explicit Particle(Vec2 at, float life) : life(life), maxLife(life) { pos = at; }
    int layer() const override { return 2; }
    void tick() override {
        if ((life -= DT) <= 0) remove();
    }

protected:
    float frac() const { return life / maxLife; }
};

class Spark : public Particle {
public:
    Vec2 vel;
    Color color;
    float size;
    bool glow;

    Spark(Vec2 at, Vec2 vel, Color color, float life, float size, bool glow)
        : Particle(at, life), vel(vel), color(color), size(size), glow(glow) {}
    static void burst(Level &level, Vec2 at, Color c, int n, float speed, bool glow);

    void onAdded() override;
    void tick() override;
    void render(Screen &s) override;
    void renderLight(Screen &s) override;
};

class TextParticle : public Particle {
public:
    int value;
    Color color;

    TextParticle(Vec2 at, int value, Color color) : Particle(at, 0.8f), value(value), color(color) {}
    static void pop(Level &level, Vec2 at, int value, Color c);

    void onAdded() override;
    void tick() override;
    void render(Screen &s) override;
};
