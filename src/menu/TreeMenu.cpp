#include "menu/TreeMenu.hpp"

#include <spdlog/fmt/fmt.h>

#include <algorithm>
#include <cmath>

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "campaign/Campaign.hpp"
#include "entity/Player.hpp"
#include "hero/Hero.hpp"
#include "item/Upgrade.hpp"
#include "item/skill/Skill.hpp"
#include "level/Level.hpp"

namespace {

constexpr float CELL = 15, GAP = 2, LABEL = 12, PAD = 8;
constexpr float COLUMN = TreeMenu::WRAP * (CELL + GAP) + 2 * PAD, SPACE = 10;
constexpr Color ACTIVE{0.45f, 0.85f, 1, 1}, WORSE{1, 0.4f, 0.35f, 1};

int lines(int n) { return (n + TreeMenu::WRAP - 1) / TreeMenu::WRAP; }

} // namespace

TreeMenu::TreeMenu(Game &game) : Menu(game), tree(Tree::of(*game.level->player)) {}

Player &TreeMenu::player() const { return *game.level->player; }

int &TreeMenu::points() const { return game.campaign->points; }

float TreeMenu::columnX(int tab) const {
  const Screen &s = game.screen;
  float total = 3 * COLUMN + 2 * SPACE;
  return (s.w - total * s.u) / 2 + tab * (COLUMN + SPACE) * s.u;
}

float TreeMenu::tierY(int tab, int tier) const {
  const Screen &s = game.screen;
  float y = s.h * 0.16f;
  for (int row = 0; row < tier; row++) {
    int n = 0;
    for (const Node &o : tree.nodes)
      n += int(o.tab) == tab && o.tier == row;
    if (n)
      y += (LABEL + lines(n) * (CELL + GAP) + 4) * s.u;
  }
  return y;
}

SDL_FRect TreeMenu::cell(int i) const {
  const Node &n = tree.nodes[size_t(i)];
  int k = 0;
  for (int j = 0; j < i; j++)
    k += tree.nodes[size_t(j)].tab == n.tab && tree.nodes[size_t(j)].tier == n.tier;
  float u = game.screen.u;
  float x = columnX(int(n.tab)) + (PAD + (k % WRAP) * (CELL + GAP)) * u;
  float y = tierY(int(n.tab), n.tier) + (LABEL + (k / WRAP) * (CELL + GAP)) * u;
  return {x, y, CELL * u, CELL * u};
}

void TreeMenu::move(int dx, int dy) {
  SDL_FRect a = cell(selected);
  float best = 1e9f;
  int pick = selected;
  for (int i = 0; i < int(tree.nodes.size()); i++) {
    SDL_FRect b = cell(i);
    float ox = b.x - a.x, oy = b.y - a.y;
    float along = ox * dx + oy * dy, across = std::abs(ox * dy) + std::abs(oy * dx);
    if (i == selected || along <= 0)
      continue;
    float d = along + across * 2.5f;
    if (d < best)
      best = d, pick = i;
  }
  selected = pick;
}

void TreeMenu::activate() {
  Player &p = player();
  const Node &n = tree.nodes[size_t(selected)];
  Upgrade &u = *n.up;
  if (u.skill && u.level > 0 && p.skill != &u &&
      (points() == 0 || u.level >= Tree::RANKS)) {
    u.levelUp(p);
    told = fmt::format("{} is now your skill", u.name), toldTime = 3;
    return Sound::select.play(0.5f, 0, 1.2f);
  }
  std::string why = tree.why(p, n, points());
  if (!why.empty()) {
    told = why, toldTime = 3;
    return Sound::hurt.play(0.3f, 0);
  }
  bool evolving = u.canEvolve(p);
  tree.spend(p, n, points());
  told = evolving ? fmt::format("{} evolved", u.name)
                  : fmt::format("{} rank {}", u.name, u.level);
  toldTime = 3;
  spdlog::info("tree: {} -> {} ({} points left)", u.name, u.level, points());
  Sound::levelUp.play(0.5f, 0);
  game.saveCampaign();
}

void TreeMenu::tick() {
  Menu::tick();
  toldTime = std::max(0.0f, toldTime - DT);
  Input &in = game.input;
  if (in.campaign.clicked || in.escape.clicked || in.inventory.clicked) {
    Sound::select.play(0.5f, 0);
    return game.setMenu(nullptr);
  }
  if (tree.nodes.empty())
    return;
  if (in.left.clicked || in.right.clicked || in.up.clicked || in.down.clicked) {
    move(in.right.clicked - in.left.clicked, in.down.clicked - in.up.clicked);
    Sound::select.play(0.25f, 0);
  }
  SDL_FPoint m{in.mouseX, in.mouseY};
  for (int i = 0; i < int(tree.nodes.size()) && (in.mouseMoved || in.mouseClicked); i++) {
    SDL_FRect r = cell(i);
    if (SDL_PointInRectFloat(&m, &r)) {
      selected = i;
      if (in.mouseClicked)
        activate();
    }
  }
  if (in.confirm.clicked)
    activate();
}

void TreeMenu::render(Screen &s) {
  Player &p = player();
  float u = s.u, small = std::max(1.0f, u - 1), in = std::min(1.0f, t * 4);
  s.rect(0, 0, s.w, s.h, {0.02f, 0.02f, 0.05f, 0.82f * in});
  float x0 = columnX(0) - 10 * u, x1 = columnX(2) + (COLUMN + 10) * u;
  float y0 = s.h * 0.16f - 52 * u, y1 = s.h * 0.84f;
  s.rect(x0, y0, x1 - x0, y1 - y0, {0.04f, 0.04f, 0.08f, 0.94f * in});
  s.bracket(x0, y0, x1 - x0, y1 - y0, u);
  s.text(fmt::format("{}  SKILL TREE", p.hero ? p.hero->name : ""), x0 + 12 * u, y0 + 8 * u, u,
         col::gold.alpha(in));
  std::string left = fmt::format("{} point{} to spend", points(), points() == 1 ? "" : "s");
  s.text(left, x1 - Screen::textWidth(left, small) - 12 * u, y0 + 12 * u, small,
         (points() ? col::gold : col::muted).alpha(in));

  for (int tab = 0; tab < int(Tab::Count); tab++) {
    s.text(Tree::tabName(Tab(tab)), columnX(tab) + PAD * u, s.h * 0.16f - 18 * u, small,
           col::white.alpha(in));
    for (int tier = 0; tier < Tree::TIERS; tier++) {
      bool any = false;
      for (const Node &n : tree.nodes)
        any |= int(n.tab) == tab && n.tier == tier;
      if (!any)
        continue;
      Color c = p.lvl >= Tree::NEED[tier] ? col::muted : WORSE * 0.8f;
      s.text(fmt::format("TIER {}  level {}", tier + 1, Tree::NEED[tier]),
             columnX(tab) + PAD * u, tierY(tab, tier), small, c.alpha(in));
    }
  }

  for (int i = 0; i < int(tree.nodes.size()); i++) {
    const Node &n = tree.nodes[size_t(i)];
    const Upgrade &up = *n.up;
    SDL_FRect r = cell(i);
    bool ready = tree.why(p, n, std::max(1, points())).empty();
    Color edge = p.skill == &up ? ACTIVE
                 : up.level > 0  ? col::gold
                 : ready         ? col::white * 0.7f
                                 : col::muted * 0.5f;
    s.rect(r.x, r.y, r.w, r.h, col::panel.alpha(0.9f * in));
    s.frame(r.x, r.y, r.w, r.h, i == selected ? 2 * u : u,
            (i == selected ? col::gold : edge).alpha(in));
    s.color(col::white.alpha((up.level > 0 ? 1 : ready ? 0.65f : 0.25f) * in));
    s.sprite(up.icon, r.x + 1.5f * u, r.y + 1.5f * u, 0, 0, u * 0.75f, u * 0.75f);
    s.color(col::white);
    if (up.level > 0)
      s.text(std::to_string(up.level), r.x + r.w - 5 * small, r.y + r.h - 9 * small, small,
             col::gold.alpha(in));
  }

  if (tree.nodes.empty())
    return;
  const Node &n = tree.nodes[size_t(selected)];
  const Upgrade &up = *n.up;
  float dy = y1 - 64 * u, dx = x0 + 12 * u;
  int ranks = up.skill ? Tree::RANKS : up.rare ? 1 : Upgrade::MAX_LEVEL;
  s.text(fmt::format("{}   rank {}/{}", up.name, up.level, ranks), dx, dy, u,
         (up.level > 0 ? col::gold : col::white).alpha(in));
  const char *desc = up.desc[size_t(std::clamp(up.level, 0, Upgrade::MAX_LEVEL - 1))];
  s.text(Screen::fit(desc ? desc : "", x1 - dx - 12 * u, small), dx, dy + 20 * u, small,
         col::white.alpha(0.85f * in));
  std::string why = tree.why(p, n, points());
  std::string state = up.skill && up.level > 0 && p.skill != &up
                          ? "Enter: make it your skill, or rank it up with a point"
                      : why.empty() ? (up.canEvolve(p) ? "Enter: evolve" : "Enter: learn")
                                    : why;
  s.text(state, dx, dy + 36 * u, small,
         (why.empty() ? col::gold : col::muted).alpha(in));
  if (toldTime > 0 && told != state) {
    std::string_view shown = told;
    s.text(shown, x1 - Screen::textWidth(shown, small) - 12 * u, dy + 36 * u, small,
           col::gold.alpha(std::min(1.0f, toldTime) * in));
  }
  s.textCenter("Arrows or mouse pick   Enter learn   K close", s.w / 2, y1 + 8 * u, small,
               col::muted.alpha(in));
}
