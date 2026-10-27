#include "item/Upgrade.hpp"

#include "entity/Player.hpp"
#include "item/Passive.hpp"
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
    auto passive = [&](const char *name, Sprite icon, const char *desc, float Player::*stat, float amount) {
        u.push_back(std::make_unique<Passive>(name, icon, desc, stat, amount));
    };
    passive("Quicksilver", atlas::tile(t::PotionBlue), "+10% movement speed", &Player::speed, 7.4f);
    passive("Heartroot", atlas::tile(t::PotionRed), "+20 max health", &Player::maxHp, 20);
    passive("Ogre Hammer", atlas::tile(t::Hammer), "+10% damage", &Player::might, 0.1f);
    passive("Hourglass Tonic", atlas::tile(t::PotionGreen), "-7% weapon cooldowns", &Player::haste, -0.07f);
    passive("Lodestone", atlas::tile(t::Coin), "+30% pickup range", &Player::magnet, 8.4f);
    passive("Iron Buckler", atlas::tile(t::Shield), "Take 1 less damage per hit", &Player::armor, 1);
    passive("Troll Blood", atlas::tile(t::VialGreen), "Regenerate 0.3 health/second", &Player::regen, 0.3f);
    passive("Lucky Key", atlas::town(tt::Key), "+4% critical hit chance", &Player::luck, 0.04f);
    return u;
}
