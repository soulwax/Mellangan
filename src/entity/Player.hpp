#pragma once

#include <memory>
#include <vector>

#include "entity/Entity.hpp"

class Upgrade;

class Player : public Mob {
public:
    Vec2 vel;
    float invuln = 0;
    int lvl = 1, pendingLevels = 0;
    float xp = 0, xpNext = 5;
    float speed = 74, might = 1, haste = 1, magnet = 28;
    bool god = false;
    std::vector<std::unique_ptr<Upgrade>> upgrades;

    Player();
    ~Player() override;

    void onAdded() override;
    void tick() override;
    void render(Screen &s) override;
    void renderShadow(Screen &s) override;
    void renderLight(Screen &s) override;

    void hurt(float damage);
    void heal(float amount);
    void gainXp(float amount);
    bool dead() const { return hp <= 0; }
};
