#pragma once

#include <entt/entity/entity.hpp>

#include <algorithm>
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
#include "campaign/Delve.hpp"
#include "campaign/Quests.hpp"

class Level;

struct Campaign {
  struct Town {
    double x, y;
  };

  static constexpr int VERSION = 1, STASH = 32;
  static constexpr float SAFE = 150, AUTOSAVE = 60, NIGHT = 180, DAY = 240, DAY_RATE = 0.35f,
                         SHADE_GAP = 18;
  static constexpr int MAX_SHADES = 3;

  std::string world;
  int hero = 0, lvl = 1, gold = 0, day = 1, points = 0, act = 1;
  int quest = 0, progress = 0, litSeen = 0, slainSeen = 0;
  QuestStage stage = QuestStage::Offered;
  double qx = 0, qy = 0;
  bool qsite = false;
  entt::entity foe = entt::null, elder = entt::null;
  std::vector<entt::entity> nests;
  entt::entity pilgrim = entt::null;
  double journey = 0;
  bool holding = false;
  float holdTick = 0, waveTime = 0;
  std::string active;
  float exp = 0, expNext = 5;
  int materials[MATS]{};
  std::vector<Item> bag, stash;
  std::array<Item, GEAR> gear{};
  std::array<bool, GEAR> worn{};
  std::vector<std::pair<std::string, int>> upgrades;
  double x = 0, y = 0, xTown = 0, yTown = 0;
  bool founded = false, hardcore = false;
  int nightmare = 0;
  std::vector<Town> outposts;
  std::vector<std::pair<int, int>> delves;
  Delve delve;
  Edits edits;
  std::vector<entt::entity> waypoints, chests;
  std::unordered_map<uint64_t, int> chops;
  int building = -1;
  bool wasDay = true;
  float smoke = 0, shadeTime = 8;

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
  std::vector<Town> towns() const;
  Town nearest(double x, double y) const;
  Town home() const {
    std::vector<Town> all = towns();
    return all[size_t(std::clamp(act, 1, int(all.size())) - 1)];
  }
  bool delved(int rx, int ry) const {
    for (const auto &[x, y] : delves)
      if (x == rx && y == ry)
        return true;
    return false;
  }
  void awaken(Level &level);
  int spent() const;
  int forgetCost() const;
  void settle(Level &level, double x, double y);
  void travel(Level &level, int town);
  void leave();

  void staff(Level &level);

private:
  void found(Level &level);
  void restore(Level &level);
  void raise(Level &level);
  void post(Level &level, const Town &t);
  void smoulder(Level &level);
  void lure(Level &level);
};
