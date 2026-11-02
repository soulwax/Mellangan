#include "item/weapon/form/Forms.hpp"

#include "audio/Sound.hpp"
#include "entity/Ally.hpp"
#include "level/Level.hpp"

void SummonForm::fire(const FormCast &c) const {
  if (c.p.hostile())
    return;
  Level &level = *c.p.level;
  AllyKind kind = allyFor(c.r.name);
  const AllyDef &def = allyDef(kind);
  int alive = 0;
  for (Ally *a : level.allies)
    if (!a->removed && a->source == c.source && &a->def == &def) {
      alive++;
      a->life = std::max(a->life, def.life), a->damage = c.damage;
    }
  int cap = c.count + int(c.p.bonus.allyCap);
  for (int i = alive; i < cap; i++) {
    double a = c.base + TAU * i / cap;
    summon(level, kind, c.p.x + std::cos(a) * 20, c.p.y + std::sin(a) * 14, c.damage, def.life,
           c.r.payload, c.source)
        ->might = c.p.might;
  }
  if (alive < cap)
    Sound::cast.play(0.25f, 0.05f, 0.9f);
}
