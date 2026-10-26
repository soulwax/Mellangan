#pragma once

#include <vector>

#include "entity/Entity.hpp"

class Enemy;
class Player;

class Projectile : public Entity {
public:
    Vec2 vel;
    float life, maxLife, damage, knock;
    int pierce;
    std::vector<entt::entity> hits;

    Projectile(Vec2 at, Vec2 vel, float life, float damage, float knock, int pierce)
        : vel(vel), life(life), maxLife(life), damage(damage), knock(knock), pierce(pierce) { pos = at; }
    int layer() const override { return 2; }

protected:
    void collide();
    virtual bool touches(const Enemy &e) const;
    virtual Vec2 push(const Enemy &) const { return normalized(vel); }
    virtual void spent() {}
    float frac() const { return life / maxLife; }
};

class Bolt : public Projectile {
public:
    entt::entity target;

    Bolt(Vec2 at, Vec2 vel, float damage, int pierce, entt::entity target);
    void tick() override;
    void render(Screen &s) override;
    void renderLight(Screen &s) override;

protected:
    void spent() override;
};

class Slash : public Projectile {
public:
    Slash(Player &p, float side, float damage, float scale);
    void tick() override;
    void render(Screen &s) override;
    void renderLight(Screen &s) override;

protected:
    bool touches(const Enemy &e) const override;
    Vec2 push(const Enemy &) const override { return {side, 0}; }

private:
    Player &player;
    float side, scale;
};

class Axe : public Projectile {
public:
    Axe(Vec2 at, Vec2 vel, float damage, int pierce, float spin);
    void tick() override;
    void render(Screen &s) override;
    void renderLight(Screen &s) override;

private:
    float rot = 0, spin;
};
