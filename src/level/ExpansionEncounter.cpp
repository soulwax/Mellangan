#include "level/ExpansionEncounter.hpp"

#include "Game.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "entity/pickup/Chest.hpp"
#include "entity/pickup/Potion.hpp"
#include "entity/pickup/Gem.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

namespace {
struct ChallengeStyle {
  const char *name, *task;
  Color color;
};
const ChallengeStyle STYLES[] = {
    {"AMBUSH CACHE", "DEFEAT THE GUARDS", col::gold},
    {"GUARDED ALTAR", "HOLD THE ALTAR: 12S", {0.7f, 0.6f, 1, 1}},
    {"ROAMING CARAVAN", "ESCORT AND PROTECT", {0.4f, 0.8f, 1, 1}},
    {"CORRUPTED SPRING", "SURVIVE THE VENOM: 10S", {0.5f, 1, 0.45f, 1}}};

}

ExpansionChallenge::ExpansionChallenge(const Structure &site, BiomeId biome)
    : biome(biome), objective(site.seed),
      state(std::make_shared<ExpansionChallengeState>()), site(site) {
  x = site.centerX();
  y = site.centerY();
  guardIds.fill(entt::null);
}

void ExpansionChallenge::onAdded() {
  if (!encounterOpenPosition(*level, x, y))
    return finish(false);
  defenders = spawnEncounterGuards(*level, biome, site, x, y, objective.heading, state, guardIds);
  if (!defenders)
    return finish(false);
  level->game.announce(std::string(STYLES[objective.kind].name) + ": " + STYLES[objective.kind].task);
  level->game.announce(std::string("REWARD: ") + biomeRewardText(biome));
}

bool ExpansionChallenge::complete() const {
  if (failed || !encounterGuardsDefeated(*state, defenders))
    return false;
  return objective.complete();
}

void ExpansionChallenge::finish(bool success) {
  if (removed)
    return;
  rewarded = success;
  failed = !success;
  if (success) {
    Player &p = *level->player;
    float strength = biomeRewardStrength(biome);
    applyBiomeReward(p, biome);
    level->add<Chest>(x, y);
    Gem::drop(*level, x - 12, y, 10 * strength);
    if (objective.kind == 2 || objective.kind == 3)
      level->add<Potion>(x + 12, y);
    if (objective.kind == 3)
      p.status.clear();
    Spark::burst(*level, x, y, STYLES[objective.kind].color, 20, 100, true);
    level->game.announce(std::string("REWARD: ") + biomeRewardText(biome));
  } else {
    level->game.announce(std::string(STYLES[objective.kind].name) + " LOST");
  }
  remove();
}

void ExpansionChallenge::tick() {
  if (removed)
    return;
  Player &p = *level->player;
  elapsed += DT;
  double px = p.x - x, py = p.y - y;
  double d2 = px * px + py * py;
  if (p.dead() || state->escaped || elapsed > 45 || d2 > 300 * 300)
    return finish(false);
  if (!encounterGuardsPresent(*level, *state, guardIds, defenders))
    return finish(false);
  if (!objective.tick(*level, p, x, y, d2))
    return finish(false);
  if (complete())
    finish(true);
}

void ExpansionChallenge::render(Screen &s) {
  if (biome >= BIOME_SALT)
    s.center(atlas::biomeProp(30 + biome - BIOME_SALT), xp, yp - 6);
  else
    s.center(atlas::tile(atlas::t::Chest), xp, yp - 6);
  s.color(STYLES[objective.kind].color.alpha(0.35f));
  s.center(atlas::Ring, xp, yp, objective.kind == 2 ? 1.2f : 2.0f, elapsed * 0.2f);
  s.color(col::white);
  int defeated = 0;
  for (int i = 0; i < defenders; i++)
    defeated += state->defeated[i];
  float amount = objective.fraction(defeated, defenders);
  s.rect(xp - 16, yp + 4, 32, 3, col::panel);
  s.rect(xp - 16, yp + 4, 32 * std::min(amount, 1.0f), 3, STYLES[objective.kind].color);
  if (objective.kind == 2) {
    s.rect(xp - 16, yp + 8, 32, 2, col::panel);
    s.rect(xp - 16, yp + 8, 32 * std::max(objective.integrity, 0.0f) / 100, 2, col::heal);
  }
}

void ExpansionChallenge::renderLight(Screen &s) {
  s.glow(xp, yp - 5, 100, STYLES[objective.kind].color * 0.12f);
}

bool expansionEncounter(Level &level, const Structure &site) {
  if (site.kind == StructureKind::Shrine)
    return false;
  int biome = int(world::biomeAt(site.x + site.w / 2, site.y + site.h / 2));
  if (biome < BIOME_SALT && site.seed % 3 != 0)
    return false;
  level.add<ExpansionChallenge>(site, BiomeId(biome));
  return true;
}
