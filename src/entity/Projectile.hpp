#pragma once

#include <array>
#include <vector>

#include "entity/Entity.hpp"

class Enemy;
class Player;

class Projectile : public Entity {
public:
    float life, maxLife, damage, knock;
    int pierce;
    std::vector<entt::entity> hits;

    Projectile(double x, double y, double xa, double ya, float life, float damage, float knock, int pierce)
        : life(life), maxLife(life), damage(damage), knock(knock), pierce(pierce) {
        this->x = x, this->y = y;
        this->xa = xa, this->ya = ya;
    }
    int layer() const override { return 2; }

protected:
    void collide();
    virtual bool touches(const Enemy &e) const;
    virtual void push(const Enemy &, double &xd, double &yd) const { xd = xa, yd = ya; }
    virtual void spent() {}
    float frac() const { return life / maxLife; }
};

class Bolt : public Projectile {
public:
    entt::entity target;

    Bolt(double x, double y, double xa, double ya, float damage, int pierce, entt::entity target);
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
    void push(const Enemy &, double &xd, double &yd) const override { xd = side, yd = 0; }

private:
    Player &player;
    float side, scale;
};

class Axe : public Projectile {
public:
    Axe(double x, double y, double xa, double ya, float damage, int pierce, float spin);
    void tick() override;
    void render(Screen &s) override;
    void renderLight(Screen &s) override;

private:
    float rot = 0, spin;
};

class Arrow : public Projectile {
public:
    Arrow(double x, double y, double xa, double ya, float damage, int pierce)
        : Projectile(x, y, xa, ya, 1.2f, damage, 35, pierce) {
        radius = 5;
    }
    void tick() override;
    void render(Screen &s) override;
};
class Strike : public Entity {
public:
    Strike(double x, double y, float damage, float radius);
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
    FirePool(double x, double y, float radius, float damage, float duration) : damage(damage), life(duration), maxLife(duration) {
        this->x = x, this->y = y, this->radius = radius;
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
    Bomb(double x0, double y0, double x1, double y1, float damage, float poolRadius, float poolDamage, float poolTime)
        : x0(x0), y0(y0), x1(x1), y1(y1), damage(damage), poolRadius(poolRadius), poolDamage(poolDamage), poolTime(poolTime) {
        x = x0, y = y0;
    }
    int layer() const override { return 2; }
    void tick() override;
    void render(Screen &s) override;

private:
    double x0, y0, x1, y1;
    float damage, poolRadius, poolDamage, poolTime, t = 0;
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
    Orb(double x, double y, double xa, double ya, float damage) : damage(damage) {
        this->x = x, this->y = y;
        this->xa = xa, this->ya = ya;
    }
    int layer() const override { return 2; }
    void tick() override;
    void render(Screen &s) override;
    void renderLight(Screen &s) override;

private:
    float damage, life = 5;
};
