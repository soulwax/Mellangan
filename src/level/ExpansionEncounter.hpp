#pragma once

#include <array>
#include <memory>

#include "entity/Entity.hpp"
#include "level/Structure.hpp"

struct ExpansionChallengeState {
  std::array<bool, 6> defeated{};
  bool escaped = false;
};

class ExpansionChallenge : public Entity {
public:
  explicit ExpansionChallenge(const Structure &site, int biome);
  void onAdded() override;
  void tick() override;
  void render(Screen &s) override;
  void renderLight(Screen &s) override;
  bool complete() const;
  int kind, biome, defenders = 0;
  float elapsed = 0, progress = 0, integrity = 100;
  double heading, distance = 0;
  bool rewarded = false, failed = false;
  std::array<entt::entity, 6> guardIds{};
  std::shared_ptr<ExpansionChallengeState> state;
  Structure site;

private:
  void finish(bool success);
};

bool expansionEncounter(Level &level, const Structure &site);
