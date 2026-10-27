#pragma once

#include "entity/enemy/Enemy.hpp"

class Cultist : public Enemy {
public:
    using Enemy::Enemy;
    void tick() override;

protected:
    void steer(double xn, double yn, double &xs, double &ys) override;

private:
    float castTimer = 2.5f;
};
