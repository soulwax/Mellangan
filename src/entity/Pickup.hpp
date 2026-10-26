#pragma once

#include "entity/Entity.hpp"

class Player;

class Pickup : public Entity {
public:
    float t = 0, speed = 0;
    bool magnet = false;

    explicit Pickup(Vec2 at) { pos = at; }
    int layer() const override { return 0; }
    void tick() override;

protected:
    virtual float reach() const;
    virtual void take(Player &p) = 0;
    float bob() const { return std::sin(t * 4 + pos.x) * 1.5f; }
};

class Gem : public Pickup {
public:
    float value;
    Vec2 vel;

    Gem(Vec2 at, float value) : Pickup(at), value(value), vel(rng.onCircle() * rng.range(10, 30)) {}
    static void drop(Level &level, Vec2 at, float value);

    void onAdded() override;
    void tick() override;
    void render(Screen &s) override;
    void renderLight(Screen &s) override;

protected:
    void take(Player &p) override;

private:
    int tier() const { return value < 3 ? 0 : value < 12 ? 1 : 2; }
    Color color() const;
};

class Potion : public Pickup {
public:
    using Pickup::Pickup;
    void render(Screen &s) override;
    void renderLight(Screen &s) override;

protected:
    float reach() const override;
    void take(Player &p) override;
};

class Chest : public Pickup {
public:
    using Pickup::Pickup;
    void render(Screen &s) override;
    void renderLight(Screen &s) override;

protected:
    void take(Player &p) override;
};
