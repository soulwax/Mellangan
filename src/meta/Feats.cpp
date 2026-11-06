#include "meta/Feats.hpp"

#include <bit>
#include <cstring>

#include "Game.hpp"
#include "entity/Player.hpp"
#include "hero/Hero.hpp"
#include "item/Upgrade.hpp"
#include "level/Level.hpp"

namespace {
const Codex &codex(const Game &g) { return g.codex; }
const Level &run(const Game &g) { return *g.level; }

template <int H> bool dawnAs(const Game &g, bool dawn) {
  return dawn && run(g).player->hero == HEROES[H];
}

int evolved(const Game &g) {
  int n = 0;
  for (auto &up : run(g).player->upgrades)
    n += up->evolved;
  return n;
}

int weapons(const Game &g) {
  int n = 0;
  for (auto &up : run(g).player->upgrades)
    n += up->weapon && up->level > 0;
  return n;
}

bool everySite(const Game &g) {
  for (int k : codex(g).entered)
    if (k <= 0)
      return false;
  return true;
}

bool everyClass(const Game &g, bool);
} // namespace

const Feat FEATS[] = {
    {"first_dawn", "FIRST LIGHT", "Reach dawn once", [](const Game &g, bool) { return codex(g).dawns >= 1; }},
    {"ten_dawns", "KEEPER OF MORNINGS", "Reach dawn 10 times", [](const Game &g, bool) { return codex(g).dawns >= 10; }},
    {"fifty_runs", "RESTLESS", "Play 50 runs", [](const Game &g, bool) { return codex(g).runs >= 50; }},
    {"kills_1k", "THINNING THE DARK", "Slay 1,000 foes in all", [](const Game &g, bool) { return codex(g).kills() >= 1000; }},
    {"kills_10k", "TEN THOUSAND", "Slay 10,000 foes in all", [](const Game &g, bool) { return codex(g).kills() >= 10000; }},
    {"kills_100k", "THE DARK RECEDES", "Slay 100,000 foes in all", [](const Game &g, bool) { return codex(g).kills() >= 100000; }},
    {"kills_run_5k", "FLOOD", "Slay 5,000 foes in one run", [](const Game &g, bool) { return run(g).kills >= 5000; }},
    {"streak_50", "ON A ROLL", "Reach a 50-kill streak", [](const Game &g, bool) { return codex(g).bestStreak >= 50; }},
    {"streak_150", "UNBROKEN", "Reach a 150-kill streak", [](const Game &g, bool) { return codex(g).bestStreak >= 150; }},
    {"streak_300", "RELENTLESS", "Reach a 300-kill streak", [](const Game &g, bool) { return codex(g).bestStreak >= 300; }},
    {"level_20", "SEASONED", "Reach level 20 in one run", [](const Game &g, bool) { return run(g).player->lvl >= 20; }},
    {"level_40", "ASCENDANT", "Reach level 40 in one run", [](const Game &g, bool) { return run(g).player->lvl >= 40; }},
    {"biomes_10", "WANDERER", "Walk 10 different lands", [](const Game &g, bool) { return codex(g).count(codex(g).biomes) >= 10; }},
    {"biomes_20", "FAR TRAVELLER", "Walk 20 different lands", [](const Game &g, bool) { return codex(g).count(codex(g).biomes) >= 20; }},
    {"biomes_all", "EVERY SHORE", "Walk every land there is", [](const Game &g, bool) { return codex(g).count(codex(g).biomes) >= BIOME_COUNT; }},
    {"relics_5", "COLLECTOR", "Find 5 different relics", [](const Game &g, bool) { return codex(g).count(codex(g).relics) >= 5; }},
    {"sites_all", "CARTOGRAPHER", "Enter every kind of place at least once", [](const Game &g, bool) { return everySite(g); }},
    {"shrines_10", "KINDLER", "Kindle 10 shrines in all", [](const Game &g, bool) { return codex(g).shrines >= 10; }},
    {"shrines_50", "LAMPLIGHTER", "Kindle 50 shrines in all", [](const Game &g, bool) { return codex(g).shrines >= 50; }},
    {"shrines_run_5", "BEACON", "Kindle 5 shrines in one run", [](const Game &g, bool) { return run(g).night.lit >= 5; }},
    {"dawn_unlit", "UNLIT", "Reach dawn without kindling a shrine", [](const Game &g, bool dawn) { return dawn && run(g).night.lit == 0; }},
    {"dawn_swift", "SWIFT DAWN", "Reach dawn before 12:00 by kindling shrines", [](const Game &g, bool dawn) { return dawn && run(g).time < 720; }},
    {"dawn_ultra", "ULTRA-VIOLENT", "Reach dawn on ULTRA-VIOLENCE", [](const Game &g, bool dawn) { return dawn && g.difficulty >= Difficulty::UltraViolence; }},
    {"dawn_nightmare", "NIGHTMARE'S END", "Reach dawn on NIGHTMARE", [](const Game &g, bool dawn) { return dawn && g.difficulty == Difficulty::Nightmare; }},
    {"dawn_omen", "OMEN BREAKER", "Reach dawn under an omen", [](const Game &g, bool dawn) { return dawn && run(g).omen >= 0; }},
    {"evolve", "TRANSMUTATION", "Evolve a weapon", [](const Game &g, bool) { return evolved(g) >= 1; }},
    {"evolve_3", "ALCHEMY", "Evolve three weapons in one run", [](const Game &g, bool) { return evolved(g) >= 3; }},
    {"one_weapon", "ONE TRUE BLADE", "Reach dawn holding a single weapon", [](const Game &g, bool dawn) { return dawn && weapons(g) == 1; }},
    {"cyclops", "GIANT SLAYER", "Slay a cyclops", [](const Game &g, bool) { return codex(g).slain[int(EnemyKind::Cyclops)] > 0; }},
    {"alpha", "PACK BREAKER", "Slay an alpha wolf", [](const Game &g, bool) { return codex(g).slain[int(EnemyKind::Alpha)] > 0; }},
    {"necromancer", "LICHBANE", "Slay a necromancer", [](const Game &g, bool) { return codex(g).slain[int(EnemyKind::Necromancer)] > 0; }},
    {"dawn_mage", "THE MAGE'S DAWN", "Reach dawn as the Mage", dawnAs<0>},
    {"dawn_ranger", "THE RANGER'S DAWN", "Reach dawn as the Ranger", dawnAs<1>},
    {"dawn_knight", "THE KNIGHT'S DAWN", "Reach dawn as the Knight", dawnAs<2>},
    {"dawn_alchemist", "THE ALCHEMIST'S DAWN", "Reach dawn as the Alchemist", dawnAs<3>},
    {"dawn_stormcaller", "THE STORMCALLER'S DAWN", "Reach dawn as the Stormcaller", dawnAs<4>},
    {"dawn_reaper", "THE REAPER'S DAWN", "Reach dawn as the Reaper", dawnAs<5>},
    {"dawn_priestess", "THE PRIESTESS'S DAWN", "Reach dawn as the Priestess", dawnAs<6>},
    {"dawn_duelist", "THE DUELIST'S DAWN", "Reach dawn as the Duelist", dawnAs<7>},
    {"dawn_warden", "THE WARDEN'S DAWN", "Reach dawn as the Warden", dawnAs<8>},
    {"dawn_tinkerer", "THE TINKERER'S DAWN", "Reach dawn as the Tinkerer", dawnAs<9>},
    {"dawn_oracle", "THE ORACLE'S DAWN", "Reach dawn as the Oracle", dawnAs<10>},
    {"dawn_corsair", "THE CORSAIR'S DAWN", "Reach dawn as the Corsair", dawnAs<11>},
    {"dawn_all", "EVERY DAWN", "Reach dawn with all twelve classes", everyClass},
    {"long_road", "LONG ROAD", "Walk 5 kilometres in one run", [](const Game &g, bool) { return run(g).distanceRan / 16 >= 5000; }},
    {"untouchable", "UNTOUCHABLE", "Reach dawn at full health", [](const Game &g, bool dawn) {
       return dawn && run(g).player->hp >= run(g).player->maxHp;
     }},
    {"dawn_heat5", "BLAZING", "Reach dawn at heat 5", [](const Game &g, bool dawn) { return dawn && g.heat >= 5; }},
    {"dawn_heat10", "INFERNO", "Reach dawn at heat 10", [](const Game &g, bool dawn) { return dawn && g.heat >= 10; }},
    {"endless_5", "STILL STANDING", "Last 5 minutes into the endless night", [](const Game &g, bool) { return codex(g).bestEndless >= 300; }},
    {"endless_15", "NIGHT EVERLASTING", "Last 15 minutes into the endless night", [](const Game &g, bool) { return codex(g).bestEndless >= 900; }}};

const int FEAT_COUNT = int(sizeof FEATS / sizeof *FEATS);
static_assert(sizeof FEATS / sizeof *FEATS <= 64);

namespace {
bool everyClass(const Game &g, bool) {
  int first = 0;
  while (first < FEAT_COUNT && std::strcmp(FEATS[first].id, "dawn_mage") != 0)
    first++;
  for (int i = 0; i < HERO_COUNT; i++)
    if (first + i >= FEAT_COUNT || !(codex(g).feats >> (first + i) & 1))
      return false;
  return true;
}
} // namespace

std::vector<int> unlockFeats(Game &g, bool dawn) {
  std::vector<int> fresh;
  for (int i = 0; i < FEAT_COUNT; i++) {
    uint64_t bit = uint64_t(1) << i;
    if (!(g.codex.feats & bit) && FEATS[i].met(g, dawn))
      g.codex.feats |= bit, fresh.push_back(i);
  }
  return fresh;
}
