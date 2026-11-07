#include "entity/Faction.hpp"

#include <iterator>

#include "Math.hpp"

namespace {

using K = EnemyKind;
using F = Faction;

struct Member {
  EnemyKind kind;
  Faction faction;
};

constexpr Member MEMBERS[] = {
    {K::Bat, F::Wildkin},
    {K::Rat, F::Wildkin},
    {K::Slime, F::Bogspawn},
    {K::Scorpion, F::Wildkin},
    {K::Spider, F::Wildkin},
    {K::Ghost, F::BarrowHost},
    {K::Cultist, F::Cinderbound},
    {K::Demon, F::Cinderbound},
    {K::Knight, F::Freeblades},
    {K::Cyclops, F::Wildkin},
    {K::Wolf, F::Wildkin},
    {K::Alpha, F::Wildkin},
    {K::Burrower, F::Wildkin},
    {K::Necromancer, F::BarrowHost},
    {K::Skeleton, F::BarrowHost},
    {K::Thief, F::Freeblades},
    {K::MireLeech, F::Bogspawn},
    {K::BogLantern, F::Bogspawn},
    {K::GlassMantis, F::Glasskin},
    {K::DustVulture, F::Wildkin},
    {K::FrostWisp, F::Rimefolk},
    {K::Iceback, F::Rimefolk},
    {K::Sporeling, F::Mycel},
    {K::MycelialBrute, F::Mycel},
    {K::EmberImp, F::Cinderbound},
    {K::CinderGolem, F::Cinderbound},
    {K::Mimic, F::Wildkin},
    {K::Outlaw, F::Freeblades},
    {K::Berserker, F::Freeblades},
    {K::PitFighter, F::Freeblades},
    {K::Deserter, F::Freeblades},
    {K::Thornling, F::Wildkin},
    {K::Boar, F::Wildkin},
    {K::GoblinArcher, F::Freeblades},
    {K::GoblinShaman, F::Freeblades},
    {K::Sapling, F::Wildkin},
    {K::Treant, F::Wildkin},
    {K::MothWisp, F::Wildkin},
    {K::Cutthroat, F::Freeblades},
    {K::BogToad, F::Bogspawn},
    {K::Drowned, F::Bogspawn},
    {K::Snapjaw, F::Bogspawn},
    {K::StranglerVine, F::Wildkin},
    {K::AmberBeetle, F::Wildkin},
    {K::SporePuffer, F::Mycel},
    {K::SunkenPriest, F::Bogspawn},
    {K::Yeti, F::Rimefolk},
    {K::IceWraith, F::Rimefolk},
    {K::BarrowWight, F::BarrowHost},
    {K::CrystalGolem, F::Glasskin},
    {K::CrystalShard, F::Glasskin},
    {K::MoonHound, F::Starborn},
    {K::Lynx, F::Wildkin},
    {K::BarrowArcher, F::BarrowHost},
    {K::SandWyrm, F::Wildkin},
    {K::Hyena, F::Wildkin},
    {K::SaltGolem, F::Glasskin},
    {K::GlassSpitter, F::Glasskin},
    {K::StormKite, F::Wildkin},
    {K::DuneRaider, F::Freeblades},
    {K::TotemShaman, F::Freeblades},
    {K::WarTotem, F::Freeblades},
    {K::MagmaSlug, F::Cinderbound},
    {K::AshWraith, F::Cinderbound},
    {K::CaveHorror, F::Starborn},
    {K::StarSpawn, F::Starborn},
    {K::Starlet, F::Starborn},
    {K::FlameCultist, F::Cinderbound},
    {K::HollowSentinel, F::BarrowHost},
    {K::EmberDrake, F::Cinderbound},
    {K::NightStalker, F::Starborn},
    {K::GraveRobber, F::Freeblades},
    {K::CaveBat, F::Wildkin},
    {K::RockCrawler, F::Wildkin},
    {K::OreGolem, F::Glasskin},
    {K::KoboldMiner, F::Freeblades},
    {K::Shade, F::BarrowHost},
    {K::Nest, F::Wildkin},
    {K::Lamplighter, F::Synod},
    {K::Penitent, F::Synod},
    {K::Censer, F::Synod},
    {K::Limer, F::Synod},
    {K::Confessor, F::Synod},
    {K::Prioress, F::Synod}};

constexpr bool ordered() {
  for (int i = 0; i < int(std::size(MEMBERS)); i++)
    if (MEMBERS[i].kind != EnemyKind(i))
      return false;
  return std::size(MEMBERS) == size_t(EnemyKind::Count);
}
static_assert(ordered());

constexpr const char *NAMES[] = {"WILDKIN",  "BARROW HOST", "MYCEL",      "CINDERBOUND", "RIMEFOLK", "GLASSKIN",
                                 "BOGSPAWN", "STARBORN",    "FREEBLADES", "LANTERN SYNOD", "HEARTH"};
static_assert(std::size(NAMES) == size_t(Faction::Count));

} // namespace

Faction factionOf(EnemyKind kind) {
  int k = int(kind);
  return k >= 0 && k < int(std::size(MEMBERS)) ? MEMBERS[k].faction : Faction::Wildkin;
}

Faction factionOf(const Enemy &e) { return factionOf(EnemyKind(e.kind())); }

const char *factionName(Faction f) { return NAMES[int(f)]; }

Grudges Grudges::roll(uint32_t seed) {
  Grudges g;
  for (int tries = 0; g.count < MAX && tries < 64; tries++) {
    uint32_t h = hash2(int(seed) * 7 + tries * 131, int(seed >> 7) - tries * 17);
    Faction a = Faction(h % FAMILIES), b = Faction((h >> 8) % FAMILIES);
    if (a == b || g.between(a, b))
      continue;
    g.pair[g.count][0] = a, g.pair[g.count][1] = b, g.count++;
  }
  return g;
}

bool unclean(Faction f) {
  for (Faction u : UNCLEAN)
    if (u == f)
      return true;
  return false;
}

bool Grudges::between(Faction a, Faction b) const {
  if (a == Faction::Synod || b == Faction::Synod)
    return unclean(a == Faction::Synod ? b : a);
  for (int i = 0; i < count; i++)
    if ((pair[i][0] == a && pair[i][1] == b) || (pair[i][0] == b && pair[i][1] == a))
      return true;
  return false;
}
