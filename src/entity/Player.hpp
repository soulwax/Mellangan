#pragma once

#include <memory>
#include <vector>

#include "entity/Entity.hpp"

class Upgrade;

class Player : public Mob {
public:
    double xAim = 1, yAim = 0;
    float invuln = 0;
    int lvl = 1, pendingLevels = 0;
    float exp = 0, expNext = 5;
    float speed = 74, might = 1, haste = 1, magnet = 28, armor = 0, regen = 0, luck = 0;
    bool god = false;
    std::vector<std::unique_ptr<Upgrade>> upgrades;

    Player();
    ~Player() override;

    void onAdded() override;
    void tick() override;
    void render(Screen &s, int xp, int yp) override;
    void renderShadow(Screen &s, int xp, int yp) override;
    void renderLight(Screen &s, int xp, int yp) override;

    void hurt(float damage);
    void heal(float amount);
    void gainXp(float amount);
    bool dead() const { return hp <= 0; }
};
