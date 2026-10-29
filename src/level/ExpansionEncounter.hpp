#pragma once

#include <array>
#include <memory>

#include "entity/Entity.hpp"
#include "level/Structure.hpp"
#include "level/biome/Biome.hpp"
#include "level/BiomeReward.hpp"
#include "level/EncounterGuards.hpp"
#include "level/EncounterObjective.hpp"

class ExpansionChallenge : public Entity {
public:
  explicit ExpansionChallenge(const Structure &site, BiomeId biome);
  void onAdded() override;
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;
  bool complete() const;
  int defenders = 0;
  BiomeId biome;
  float elapsed = 0;
  EncounterObjective objective;
  bool rewarded = false, failed = false;
  std::array<entt::entity, 6> guardIds{};
  std::shared_ptr<ExpansionChallengeState> state;
  Structure site;

private:
  void finish(bool success);
};

bool expansionEncounter(Level &level, const Structure &site);
