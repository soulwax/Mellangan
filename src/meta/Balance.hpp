#pragma once

#include <string>

enum class Difficulty {
  TooYoung,
  NotTooRough,
  HurtMePlenty,
  UltraViolence,
  Nightmare,
  Count
};

struct Balance {
  float spawnInterval = 0.9f, spawnRamp = 0.005f, spawnFloor = 0.06f;
  float batchEvery = 70, enemyCap = 550;
  float hordeEvery = 60, hordeBase = 24, hordeRamp = 12, hordeMax = 64;
  float bossEvery = 150, bossHpStep = 0.75f;
  float enemyHp = 1, enemyHpRamp = 150, enemyDamage = 1,
        enemyDamageRamp = 600, enemySpeed = 1;
  float eliteStart = 60, eliteBase = 0.03f, eliteRamp = 6000, eliteHp = 4;
  float localChance = 0.45f, strongChance = 0.55f;
  float playerHp = 100, damageTaken = 1, invuln = 0.6f;
  float xp = 1, xpBase = 6, xpGrowth = 1.08f;

  void load(const std::string &text, Difficulty tier);
};

const char *difficultyName(Difficulty d);
const char *difficultyTitle(Difficulty d);
const char *difficultyBlurb(Difficulty d);
Difficulty difficultyFrom(const std::string &name);
