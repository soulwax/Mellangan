#include "meta/RunLog.hpp"

#include <SDL3/SDL.h>
#include <spdlog/fmt/fmt.h>

#include "Game.hpp"
#include "Version.hpp"
#include "entity/Player.hpp"
#include "hero/Hero.hpp"
#include "item/Upgrade.hpp"
#include "level/Chart.hpp"
#include "level/Flavour.hpp"
#include "level/Level.hpp"

namespace {
std::string clean(std::string s) {
  for (char &c : s)
    if (c == ',' || c == '|' || c == ':' || c == '\n')
      c = ' ';
  return s;
}

std::string stamp() {
  SDL_Time now = 0;
  SDL_DateTime t{};
  if (!SDL_GetCurrentTime(&now) || !SDL_TimeToDateTime(now, &t, true))
    return "";
  return fmt::format("{:04}-{:02}-{:02}T{:02}:{:02}", t.year, t.month, t.day, t.hour, t.minute);
}
} // namespace

std::string runRow(const Game &game, bool dawn) {
  const Level &l = *game.level;
  const Player &p = *l.player;
  std::string weapons, picks, levels, biomes;
  for (auto &up : p.upgrades)
    if (up->weapon && up->level > 0)
      weapons += fmt::format("{}{}:{}:{:.0f}", weapons.empty() ? "" : "|", clean(up->name),
                             up->level, up->dealt);
  for (const char *name : l.trace.picks)
    picks += (picks.empty() ? "" : "|") + clean(name);
  for (int lvl : l.trace.levels)
    levels += fmt::format("{}{}", levels.empty() ? "" : "|", lvl);
  for (int b = 0; b < BIOME_COUNT; b++)
    if (l.trace.biomes[b] > 0)
      biomes += fmt::format("{}{}:{:.0f}", biomes.empty() ? "" : "|",
                            clean(Chart::title(BiomeId(b))), l.trace.biomes[b]);
  return fmt::format("{},{},{},{},{},{},{},{:.0f},{},{},{},{},{},{:.0f},{:.0f},{},{},{},{},{}\n",
                     stamp(), MELLANGAN_VERSION, clean(game.worldName()),
                     game.campaign ? "campaign" : "run", difficultyName(game.difficulty),
                     p.hero ? p.hero->name : "", l.omen >= 0 ? clean(OMENS[l.omen].name) : "",
                     l.time, int(dawn), dawn || !p.felledBy ? "" : clean(p.felledBy), p.lvl,
                     l.kills, l.night.lit, l.distanceRan / 16, l.totalDamage, weapons, picks,
                     levels, biomes, game.campaign ? 0 : game.heat);
}
