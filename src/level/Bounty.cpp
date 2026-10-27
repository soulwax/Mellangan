#include "level/Bounty.hpp"

#include <cctype>

#include "Game.hpp"
#include "level/Chart.hpp"
#include "level/Level.hpp"

namespace {

enum { SLAY, KINDLE, ENTER, WALK, SURVIVE, STREAK, KINDS };

const char *const FOES[] = {"bats",      "rats",    "slimes",
                            "scorpions", "spiders", "ghosts"};
const BiomeId LANDS[] = {BIOME_MEADOW, BIOME_FOREST, BIOME_AUTUMN,
                         BIOME_HEATH,  BIOME_SNOW,   BIOME_MIRE};
const StructureKind SITES[] = {StructureKind::Cottage, StructureKind::Graveyard,
                               StructureKind::Camp, StructureKind::Ruin,
                               StructureKind::Castle};

std::string lower(const char *s) {
  std::string out(s);
  for (char &c : out)
    c = char(std::tolower(uint8_t(c)));
  return out;
}

} // namespace

void Bounty::roll(Bounty *out, int n) {
  uint32_t used = 0;
  for (int i = 0; i < n; i++) {
    uint32_t h = hash2(i * 7919 + 13, 4242);
    int kind = int(h % KINDS);
    while (used >> kind & 1)
      kind = (kind + 1) % KINDS;
    used |= 1u << kind;
    uint32_t r = h >> 8;
    Bounty &b = out[i];
    b = {kind, 0, 1, false};
    switch (kind) {
    case SLAY:
      b.arg = int(r % 6), b.goal = 25 + int(r >> 4) % 5 * 10;
      break;
    case KINDLE:
      b.arg = LANDS[r % 6];
      break;
    case ENTER:
      b.arg = int(SITES[r % 5]);
      break;
    case WALK:
      b.goal = 4 + int(r % 3);
      break;
    case SURVIVE:
      b.goal = 300 + int(r % 5) * 60;
      break;
    case STREAK:
      b.goal = 30 + int(r % 4) * 10;
      break;
    }
  }
}

std::string Bounty::text() const {
  switch (kind) {
  case SLAY:
    return fmt::format("Slay {} {}", goal, FOES[arg]);
  case KINDLE:
    return fmt::format("Kindle a shrine in {}", lower(Chart::title(BiomeId(arg))));
  case ENTER:
    return fmt::format("Find a {}", lower(STRUCTURES[arg].name));
  case WALK:
    return fmt::format("Walk {} regions", goal);
  case SURVIVE:
    return fmt::format("Survive until {}", clockText(float(goal)));
  default:
    return fmt::format("Reach a {} kill streak", goal);
  }
}

int Bounty::progress(const Level &l) const {
  switch (kind) {
  case SLAY:
    return l.slain[arg];
  case KINDLE:
    return int(l.litBiomes >> arg & 1);
  case ENTER:
    return int(l.entered >> arg & 1);
  case WALK:
    return l.chart.discovered;
  case SURVIVE:
    return int(l.time);
  default:
    return l.bestStreak;
  }
}
