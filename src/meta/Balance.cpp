#include "meta/Balance.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <sstream>

namespace {
struct Field {
  const char *key;
  float Balance::*member;
};

const Field FIELDS[] = {
    {"spawn_interval", &Balance::spawnInterval},
    {"spawn_ramp", &Balance::spawnRamp},
    {"spawn_floor", &Balance::spawnFloor},
    {"batch_every", &Balance::batchEvery},
    {"enemy_cap", &Balance::enemyCap},
    {"horde_every", &Balance::hordeEvery},
    {"horde_base", &Balance::hordeBase},
    {"horde_ramp", &Balance::hordeRamp},
    {"horde_max", &Balance::hordeMax},
    {"boss_every", &Balance::bossEvery},
    {"boss_hp_step", &Balance::bossHpStep},
    {"enemy_hp", &Balance::enemyHp},
    {"enemy_hp_ramp", &Balance::enemyHpRamp},
    {"enemy_damage", &Balance::enemyDamage},
    {"enemy_damage_ramp", &Balance::enemyDamageRamp},
    {"enemy_speed", &Balance::enemySpeed},
    {"elite_start", &Balance::eliteStart},
    {"elite_base", &Balance::eliteBase},
    {"elite_ramp", &Balance::eliteRamp},
    {"elite_hp", &Balance::eliteHp},
    {"local_chance", &Balance::localChance},
    {"strong_chance", &Balance::strongChance},
    {"player_hp", &Balance::playerHp},
    {"damage_taken", &Balance::damageTaken},
    {"invuln", &Balance::invuln},
    {"xp", &Balance::xp},
    {"xp_base", &Balance::xpBase},
    {"xp_growth", &Balance::xpGrowth},
};

const char *const NAMES[] = {"too_young", "not_too_rough", "hurt_me_plenty",
                             "ultra_violence", "nightmare"};
const char *const TITLES[] = {"I'M TOO YOUNG TO DIE", "HEY, NOT TOO ROUGH",
                              "HURT ME PLENTY", "ULTRA-VIOLENCE", "NIGHTMARE!"};
const char *const BLURBS[] = {"I play with dolls like a little girl",
                              "be gentle, senpai", "I like it rough",
                              "I like it rougher",
                              "Give it to me hard and fast, daddy"};
static_assert(std::size(NAMES) == int(Difficulty::Count));

std::string trim(const std::string &s) {
  size_t a = s.find_first_not_of(" \t\r"), b = s.find_last_not_of(" \t\r");
  return a == std::string::npos ? "" : s.substr(a, b - a + 1);
}
} // namespace

const char *difficultyName(Difficulty d) {
  return NAMES[std::clamp(int(d), 0, int(Difficulty::Count) - 1)];
}

const char *difficultyTitle(Difficulty d) {
  return TITLES[std::clamp(int(d), 0, int(Difficulty::Count) - 1)];
}

const char *difficultyBlurb(Difficulty d) {
  return BLURBS[std::clamp(int(d), 0, int(Difficulty::Count) - 1)];
}

Difficulty difficultyFrom(const std::string &name) {
  for (int i = 0; i < int(Difficulty::Count); i++)
    if (trim(name) == NAMES[i])
      return Difficulty(i);
  return Difficulty::HurtMePlenty;
}

void Balance::load(const std::string &text, Difficulty tier) {
  std::istringstream in(text);
  std::string line, section = "all";
  while (std::getline(in, line)) {
    line = trim(line.substr(0, line.find_first_of(";#")));
    if (line.empty())
      continue;
    if (line.front() == '[') {
      section = trim(line.substr(1, line.find(']') - 1));
      continue;
    }
    if (section != "all" && section != difficultyName(tier))
      continue;
    size_t eq = line.find('=');
    if (eq == std::string::npos)
      continue;
    std::string key = trim(line.substr(0, eq)),
                value = trim(line.substr(eq + 1));
    bool known = false;
    for (const Field &f : FIELDS)
      if (key == f.key)
        this->*f.member = std::strtof(value.c_str(), nullptr), known = true;
    if (!known)
      spdlog::warn("balance: unknown key {}", key);
  }
}
