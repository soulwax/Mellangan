#pragma once

#include <memory>

#include "SpriteObject.hpp"

class Debris : public SpriteObject {
public:
    static constexpr int MAX = 1200;
    inline static int alive = 0;

    int life, icon = 0;
    double drag = 0.998, bounce = 0.6, gravity = 0.08;
    Color color = col::white;

    Debris(double x, double y, double z);
    ~Debris() override { alive--; }
    static bool room();
    static void gore(Level &level, double x, double y, double z, Color blood, bool meat);

    void tick() override;
    void render(Screen &s) override;
    void renderShadow(Screen &s) override;

protected:
    virtual void collide(double xxa, double yya, double zza);
    void shed(std::unique_ptr<Debris> d, double keep, double carry);

private:
    void attemptMove();
    void move(double xxa, double yya, double zza);
};

class BloodDebris : public Debris {
public:
    BloodDebris(double x, double y, double z, Color c);
    void render(Screen &s) override;
    void renderShadow(Screen &s) override;
};

class SmokeDebris : public Debris {
public:
    int lifeTime;

    SmokeDebris(double x, double y, double z);
    void render(Screen &s) override;
    void renderShadow(Screen &) override {}
};

class FlameDebris : public Debris {
public:
    int maxLife;
    bool noSmoke = false, lit;

    FlameDebris(double x, double y, double z);
    static void lick(Level &level, double x, double y, double z, float heat);
    void tick() override;
    void render(Screen &s) override;
    void renderShadow(Screen &) override {}
    void renderLight(Screen &s) override;
};

class MeatDebris : public Debris {
public:
    MeatDebris(double x, double y, double z, Color c);
    void tick() override;

protected:
    void collide(double xxa, double yya, double zza) override;
};

class HealDebris : public Debris {
public:
    bool lit;

    HealDebris(double x, double y, double z, int kind);
    void renderShadow(Screen &) override {}
    void renderLight(Screen &s) override;
};

class Explosion : public SpriteObject {
public:
    int life, maxLife;

    Explosion(double x, double y, double z, int life = 5);
    int layer() const override { return 2; }
    void tick() override;
};
