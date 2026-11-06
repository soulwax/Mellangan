#pragma once

#include "entity/Entity.hpp"
#include "item/Upgrade.hpp"

namespace xw {
enum Kind { Sling, Harpoon, Sigil, Boomerang, Mortar, Lash, Prism, Swarm, Count };
} // namespace xw

class ExpansionWeapon : public Upgrade {
public:
  explicit ExpansionWeapon(int kind);
  void tick(Mob &p) override;
  Figures figures(int lvl, bool evolved) const override;
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
  bool returning = false, landed = false;
  std::vector<entt::entity> hits;

private:
  bool hitBefore(entt::entity id) const;
  Mob *freshFoe(float range) const;
  void steer(double tx, double ty);
  void strike(Mob &e);
  void sweep(bool area, bool line, Mob *&bounced);
  void born();
  void trail();
  void blast();
  bool evolved() const;
  void spend();
  void burst(double bx, double by, float r, float k);
  void converge(float r);
  void crater();
  void impact(Mob &e);
  void renderBolt(Screen &s, Color c) const;
};
