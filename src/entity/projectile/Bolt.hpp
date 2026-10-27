#pragma once

#include "entity/projectile/Projectile.hpp"

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
