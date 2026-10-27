#pragma once

#include <array>
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

class Arrow : public Projectile {
public:
    Arrow(Vec2 at, Vec2 vel, float damage, int pierce) : Projectile(at, vel, 1.2f, damage, 35, pierce) { radius = 5; }
    void tick() override;
    void render(Screen &s) override;
};
class Strike : public Entity {
public:
    Strike(Vec2 at, float damage, float radius);
    int layer() const override { return 2; }
    void tick() override;
    void render(Screen &s) override;
    void renderLight(Screen &s) override;

private:
    float damage, life = 0.3f;
    bool struck = false;
    std::array<float, 7> jag{};
};

class FirePool : public Entity {
public:
    FirePool(Vec2 at, float radius, float damage, float duration) : damage(damage), life(duration), maxLife(duration) {
        pos = at, this->radius = radius;
    }
    int layer() const override { return 0; }
    void tick() override;
    void render(Screen &s) override;
    void renderLight(Screen &s) override;

private:
    float damage, life, maxLife, timer = 0;
};
class Bomb : public Entity {
public:
    Bomb(Vec2 from, Vec2 to, float damage, float poolRadius, float poolDamage, float poolTime)
        : from(from), to(to), damage(damage), poolRadius(poolRadius), poolDamage(poolDamage), poolTime(poolTime) {
        pos = from;
    }
    int layer() const override { return 2; }
    void tick() override;
    void render(Screen &s) override;

private:
    Vec2 from, to;
    float damage, poolRadius, poolDamage, poolTime, t = 0, height = 0;
};
class Sweep : public Entity {
public:
    Sweep(const Player &p, float reach, float damage);
    int layer() const override { return 2; }
    void tick() override;
    void render(Screen &s) override;
    void renderLight(Screen &s) override;

private:
    const Player &player;
    float reach, damage, angle, start, t = 0;
    std::vector<entt::entity> hits;
};
class Orb : public Entity {
public:
    Orb(Vec2 at, Vec2 vel, float damage) : vel(vel), damage(damage) { pos = at; }
    int layer() const override { return 2; }
    void tick() override;
    void render(Screen &s) override;
    void renderLight(Screen &s) override;

private:
    Vec2 vel;
    float damage, life = 5;
};
