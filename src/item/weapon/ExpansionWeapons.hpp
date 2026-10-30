#pragma once

#include "entity/Entity.hpp"
#include "item/Upgrade.hpp"

class ExpansionWeapon : public Upgrade {
public:
  explicit ExpansionWeapon(int kind);
  void tick(Player &p) override;
  int kind;
  float cd = 0.4f;
};

class ExpansionAttack : public Entity {
public:
  ExpansionAttack(int kind, double x, double y, double dx, double dy,
                  float damage, float reach, int count);
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;
  int layer() const override { return 2; }
  int kind, count;
  float age = 0, damage, reach, pulse = 0;
  double ox, oy, dx, dy;
  bool returning = false;
  std::vector<entt::entity> hits;

private:
  bool hitBefore(entt::entity id) const;
  void strike(Enemy &e);
  void born();
  void trail();
  void blast();
  void impact(Enemy &e);
};
