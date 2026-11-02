#include "item/tree/Tree.hpp"

#include <spdlog/fmt/fmt.h>

#include <algorithm>
#include <cstring>

#include "entity/Player.hpp"
#include "hero/Hero.hpp"
#include "item/Passive.hpp"
#include "item/Upgrade.hpp"
#include "item/skill/Skill.hpp"

namespace {

bool mine(const Upgrade &u, const Hero &h) { return u.owner && std::strcmp(u.owner, h.name) == 0; }

bool open(const Upgrade &u, const Hero &h) { return !u.owner || mine(u, h); }

int place(const Upgrade &u, const Hero &h, Tab &tab) {
  if (u.weapon) {
    tab = Tab::Arms;
    if (std::strcmp(u.name, h.weapon) == 0)
      return 0;
    if (mine(u, h))
      return 1;
    return h.favours(u.name) ? 2 : 3;
  }
  if (u.skill) {
    tab = Tab::Arts;
    if (h.skill && std::strcmp(u.name, h.skill) == 0)
      return 0;
    return mine(u, h) ? 1 : 2;
  }
  tab = Tab::Instincts;
  if (mine(u, h))
    return 0;
  return h.favours(u.name) ? 1 : 2;
}

int owned(const Player &p, bool weapon) {
  int n = 0;
  for (const auto &u : p.upgrades)
    n += u->level > 0 && !u->rare && !u->skill && u->weapon == weapon;
  return n;
}

} // namespace

const char *Tree::tabName(Tab t) {
  constexpr const char *names[] = {"ARMS", "ARTS", "INSTINCTS"};
  return names[int(t)];
}

Tree Tree::of(const Player &p) {
  Tree tree;
  if (!p.hero)
    return tree;
  for (const auto &u : p.upgrades) {
    if (u->rare || u->hidden || !open(*u, *p.hero))
      continue;
    Tab tab;
    int tier = place(*u, *p.hero, tab);
    if (tab == Tab::Arms && tier == 3 && !p.hero->favours(u->name) && u->level == 0)
      continue;
    tree.nodes.push_back({u.get(), tab, tier});
  }
  std::stable_sort(tree.nodes.begin(), tree.nodes.end(), [](const Node &a, const Node &b) {
    if (a.tab != b.tab || a.tier != b.tier)
      return a.tab != b.tab ? a.tab < b.tab : a.tier < b.tier;
    return dynamic_cast<Passive *>(a.up) && !dynamic_cast<Passive *>(b.up);
  });
  for (int t = 0; t < int(Tab::Count); t++) {
    int wide = 0, seen = 0;
    for (const Node &n : tree.nodes)
      wide += int(n.tab) == t && n.tier == 2;
    if (wide > SPLIT)
      for (Node &n : tree.nodes)
        if (int(n.tab) == t && n.tier == 2 && seen++ >= wide / 2)
          n.tier = 3;
  }
  return tree;
}

void Tree::rank(Upgrade &skill, int rank) {
  auto *s = dynamic_cast<Skill *>(&skill);
  if (!s)
    return;
  while (skill.level < rank) {
    if (skill.level++ > 0)
      s->cooldown *= SKILL_COOL;
  }
}

int Tree::tierSpent(const Player &, Tab tab, int tier) const {
  int n = 0;
  for (const Node &o : nodes)
    n += o.tab == tab && o.tier == tier && o.up->level > 0;
  return n;
}

std::string Tree::why(const Player &p, const Node &n, int points) const {
  const Upgrade &u = *n.up;
  if (u.canEvolve(p))
    return points > 0 ? std::string() : "needs a skill point";
  if (u.skill ? u.level >= RANKS : u.maxed())
    return "fully learned";
  if (points <= 0)
    return "needs a skill point";
  if (p.lvl < NEED[n.tier])
    return fmt::format("needs hero level {}", NEED[n.tier]);
  bool above = false, any = false;
  for (const Node &o : nodes)
    if (o.tab == n.tab && o.tier == n.tier - 1)
      any = true, above |= o.up->level > 0;
  if (n.tier > 0 && any && !above && u.level == 0)
    return fmt::format("learn something from tier {} first", n.tier);
  if (!u.skill && u.level == 0 && owned(p, u.weapon) >= Upgrade::SLOTS)
    return u.weapon ? "all weapon slots are full" : "all passive slots are full";
  return {};
}

bool Tree::spend(Player &p, const Node &n, int &points) const {
  if (!why(p, n, points).empty())
    return false;
  Upgrade &u = *n.up;
  points--;
  if (u.canEvolve(p))
    return u.evolve(), true;
  if (u.skill) {
    bool fresh = u.level == 0;
    rank(u, u.level + 1);
    if (fresh || p.skill != &u)
      u.levelUp(p);
    return true;
  }
  u.levelUp(p);
  return true;
}
