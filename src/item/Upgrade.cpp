#include "item/Upgrade.hpp"

#include <cstring>

#include "entity/Player.hpp"
#include "item/Passive.hpp"
#include "item/relic/Relic.hpp"
#include "item/weapon/Meteor.hpp"
#include "item/weapon/Scattershot.hpp"
#include "item/weapon/CrusaderStrike.hpp"
#include "item/weapon/Transmute.hpp"
#include "item/weapon/StaticField.hpp"
#include "item/weapon/SoulSiphon.hpp"
#include "item/weapon/Aegis.hpp"
#include "item/weapon/Flurry.hpp"
#include "item/talent/EchoingBlink.hpp"
#include "item/talent/SecondWind.hpp"
#include "item/talent/Retribution.hpp"
#include "item/talent/VolatileMix.hpp"
#include "item/talent/Overcharge.hpp"
#include "item/talent/Harvest.hpp"
#include "item/talent/Beacon.hpp"
#include "item/talent/Riposte.hpp"
#include "item/weapon/FrostNova.hpp"
#include "item/weapon/Radiance.hpp"
#include "item/weapon/WhirlingEdge.hpp"
#include "item/weapon/PiercingShot.hpp"
#include "item/weapon/Judgement.hpp"
#include "item/weapon/AcidFlask.hpp"
#include "item/weapon/ChainLightning.hpp"
#include "item/weapon/WraithBolts.hpp"
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

const char *const EVOLUTIONS[][3] = {
    {"Arcane Staff", "Hourglass Tonic", "Archmage's Staff"},
    {"Moon Blade", "Ogre Hammer", "Eclipse Blade"},
    {"Warding Halo", "Iron Buckler", "Sun Ward"},
    {"Orbiting Knives", "Quicksilver", "Whirling Steel"},
    {"Hurled Axe", "Heartroot", "Titan's Axe"},
    {"Storm Rod", "Lucky Key", "Thunder Sceptre"},
    {"Fire Bomb", "Troll Blood", "Hellfire Flask"},
    {"Hunter's Bow", "Lodestone", "Windpiercer"},
    {"Reaper's Scythe", "Ogre Hammer", "Harvest Moon"}};

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
  u.push_back(std::make_unique<FrostNova>());
  u.push_back(std::make_unique<PiercingShot>());
  u.push_back(std::make_unique<Judgement>());
  u.push_back(std::make_unique<AcidFlask>());
  u.push_back(std::make_unique<ChainLightning>());
  u.push_back(std::make_unique<WraithBolts>());
  u.push_back(std::make_unique<Radiance>());
  u.push_back(std::make_unique<WhirlingEdge>());
  u.push_back(std::make_unique<Meteor>());
  u.push_back(std::make_unique<Scattershot>());
  u.push_back(std::make_unique<CrusaderStrike>());
  u.push_back(std::make_unique<Transmute>());
  u.push_back(std::make_unique<StaticField>());
  u.push_back(std::make_unique<SoulSiphon>());
  u.push_back(std::make_unique<Aegis>());
  u.push_back(std::make_unique<Flurry>());
  u.push_back(std::make_unique<EchoingBlink>());
  u.push_back(std::make_unique<SecondWind>());
  u.push_back(std::make_unique<Retribution>());
  u.push_back(std::make_unique<VolatileMix>());
  u.push_back(std::make_unique<Overcharge>());
  u.push_back(std::make_unique<Harvest>());
  u.push_back(std::make_unique<Beacon>());
  u.push_back(std::make_unique<Riposte>());
  auto kin = [&](const char *owner, const char *name, Sprite icon,
                 const char *desc, float Player::*stat, float amount) {
    passive(name, icon, desc, stat, amount);
    u.back()->owner = owner;
  };
  kin("MAGE", "Arcane Mind", atlas::tile(128), "-6% weapon cooldowns",
      &Player::haste, -0.06f);
  kin("RANGER", "Fleet Foot", atlas::kit(kt::Fern), "+8% movement speed",
      &Player::speed, 6);
  kin("KNIGHT", "Oathbound", atlas::tile(74), "+25 max health",
      &Player::maxHp, 25);
  kin("ALCHEMIST", "Distillation", atlas::kit(kt::Glowshrooms),
      "Regenerate 0.4 health/second", &Player::regen, 0.4f);
  kin("STORMCALLER", "Conduit", atlas::kit(kt::Lavender),
      "+5% critical hit chance", &Player::luck, 0.05f);
  kin("REAPER", "Bloodlust", atlas::kit(kt::Roses), "+8% damage",
      &Player::might, 0.08f);
  kin("PRIESTESS", "Grace", atlas::kit(kt::CallaLilies),
      "Take 0.5 less damage per hit", &Player::armor, 0.5f);
  kin("DUELIST", "Keen Eye", atlas::kit(kt::Snowdrops),
      "+5% critical hit chance", &Player::luck, 0.05f);
  for (auto &w : u)
    for (auto &e : EVOLUTIONS)
      if (std::strcmp(w->name, e[0]) == 0)
        w->partner = e[1], w->evolvedName = e[2];
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

bool Upgrade::canEvolve(const Player &p) const {
  if (!partner || evolved || level < MAX_LEVEL)
    return false;
  for (auto &u : p.upgrades)
    if (u->level > 0 && std::strcmp(u->name, partner) == 0)
      return true;
  return false;
}
