#pragma once

#include <memory>

#include "entity/Entity.hpp"

// Cyclops stays last: the spawner draws regular enemies from every kind before it.
enum class EnemyKind { Bat, Rat, Slime, Scorpion, Spider, Ghost, Cultist, Demon, Knight, Cyclops };

struct EnemyType {
    int tile;
    float hp, speed, damage, radius, scale, xp, knockResist;
    Color blood;
    bool floats;
    float unlockAt;
};

class Enemy : public Mob {
public:
    const EnemyType &type;
    float damageMul, knifeCooldown = 0;
    bool elite = false;

    Enemy(const EnemyType &type, float hpMul, float damageMul);
    static const EnemyType &typeOf(EnemyKind kind);
    static std::unique_ptr<Enemy> create(EnemyKind kind, float hpMul, float damageMul);

    void onAdded() override;
    void tick() override;
    void render(Screen &s) override;
    void renderShadow(Screen &s) override;
    void renderLight(Screen &s) override;
    void hurt(float damage, Vec2 dir, float force);
    void makeElite();

protected:
    virtual Vec2 steer(Vec2 dir) { return dir * type.speed; }
    virtual Color tint() const { return col::white; }
    virtual void die();

private:
    void separate();
};

class Bat : public Enemy {
public:
    using Enemy::Enemy;

protected:
    Vec2 steer(Vec2 dir) override;
};

class Scorpion : public Enemy {
public:
    using Enemy::Enemy;

protected:
    Vec2 steer(Vec2 dir) override;
};

class Cultist : public Enemy {
public:
    using Enemy::Enemy;
    void tick() override;

protected:
    Vec2 steer(Vec2 dir) override;

private:
    float castTimer = 2.5f;
};

class Ghost : public Enemy {
public:
    using Enemy::Enemy;
    void renderLight(Screen &s) override;

protected:
    Vec2 steer(Vec2 dir) override;
    Color tint() const override { return col::white.alpha(0.72f); }
};

// Breakable supplies: an Enemy so every weapon can hit it, but it never moves, attacks or counts as a kill.
class SupplyBarrel : public Enemy {
public:
    SupplyBarrel();
    void tick() override;
    void render(Screen &s) override;
    void renderLight(Screen &s) override;

protected:
    void die() override;
};

class Cyclops : public Enemy {
public:
    using Enemy::Enemy;
    void renderLight(Screen &s) override;

protected:
    void die() override;
};
