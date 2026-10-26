#pragma once

#include <memory>

#include "entity/Entity.hpp"

enum class EnemyKind { Bat, Rat, Slime, Spider, Ghost, Demon, Cyclops };

struct EnemyType {
    int tile;
    float hp, speed, damage, radius, scale, xp;
    Color blood;
    bool floats;
    float unlockAt;
};

class Enemy : public Mob {
public:
    const EnemyType &type;
    float damageMul, knockResist = 1, knifeCooldown = 0;

    Enemy(const EnemyType &type, float hpMul, float damageMul);
    static const EnemyType &typeOf(EnemyKind kind);
    static std::unique_ptr<Enemy> create(EnemyKind kind, float hpMul, float damageMul);

    void onAdded() override;
    void tick() override;
    void render(Screen &s) override;
    void renderShadow(Screen &s) override;
    void hurt(float damage, Vec2 dir, float force);

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

class Ghost : public Enemy {
public:
    using Enemy::Enemy;
    void renderLight(Screen &s) override;

protected:
    Vec2 steer(Vec2 dir) override;
    Color tint() const override { return col::white.alpha(0.72f); }
};

class Cyclops : public Enemy {
public:
    Cyclops(const EnemyType &type, float hpMul, float damageMul);
    void renderLight(Screen &s) override;

protected:
    void die() override;
};
