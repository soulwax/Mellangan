#pragma once

#include "SpriteObject.hpp"

class Particle : public SpriteObject {
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

    inline static int alive = 0;

    Spark(double x, double y, double xa, double ya, Color color, float life, float size, bool glow)
        : Particle(x, y, life), color(color), size(size), glow(glow) {
        this->xa = xa, this->ya = ya, sprite = atlas::Dot, alive++;
    }
    ~Spark() override { alive--; }
    static void burst(Level &level, double x, double y, Color c, int n, float speed, bool glow);

    void tick() override;
    void render(Screen &s) override;
    void renderLight(Screen &s) override;
};

class TextParticle : public Particle {
public:
    int value;
    Color color;

    inline static int alive = 0;

    TextParticle(double x, double y, int value, Color color) : Particle(x, y, 0.8f), value(value), color(color) { alive++; }
    ~TextParticle() override { alive--; }
    static void pop(Level &level, double x, double y, int value, Color c);

    void tick() override;
    void render(Screen &s) override;
};
