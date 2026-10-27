#pragma once

#include "entity/Entity.hpp"

class Player;

class Pickup : public Entity {
public:
    float t = 0, speed = 0;
    bool magnet = false;

    Pickup(double x, double y) { this->x = x, this->y = y; }
    int layer() const override { return 0; }
    void tick() override;

protected:
    virtual float reach() const;
    virtual void take(Player &p) = 0;
};

class Gem : public Pickup {
public:
    float value;

    Gem(double x, double y, float value) : Pickup(x, y), value(value) {
        float dir = rng.range(0, TAU), v = rng.range(10, 30);
        xa = std::cos(dir) * v, ya = std::sin(dir) * v;
    }
    static void drop(Level &level, double x, double y, float value);

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

// Pulls every gem on the map to the player.
class Magnet : public Pickup {
public:
    using Pickup::Pickup;
    void render(Screen &s) override;
    void renderLight(Screen &s) override;

protected:
    void take(Player &p) override;
};

// Starfire vial: blasts every enemy near the player.
class Nova : public Pickup {
public:
    using Pickup::Pickup;
    void render(Screen &s) override;
    void renderLight(Screen &s) override;

protected:
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
