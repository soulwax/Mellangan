#pragma once

#include <string>
#include <vector>

class Player;
class Upgrade;

enum class Tab { Arms, Arts, Instincts, Count };

struct Node {
  Upgrade *up;
  Tab tab;
  int tier;
};

struct Tree {
  static constexpr int TIERS = 4, RANKS = 5, SPLIT = 8;
  static constexpr int NEED[TIERS] = {1, 4, 9, 15};
  static constexpr float SKILL_COOL = 0.9f;

  std::vector<Node> nodes;

  static Tree of(const Player &p);
  static const char *tabName(Tab t);
  static void rank(Upgrade &skill, int rank);
  int tierSpent(const Player &p, Tab tab, int tier) const;
  std::string why(const Player &p, const Node &n, int points) const;
  bool spend(Player &p, const Node &n, int &points) const;
};
