#pragma once

#include "item/Upgrade.hpp"

class Scythe : public Upgrade {
public:
    Scythe();
    void tick(Player &p) override;

private:
    float cd = 1;
};
