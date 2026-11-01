#include "item/skill/Skill.hpp"

#include "entity/Player.hpp"
#include "hero/Hero.hpp"

Skill::Skill(const char *name, Sprite icon, const char *desc,
             const char *owner, float cooldown)
    : Upgrade(name, icon, {desc, desc, desc, desc, desc}, false),
      cooldown(cooldown) {
  this->owner = owner;
  skill = true;
}

void Skill::levelUp(Mob &m) {
  Player *p = m.asPlayer();
  if (!p) {
    level = 1;
    return;
  }
  if (p->skill == this)
    return;
  if (p->skill)
    p->skill->unequip(*p);
  p->skill = this;
  p->skillTime = 0;
}

HeroSkill::HeroSkill(const Hero &hero)
    : Skill(hero.skill, hero.sprite(), hero.skillDesc, hero.name,
            hero.cooldown),
      hero(hero) {}

void HeroSkill::use(Mob &p) { hero.cast(p); }

void addSkills(std::vector<std::unique_ptr<Upgrade>> &u) {
  for (const Hero *h : HEROES)
    u.push_back(std::make_unique<HeroSkill>(*h));
  addMageSkills(u);
  addArtSkills(u);
}
