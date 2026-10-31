#pragma once

#include "item/Upgrade.hpp"

class Hero;

class Skill : public Upgrade {
public:
  float cooldown;

  Skill(const char *name, Sprite icon, const char *desc, const char *owner,
        float cooldown);
  virtual void use(Player &p) = 0;
  virtual void active(Player &) {}
  virtual void unequip(Player &) {}
  void levelUp(Player &p) override;
};

class HeroSkill : public Skill {
public:
  explicit HeroSkill(const Hero &hero);
  void use(Player &p) override;

  const Hero &hero;
};

void addSkills(std::vector<std::unique_ptr<Upgrade>> &u);
