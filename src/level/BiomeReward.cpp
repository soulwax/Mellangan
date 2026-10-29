#include "level/BiomeReward.hpp"

#include "entity/Player.hpp"

BiomeReward biomeReward(BiomeId biome) {
  constexpr BiomeReward rewards[] = {
      BiomeReward::Vitality, BiomeReward::Recovery, BiomeReward::Magnet,
      BiomeReward::Speed, BiomeReward::Might, BiomeReward::Armor,
      BiomeReward::Recovery, BiomeReward::Vitality, BiomeReward::Speed,
      BiomeReward::Might, BiomeReward::Speed, BiomeReward::Might,
      BiomeReward::Armor, BiomeReward::Recovery, BiomeReward::Might,
      BiomeReward::Armor, BiomeReward::Armor, BiomeReward::Speed,
      BiomeReward::Recovery, BiomeReward::Magnet, BiomeReward::Might,
      BiomeReward::Vitality, BiomeReward::Speed, BiomeReward::Armor,
      BiomeReward::Recovery, BiomeReward::Magnet};
  static_assert(std::size(rewards) == BIOME_COUNT);
  return rewards[std::clamp(int(biome), 0, int(BIOME_COUNT) - 1)];
}

float biomeRewardStrength(BiomeId biome) { return biome >= BIOME_SALT ? 2.0f : 1.0f; }

const char *biomeRewardText(BiomeId biome) {
  bool exotic = biome >= BIOME_SALT;
  switch (biomeReward(biome)) {
  case BiomeReward::Might: return exotic ? "+6% MIGHT" : "+3% MIGHT";
  case BiomeReward::Vitality: return exotic ? "+10 MAX HP" : "+5 MAX HP";
  case BiomeReward::Armor: return exotic ? "+0.5 ARMOUR" : "+0.25 ARMOUR";
  case BiomeReward::Speed: return exotic ? "+4 SPEED" : "+2 SPEED";
  case BiomeReward::Magnet: return exotic ? "+8 PICKUP RANGE" : "+4 PICKUP RANGE";
  case BiomeReward::Recovery: return exotic ? "+0.2 REGEN/S" : "+0.1 REGEN/S";
  }
  return "";
}

void applyBiomeReward(Player &p, BiomeId biome) {
  float strength = biomeRewardStrength(biome);
  switch (biomeReward(biome)) {
  case BiomeReward::Might: p.might += 0.03f * strength; break;
  case BiomeReward::Vitality: p.maxHp += 5 * strength; p.heal(5 * strength); break;
  case BiomeReward::Armor: p.armor += 0.25f * strength; break;
  case BiomeReward::Speed: p.speed += 2 * strength; break;
  case BiomeReward::Magnet: p.magnet += 4 * strength; break;
  case BiomeReward::Recovery: p.regen += 0.1f * strength; break;
  }
}
