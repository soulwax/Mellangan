#pragma once

#include "level/biome/Biome.hpp"

class Player;

enum class BiomeReward { Might, Vitality, Armor, Speed, Magnet, Recovery };

BiomeReward biomeReward(BiomeId biome);
const char *biomeRewardText(BiomeId biome);
float biomeRewardStrength(BiomeId biome);
void applyBiomeReward(Player &player, BiomeId biome);
