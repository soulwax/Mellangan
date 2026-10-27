#pragma once

#include "entity/enemy/Enemy.hpp"

class Scorpion : public Enemy {
public:
    using Enemy::Enemy;

protected:
    void steer(double xn, double yn, double &xs, double &ys) override;
};
