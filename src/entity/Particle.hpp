#pragma once

#include "entity/Entity.hpp"

class Particle : public Entity {
public:
    float life, maxLife;

    Particle(double x, double y, float life) : life(life), maxLife(life) { this->x = x, this->y = y; }
    int layer() const override { return 2; }
    void tick() override {
        if ((life -= DT) <= 0) remove();
    }

protected:
    float frac() const { return life / maxLife; }
};

class Spark : public Particle {
public:
    Color color;
    float size;
    bool glow;

    Spark(double x, double y, double xa, double ya, Color color, float life, float size, bool glow)
        : Particle(x, y, life), color(color), size(size), glow(glow) {
        this->xa = xa, this->ya = ya;
    }
    static void burst(Level &level, double x, double y, Color c, int n, float speed, bool glow);

    void onAdded() override;
    void tick() override;
    void render(Screen &s, int xp, int yp) override;
    void renderLight(Screen &s, int xp, int yp) override;
};

class TextParticle : public Particle {
public:
    int value;
    Color color;

    TextParticle(double x, double y, int value, Color color) : Particle(x, y, 0.8f), value(value), color(color) {}
    static void pop(Level &level, double x, double y, int value, Color c);

    void onAdded() override;
    void tick() override;
    void render(Screen &s, int xp, int yp) override;
};
