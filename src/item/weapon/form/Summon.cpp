#include "item/weapon/form/Forms.hpp"

#include "audio/Sound.hpp"
#include "entity/Ally.hpp"
#include "item/Upgrade.hpp"
#include "level/Level.hpp"

void SummonForm::fire(const FormCast &c) const {
  if (c.p.hostile())
    return;
  Level &level = *c.p.level;
  AllyKind kind = allyFor(c.r.name);
  const AllyDef &def = allyDef(kind);
  bool hardy = c.source && c.source->level >= 4;
  float hp = def.hp * (1 + 0.6f * hardy + 0.4f * (c.source && c.source->evolved));
  float haste = hardy ? 0.75f : 1;
  int alive = 0;
  for (Ally *a : level.allies)
    if (!a->removed && a->source == c.source && &a->def == &def) {
      alive++;
      a->life = std::max(a->life, def.life), a->damage = c.damage;
      a->hp += std::max(0.0f, hp - a->maxHp), a->maxHp = hp, a->haste = haste;
    }
  int cap = c.count + int(c.p.bonus().allyCap);
  for (int i = alive; i < cap; i++) {
    double a = c.base + TAU * i / cap;
    Ally *made = summon(level, kind, c.p.x + std::cos(a) * 20, c.p.y + std::sin(a) * 14,
                        c.damage, def.life, c.r.payload, c.source);
    made->might = c.p.might, made->hp = made->maxHp = hp, made->haste = haste;
  }
  if (alive < cap)
    Sound::cast.play(0.25f, 0.05f, 0.9f);
}
