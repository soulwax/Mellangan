#pragma once

#include "entity/projectile/Projectile.hpp"

enum class BoltStyle { Arcane, Wraith, Tinker };

class Bolt : public Projectile {
public:
  entt::entity target;
  BoltStyle style;

  Bolt(double x, double y, double xa, double ya, float damage, int pierce,
       entt::entity target, BoltStyle style = BoltStyle::Arcane);
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;

protected:
  void spent() override;

private:
  float age = 0;
  float scale() const;
};
