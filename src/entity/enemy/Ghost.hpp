#pragma once

#include "entity/enemy/Enemy.hpp"

class Ghost : public Enemy {
public:
    using Enemy::Enemy;
    void renderLight(Screen &s) override;

protected:
    void steer(double xn, double yn, double &xs, double &ys) override;
    Color tint() const override { return col::white.alpha(0.72f); }
};
