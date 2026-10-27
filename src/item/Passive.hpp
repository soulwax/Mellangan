#pragma once

#include "item/Upgrade.hpp"

class Passive : public Upgrade {
public:
    Passive(const char *name, Sprite icon, const char *desc, float Player::*stat, float amount)
        : Upgrade(name, icon, {desc, desc, desc, desc, desc}, false), stat(stat), amount(amount) {}
    void levelUp(Player &p) override;

private:
    float Player::*stat;
    float amount;
};
