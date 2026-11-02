#undef NDEBUG
#include <cassert>

#include <iostream>

#include "test/Tests.hpp"

#include "Game.hpp"
#include "campaign/Campaign.hpp"
#include "entity/Player.hpp"
#include "hero/Hero.hpp"
#include "item/Upgrade.hpp"
#include "item/skill/Skill.hpp"
#include "item/tree/Tree.hpp"
#include "level/Level.hpp"

namespace {

const Node *first(const Tree &tree, Tab tab, int tier) {
  for (const Node &n : tree.nodes)
    if (n.tab == tab && n.tier == tier)
      return &n;
  return nullptr;
}

} // namespace

void testSkillTrees() {
  for (int h = 0; h < HERO_COUNT; h++) {
    Game game;
    game.dev = true, game.start.hero = h;
    game.level = std::make_unique<Level>(game);
    game.level->tick();
    Player &p = *game.level->player;
    Tree tree = Tree::of(p);
    for (int t = 0; t < int(Tab::Count); t++) {
      assert(first(tree, Tab(t), 0));
      int deepest = 0;
      for (const Node &n : tree.nodes)
        if (int(n.tab) == t)
          deepest = std::max(deepest, n.tier);
      for (int k = 1; k <= deepest; k++)
        assert(first(tree, Tab(t), k));
    }
    for (const Node &n : tree.nodes) {
      assert(n.up && n.tier >= 0 && n.tier < Tree::TIERS && !n.up->rare && !n.up->hidden);
      assert(!n.up->owner || std::strcmp(n.up->owner, p.hero->name) == 0);
    }

    int points = 0;
    const Node *deep = nullptr;
    for (const Node &n : tree.nodes)
      if (n.tab == Tab::Instincts && n.tier == Tree::TIERS - 1)
        deep = &n;
    assert(deep && tree.why(p, *deep, 1).find("hero level") != std::string::npos);
    p.lvl = 40;
    assert(tree.why(p, *deep, 1).find("tier") != std::string::npos);
    assert(tree.why(p, *deep, 0) == "needs a skill point");

    points = 40;
    for (int t = 0; t < int(Tab::Count); t++) {
      int deepest = 0;
      for (const Node &n : tree.nodes)
        if (int(n.tab) == t)
          deepest = std::max(deepest, n.tier);
      for (int k = 0; k <= deepest; k++) {
        const Node *n = nullptr;
        for (const Node &o : tree.nodes)
          if (int(o.tab) == t && o.tier == k && (k > 0 || o.up->level == 0 || o.up->skill))
            n = n ? n : &o;
        if (!n)
          continue;
        int before = n->up->level, left = points;
        assert(tree.spend(p, *n, points) && points == left - 1 && n->up->level == before + 1);
      }
    }

    const Node *art = first(tree, Tab::Arts, 1);
    auto *skill = dynamic_cast<Skill *>(art->up);
    assert(skill && skill->level == 1 && p.skill != skill);
    float cool = skill->cooldown;
    while (skill->level < Tree::RANKS)
      assert(tree.spend(p, *art, points));
    assert(std::abs(skill->cooldown - cool * std::pow(Tree::SKILL_COOL, 4)) < 1e-3f);
    assert(p.skill == skill);
    assert(tree.why(p, *art, points) == "fully learned" && !tree.spend(p, *art, points));
    game.level.reset();
  }

  Game game;
  game.dev = true, game.start.hero = 2;
  Campaign c;
  c.world = "TREETEST";
  game.level = std::make_unique<Level>(game, c.rules());
  c.enter(*game.level);
  game.level->tick();
  Player &p = *game.level->player;
  assert(p.skill && p.skill->level == 1);
  Tree tree = Tree::of(p);
  p.lvl = 6, c.points = 3;
  const Node *art = first(tree, Tab::Arts, 1);
  assert(tree.spend(p, *art, c.points) && tree.spend(p, *art, c.points));
  float cool = dynamic_cast<Skill *>(art->up)->cooldown;
  c.capture(*game.level);
  std::string text = c.save();
  c.leave();
  Campaign back;
  assert(Campaign::load(text, back) && back.points == 1 && back.active == art->up->name);
  Game again;
  again.dev = true, again.start.hero = 2;
  again.level = std::make_unique<Level>(again, back.rules());
  back.enter(*again.level);
  Player &q = *again.level->player;
  Upgrade *same = q.upgrade(art->up->name);
  assert(q.skill == same && same->level == 2 &&
         std::abs(dynamic_cast<Skill *>(same)->cooldown - cool) < 1e-4f);
  back.leave();
  std::cout << "[TEST] Skill trees: all twelve classes have three tabs with no gaps between "
               "tiers and nothing foreign; tiers wait for hero level and the tier above; a path "
               "through every tab spends cleanly; skills rank to five with shorter cooldowns; "
               "points, ranks and the active skill survive a campaign save: PASS"
            << std::endl;
}
