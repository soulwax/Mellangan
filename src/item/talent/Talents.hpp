#pragma once

#include "item/Upgrade.hpp"

enum class TalentKind {
  Bloodthirst,
  GlassCannon,
  Momentum,
  ColdBlood,
  Pyromaniac,
  Scavenger,
  ThickSkin,
  Vengeance,
  NightOwl,
  HuntersInstinct,
  Overflow,
  Afterimage,
  Spellweave,
  VolleyMaster,
  Juggernaut,
  Catalyst,
  StaticBuild,
  SoulHoard,
  Sanctified,
  Flow,
  Rooted,
  SpareParts,
  Prophecy,
  Plunderer,
  SpellweaveThread,
  WideGrip,
  SwiftQuiver,
  KeenEdge,
  FleetHeart,
  EmberHeart,
  Collector,
  SummonersSeal,
  Count
};

class Talent : public Upgrade {
public:
  static constexpr int ICONS = 264, PASSIVE_ICONS = 152;
  explicit Talent(TalentKind kind);
  void levelUp(Mob &p) override;
  void tick(Mob &p) override;
  void slay(Mob &p, Enemy &e) override;

  TalentKind kind;

private:
  void hold(Mob &p, float Mob::*stat, float want);
  float run = 0, applied = 0, flowTime = 0;
  int souls = 0, stacks = 0, seen = 0;
};

void addTalents(std::vector<std::unique_ptr<Upgrade>> &u);
