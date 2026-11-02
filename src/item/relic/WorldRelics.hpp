#pragma once

#include <unordered_set>

#include "entity/ItemStats.hpp"
#include "entity/Mob.hpp"
#include "item/Upgrade.hpp"

enum class Charm {
  ShrineLamp,
  Quill,
  MoonDial,
  StormGlass,
  SnowGlobe,
  SunShard,
  BogHeart,
  HollowCrown,
  GraveDirt,
  PilgrimStaff,
  Tidecaller,
  LanternOfAges,
  WandererBoots,
  StarChart,
  ThunderIdol,
  Dawnstone,
  EmberLocket,
  Hearthstone,
  ShrineKey,
  PackBanner,
  SeaCharm,
  MapOfOldRoads,
  BoneDice,
  LastLight,
  Count
};

class WorldRelic : public Upgrade {
public:
  static constexpr int RELICS = int(Charm::EmberLocket), ICONS = 160;
  explicit WorldRelic(Charm kind);
  void levelUp(Mob &p) override;
  void tick(Mob &p) override;
  void slay(Mob &p, Enemy &e) override;
  void forget(Mob &p) override;

  Charm kind;

private:
  void hold(Mob &p, float Mob::*stat, float want);
  void lend(Mob &p, float ItemStats::*stat, float want);
  float Mob::*held = nullptr;
  float ItemStats::*lent = nullptr;
  float applied = 0, loaned = 0, timer = 0, cd = 0;
  int seen = 0, crowns = 0, nights = 0, lit = 0;
  bool given = false, sealed = false;
  entt::entity pet = entt::null;
  std::unordered_set<int64_t> regions;
};

void addWorldRelics(std::vector<std::unique_ptr<Upgrade>> &u);
