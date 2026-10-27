#pragma once

#include <memory>

#include "entity/Mob.hpp"

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
    float occlusion(float &width) const override { return type.floats ? (width = 0, 0.0f) : Mob::occlusion(width); }
    void renderLight(Screen &s) override;
    void hurt(float damage, double xd, double yd, float force);
    void makeElite();

protected:
    virtual void steer(double xn, double yn, double &xs, double &ys) {
        xs = xn * type.speed;
        ys = yn * type.speed;
    }
    virtual Color tint() const { return col::white; }
    virtual void die();

private:
    void separate();
};
