#pragma once

#include <array>
#include <memory>

#include <entt/entt.hpp>

#include "level/Structure.hpp"
#include "level/biome/Biome.hpp"

class Level;

struct ExpansionChallengeState {
  std::array<bool, 6> defeated{};
  bool escaped = false;
};

bool encounterOpenPosition(Level &level, double &x, double &y);
int spawnEncounterGuards(Level &level, BiomeId biome, const Structure &site,
                         double x, double y, double heading,
                         std::shared_ptr<ExpansionChallengeState> state,
                         std::array<entt::entity, 6> &ids);
bool encounterGuardsDefeated(const ExpansionChallengeState &state, int count);
bool encounterGuardsPresent(Level &level, const ExpansionChallengeState &state,
                            const std::array<entt::entity, 6> &ids, int count);
