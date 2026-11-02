#pragma once

#include <entt/entity/entity.hpp>

#include <array>
#include <unordered_map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "item/craft/Recipes.hpp"
#include "item/loot/Item.hpp"
#include "level/Edits.hpp"
#include "level/RunRules.hpp"
#include "campaign/Quests.hpp"

class Level;

struct Campaign {
  static constexpr int VERSION = 1, STASH = 32;
  static constexpr float SAFE = 150, AUTOSAVE = 60, NIGHT = 180, DAY = 240, DAY_RATE = 0.35f;

  std::string world;
  int hero = 0, lvl = 1, gold = 0, day = 1, points = 0, act = 1;
  int quest = 0, progress = 0, litSeen = 0, slainSeen = 0;
  QuestStage stage = QuestStage::Offered;
  double qx = 0, qy = 0;
  bool qsite = false;
  entt::entity foe = entt::null, elder = entt::null;
  std::string active;
  float exp = 0, expNext = 5;
  int materials[MATS]{};
  std::vector<Item> bag, stash;
  std::array<Item, GEAR> gear{};
  std::array<bool, GEAR> worn{};
  std::vector<std::pair<std::string, int>> upgrades;
  double x = 0, y = 0, xTown = 0, yTown = 0;
  bool founded = false;
  Edits edits;
  entt::entity waypoint = entt::null, chest = entt::null;
  std::unordered_map<uint64_t, int> chops;
  int building = -1;
  bool wasDay = true;
  float smoke = 0;

  std::string save() const;
  static bool load(std::string_view text, Campaign &out);
  RunRules rules() const;
  void capture(const Level &level);
  void enter(Level &level);
  void fall(Level &level);
  void tick(Level &level);
  void regrow(int day);
  void barricade();
  bool founding(int tx, int ty) const;
  void leave();

private:
  void found(Level &level);
  void restore(Level &level);
  void raise(Level &level);
  void smoulder(Level &level);
};
