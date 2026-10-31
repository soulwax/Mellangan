#pragma once

#include "item/Upgrade.hpp"

class Hero;

class Skill : public Upgrade {
public:
  float cooldown;

  Skill(const char *name, Sprite icon, const char *desc, const char *owner,
        float cooldown);
  virtual void use(Mob &p) = 0;
  virtual void active(Mob &) {}
  virtual void unequip(Mob &) {}
  void levelUp(Mob &p) override;
};

class HeroSkill : public Skill {
public:
  explicit HeroSkill(const Hero &hero);
  void use(Mob &p) override;

  const Hero &hero;
};

void addSkills(std::vector<std::unique_ptr<Upgrade>> &u);
