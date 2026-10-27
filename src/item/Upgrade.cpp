#include "item/Upgrade.hpp"

#include "entity/Player.hpp"
#include "item/Passive.hpp"
#include "item/relic/Relic.hpp"
#include "item/relic/SunfireCrown.hpp"
#include "item/relic/TempestEye.hpp"
#include "item/weapon/AxeThrow.hpp"
#include "item/weapon/Blade.hpp"
#include "item/weapon/Bow.hpp"
#include "item/weapon/FireBomb.hpp"
#include "item/weapon/Halo.hpp"
#include "item/weapon/Knives.hpp"
#include "item/weapon/Scythe.hpp"
#include "item/weapon/Staff.hpp"
#include "item/weapon/StormRod.hpp"

namespace {
namespace t = atlas::t;
namespace tt = atlas::tt;
namespace kt = atlas::kt;

void phoenix(Player &p) { p.revives++; }
void grove(Player &p) { p.maxHp += 60, p.hp += 60, p.regen += 1.5f; }
void frozen(Player &p) { p.haste -= 0.25f, p.speed += 11; }
void signet(Player &p) { p.might += 0.5f, p.armor += 2; }
} // namespace

std::vector<std::unique_ptr<Upgrade>> Upgrade::all() {
  std::vector<std::unique_ptr<Upgrade>> u;
  u.push_back(std::make_unique<Staff>());
  u.push_back(std::make_unique<Blade>());
  u.push_back(std::make_unique<Halo>());
  u.push_back(std::make_unique<Knives>());
  u.push_back(std::make_unique<AxeThrow>());
  u.push_back(std::make_unique<StormRod>());
  u.push_back(std::make_unique<FireBomb>());
  u.push_back(std::make_unique<Bow>());
  u.push_back(std::make_unique<Scythe>());
  auto passive = [&](const char *name, Sprite icon, const char *desc,
                     float Player::*stat, float amount) {
    u.push_back(std::make_unique<Passive>(name, icon, desc, stat, amount));
  };
  passive("Quicksilver", atlas::tile(t::PotionBlue), "+10% movement speed",
          &Player::speed, 7.4f);
  passive("Heartroot", atlas::tile(t::PotionRed), "+20 max health",
          &Player::maxHp, 20);
  passive("Ogre Hammer", atlas::tile(t::Hammer), "+10% damage", &Player::might,
          0.1f);
  passive("Hourglass Tonic", atlas::tile(t::PotionGreen),
          "-7% weapon cooldowns", &Player::haste, -0.07f);
  passive("Lodestone", atlas::tile(t::Coin), "+30% pickup range",
          &Player::magnet, 8.4f);
  passive("Iron Buckler", atlas::tile(t::Shield), "Take 1 less damage per hit",
          &Player::armor, 1);
  passive("Troll Blood", atlas::tile(t::VialGreen),
          "Regenerate 0.3 health/second", &Player::regen, 0.3f);
  passive("Lucky Key", atlas::town(tt::Key), "+4% critical hit chance",
          &Player::luck, 0.04f);
  u.push_back(std::make_unique<SunfireCrown>());
  u.push_back(std::make_unique<TempestEye>());
  u.push_back(std::make_unique<Relic>("Phoenix Feather", atlas::kit(kt::Poppies),
                                      "Rise once from death in a blaze",
                                      phoenix));
  u.push_back(std::make_unique<Relic>("Heart of the Grove",
                                      atlas::kit(kt::Clover),
                                      "+60 max health, regenerate 1.5/s",
                                      grove));
  u.push_back(std::make_unique<Relic>("Frozen Hourglass",
                                      atlas::kit(kt::IceChunk),
                                      "-25% weapon cooldowns, +15% speed",
                                      frozen));
  u.push_back(std::make_unique<Relic>("Obsidian Signet",
                                      atlas::kit(kt::Obsidian),
                                      "+50% damage, take 2 less per hit",
                                      signet));
  return u;
}
