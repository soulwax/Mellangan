#include "item/loot/Equip.hpp"

#include "entity/Mob.hpp"
#include "item/loot/Item.hpp"

namespace {

constexpr float BASE_SPEED = 74, BASE_MAGNET = 28;

void apply(Mob &m, Stat stat, float v) {
  ItemStats &b = m.kit();
  switch (stat) {
  case Stat::Damage:
    m.might += v / 100;
    break;
  case Stat::Burn:
    b.burn += v;
    break;
  case Stat::Chill:
    b.chill += v;
    break;
  case Stat::Poison:
    b.poison += v;
    break;
  case Stat::Shock:
    b.shock += v;
    break;
  case Stat::Bleed:
    b.bleed += v;
    break;
  case Stat::Projectiles:
    b.shots += v;
    break;
  case Stat::Area:
    b.area += v;
    break;
  case Stat::Crit:
    m.luck += v / 100;
    break;
  case Stat::Leech:
    b.leech += v;
    break;
  case Stat::Armour:
    m.armor += v;
    break;
  case Stat::Life:
    m.maxHp += v;
    m.hp = v > 0 ? m.hp + v : std::min(m.hp, m.maxHp);
    break;
  case Stat::MagicFind:
    b.magicFind += v;
    break;
  case Stat::GoldFind:
    b.goldFind += v;
    break;
  case Stat::Speed:
    m.speed += BASE_SPEED * v / 100;
    break;
  case Stat::Haste:
    m.haste -= v / 100;
    break;
  case Stat::Regen:
    m.regen += v;
    break;
  case Stat::Reach:
    m.magnet += BASE_MAGNET * v / 100;
    break;
  case Stat::Wisdom:
    b.wisdom += v;
    break;
  case Stat::Thorns:
    b.thorns += v;
    break;
  case Stat::ResistBurn:
    m.status.resist[int(StatusKind::Burn)] += v / 100;
    break;
  case Stat::ResistChill:
    m.status.resist[int(StatusKind::Chill)] += v / 100;
    break;
  case Stat::ResistPoison:
    m.status.resist[int(StatusKind::Poison)] += v / 100;
    break;
  case Stat::Kindling:
    b.kindling += v;
    break;
  case Stat::NightDamage:
    b.nightDamage += v;
    break;
  case Stat::BossDamage:
    b.bossDamage += v;
    break;
  case Stat::EliteDamage:
    b.eliteDamage += v;
    break;
  case Stat::Block:
    b.block += v;
    break;
  case Stat::ProjectileSpeed:
    b.shotSpeed += v;
    break;
  case Stat::Light:
    b.light += v;
    break;
  default:
    break;
  }
}

void curse(Mob &m, int which, float sign) {
  switch (which) {
  case 0:
    return apply(m, Stat::Life, -10 * sign);
  case 1:
    return apply(m, Stat::Speed, -5 * sign);
  case 2:
    m.kit().hunted += 5 * sign;
    return;
  case 3:
    return apply(m, Stat::Regen, -1 * sign);
  default:
    return apply(m, Stat::Light, -20 * sign);
  }
}

} // namespace

void applyItem(Mob &m, const Item &item, float sign) {
  if (item.def().implicit != Stat::None)
    apply(m, item.def().implicit, item.implicit() * sign);
  for (int i = 0; i < item.count; i++)
    apply(m, AFFIXES[item.rolls[i].affix].stat, item.rolls[i].value * sign);
  if (item.night())
    curse(m, item.curse, sign);
}
