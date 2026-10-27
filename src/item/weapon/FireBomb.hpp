#pragma once

#include "item/Upgrade.hpp"

class FireBomb : public Upgrade {
public:
    FireBomb();
    void tick(Player &p) override;

private:
    float cd = 1.2f;
};
