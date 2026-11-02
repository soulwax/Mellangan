#include "campaign/Campaign.hpp"

#include <spdlog/fmt/fmt.h>

#include <cmath>
#include <sstream>

#include "Game.hpp"
#include "campaign/Harvest.hpp"
#include "campaign/Elder.hpp"
#include "campaign/Stash.hpp"
#include "campaign/Waypoint.hpp"
#include "entity/Merchant.hpp"
#include "entity/Player.hpp"
#include "hero/Hero.hpp"
#include "item/Upgrade.hpp"
#include "item/skill/Skill.hpp"
#include "item/tree/Tree.hpp"
#include "level/Level.hpp"
#include "level/World.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Drift.hpp"

namespace {

double townX = 0, townY = 0;
float threatScale = 1;

float areaThreat(double x, double y) {
  double xd = x - townX, yd = y - townY;
  return float(std::sqrt(xd * xd + yd * yd) / 8) * threatScale + (threatScale - 1) * 300;
}

constexpr int TOWN_W = 6, TOWN_H = 5;

struct Spot {
  int dx, dy;
  Piece piece;
};

const Spot BARRICADE[] = {
    {-1, -TOWN_H, Piece::Cart},          {1, -TOWN_H, Piece::Crates},
    {-1, TOWN_H, Piece::Logs},           {1, TOWN_H, Piece::Sacks},
    {-TOWN_W, -1, Piece::Crates},        {-TOWN_W, 1, Piece::Sacks},
    {TOWN_W, -1, Piece::Logs},           {TOWN_W, 1, Piece::Crates},
    {-1, -TOWN_H - 2, Piece::Thorns},    {1, -TOWN_H - 2, Piece::Thorns},
    {-1, TOWN_H + 2, Piece::Thorns},     {1, TOWN_H + 2, Piece::Thorns},
    {-TOWN_W - 2, -1, Piece::Thorns},    {-TOWN_W - 2, 1, Piece::Thorns},
    {TOWN_W + 2, -1, Piece::Thorns},     {TOWN_W + 2, 1, Piece::Thorns},
    {2, TOWN_H + 1, Piece::Sign},        {-3, -TOWN_H - 1, Piece::Skull},
    {4, TOWN_H + 2, Piece::Ribs},        {-TOWN_W - 1, 3, Piece::Skull},
    {TOWN_W + 1, -3, Piece::Thorns},     {-TOWN_W - 1, -3, Piece::Thorns},
    {3, -3, Piece::Charred},             {-5, -4, Piece::Sacks},
    {5, 4, Piece::Crates},               {-5, 4, Piece::Logs},
    {0, 2, Piece::Campfire},
};

void clearing(Edits &edits, double x, double y) {
  int cx = int(std::floor(x / 16)), cy = int(std::floor(y / 16));
  for (int ty = cy - TOWN_H; ty <= cy + TOWN_H; ty++)
    for (int tx = cx - TOWN_W; tx <= cx + TOWN_W; tx++)
      edits.harvest(tx, ty);
}

template <class F> void townLayout(int cx, int cy, F &&fn) {
  fn(cx - TOWN_W, cy - TOWN_H, Piece::CornerNW), fn(cx + TOWN_W, cy - TOWN_H, Piece::CornerNE);
  fn(cx - TOWN_W, cy + TOWN_H, Piece::CornerSW), fn(cx + TOWN_W, cy + TOWN_H, Piece::CornerSE);
  for (int d = 2; d < TOWN_W; d++) {
    Piece west = d == 2 ? Piece::EndEast : Piece::Rail, east = d == 2 ? Piece::EndWest : Piece::Rail;
    fn(cx - d, cy - TOWN_H, west), fn(cx - d, cy + TOWN_H, west);
    fn(cx + d, cy - TOWN_H, east), fn(cx + d, cy + TOWN_H, east);
  }
  for (int d = 2; d < TOWN_H; d++)
    for (int s : {-1, 1})
      fn(cx - TOWN_W, cy + s * d, Piece::Post), fn(cx + TOWN_W, cy + s * d, Piece::PostEast);
  for (const Spot &s : BARRICADE)
    fn(cx + s.dx, cy + s.dy, s.piece);
  for (int sx : {-4, 4})
    for (int sy : {-3, 3})
      fn(cx + sx, cy + sy, Piece::Torch);
}

std::string_view rest(std::string_view line, int words) {
  for (int i = 0; i < words; i++) {
    size_t at = line.find(' ');
    if (at == std::string_view::npos)
      return {};
    line.remove_prefix(at + 1);
  }
  return line;
}

} // namespace

std::string Campaign::save() const {
  std::string out = fmt::format("mellangan-campaign {}\n", VERSION);
  out += fmt::format("world {}\nhero {}\nlevel {} {:.2f} {:.2f}\ngold {}\nday {}\npoints {}\n",
                     world, hero, lvl, exp, expNext, gold, day, points);
  if (!active.empty())
    out += "skill " + active + "\n";
  out += fmt::format("quest {} {} {} {:.1f} {:.1f} {} {}\n", quest, int(stage), progress, qx, qy,
                     int(qsite), act);
  out += fmt::format("pos {:.1f} {:.1f}\ntown {:.1f} {:.1f} {}\n", x, y, xTown, yTown,
                     int(founded));
  for (const Town &t : outposts)
    out += fmt::format("outpost {:.1f} {:.1f}\n", t.x, t.y);
  out += fmt::format("mode {} {}\n", int(hardcore), nightmare);
  for (const auto &[dx, dy] : delves)
    out += fmt::format("delved {} {}\n", dx, dy);
  out += "mats";
  for (int m : materials)
    out += fmt::format(" {}", m);
  out += "\n";
  for (const auto &[name, level] : upgrades)
    out += fmt::format("up {} {}\n", level, name);
  for (int s = 0; s < GEAR; s++)
    if (worn[s])
      out += fmt::format("gear {} {}\n", s, gear[s].save());
  for (const Item &item : bag)
    out += "bag " + item.save() + "\n";
  for (const Item &item : stash)
    out += "stash " + item.save() + "\n";
  return out + edits.save();
}

bool Campaign::load(std::string_view text, Campaign &out) {
  std::istringstream in{std::string(text)};
  std::string line;
  int version = 0;
  if (!std::getline(in, line) || std::sscanf(line.c_str(), "mellangan-campaign %d", &version) != 1 ||
      version < 1 || version > VERSION)
    return false;
  out = Campaign{};
  while (std::getline(in, line)) {
    std::istringstream words(line);
    std::string key;
    words >> key;
    Item item;
    if (key == "world")
      words >> out.world;
    else if (key == "hero")
      words >> out.hero;
    else if (key == "level")
      words >> out.lvl >> out.exp >> out.expNext;
    else if (key == "gold")
      words >> out.gold;
    else if (key == "day")
      words >> out.day;
    else if (key == "points")
      words >> out.points;
    else if (key == "quest") {
      int stage = 0;
      words >> out.quest >> stage >> out.progress >> out.qx >> out.qy >> out.qsite >> out.act;
      out.stage = QuestStage(std::clamp(stage, 0, int(QuestStage::Done)));
    }
    else if (key == "skill")
      out.active = std::string(rest(line, 1));
    else if (key == "pos")
      words >> out.x >> out.y;
    else if (key == "town")
      words >> out.xTown >> out.yTown >> out.founded;
    else if (key == "mode")
      words >> out.hardcore >> out.nightmare;
    else if (key == "delved") {
      std::pair<int, int> d{};
      words >> d.first >> d.second;
      out.delves.push_back(d);
    } else if (key == "outpost") {
      Town t{};
      words >> t.x >> t.y;
      out.outposts.push_back(t);
    }
    else if (key == "mats")
      for (int &m : out.materials)
        words >> m;
    else if (key == "up") {
      int level = 0;
      words >> level;
      out.upgrades.emplace_back(std::string(rest(line, 2)), level);
    } else if (key == "gear") {
      int slot = -1;
      words >> slot;
      if (slot >= 0 && slot < GEAR && Item::load(rest(line, 2), item))
        out.gear[slot] = item, out.worn[slot] = true;
    } else if (key == "bag" && Item::load(rest(line, 1), item))
      out.bag.push_back(item);
    else if (key == "stash" && Item::load(rest(line, 1), item))
      out.stash.push_back(item);
    else if (key == "edit")
      out.edits.load(line);
  }
  return !out.world.empty();
}

RunRules Campaign::rules() const {
  RunRules r;
  r.nightLength = NIGHT, r.dayLength = DAY, r.endsAtDawn = false;
  r.loot = true, r.bosses = false, r.town = true, r.omen = -1, r.dayRate = DAY_RATE;
  r.safe.push_back({xTown, yTown, SAFE});
  r.threat = areaThreat;
  return r;
}

void Campaign::capture(const Level &level) {
  const Player &p = *level.player;
  lvl = p.lvl, exp = p.exp, expNext = p.expNext, gold = p.gold;
  for (int m = 0; m < MATS; m++)
    materials[m] = p.materials[m];
  bag = p.bag, gear = p.gear, worn = p.worn;
  x = p.x, y = p.y, day = level.night.nights;
  active = p.skill ? p.skill->name : "";
  upgrades.clear();
  for (const auto &up : p.upgrades) {
    int own = up->level;
    for (int s = 0; s < GEAR; s++)
      if (p.worn[s] && p.gear[s].grants() && up.get() == p.upgrade(p.gear[s].grants()))
        own -= p.gearLevels[s];
    if (own > 0)
      upgrades.emplace_back(up->name, own);
  }
}

void Campaign::enter(Level &level) {
  world::edits = &edits;
  if (!founded)
    found(level);
  Delve::unseal(edits);
  delve = Delve{};
  barricade();
  threatScale = 1 + nightmare;
  townX = xTown, townY = yTown;
  level.rules.safe.clear();
  for (const Town &t : towns())
    level.rules.safe.push_back({t.x, t.y, SAFE});
  for (const auto &[k, e] : edits.changes)
    if (e.placed && e.piece == Piece::Campfire) {
      double fx = int(int32_t(k >> 32)) * 16 + 8.0, fy = int(int32_t(k & 0xFFFFFFFFu)) * 16 + 16.0;
      double xd = fx - xTown, yd = fy - yTown;
      if (xd * xd + yd * yd > harvest::TOWN * harvest::TOWN)
        level.rules.safe.push_back({fx, fy, harvest::CAMPFIRE_SAFE});
    }
  regrow(day);
  level.night.dawn = true, level.night.dayEnds = level.time + DAY;
  level.night.nights = day;
  restore(level);
  raise(level);
}

void Campaign::found(Level &level) {
  Player &p = *level.player;
  xTown = p.x, yTown = p.y, x = p.x, y = p.y, founded = true;
  clearing(edits, xTown, yTown);
}

std::vector<Campaign::Town> Campaign::towns() const {
  std::vector<Town> all{{xTown, yTown}};
  all.insert(all.end(), outposts.begin(), outposts.end());
  return all;
}

void Campaign::settle(Level &level, double tx, double ty) {
  Town t{tx, ty};
  outposts.push_back(t);
  clearing(edits, tx, ty);
  barricade();
  level.rules.safe.push_back({tx, ty, SAFE});
  post(level, t);
  staff(level);
  level.add<Shockwave>(tx, ty, 8, 120, 0.8f, col::gold);
}

void Campaign::travel(Level &level, int town) {
  std::vector<Town> all = towns();
  if (town < 0 || town >= int(all.size()))
    return;
  Player &p = *level.player;
  p.x = all[size_t(town)].x, p.y = all[size_t(town)].y + 20;
  level.add<Shockwave>(p.x, p.bodyY(), 5, 50, 0.6f, Color{0.45f, 0.85f, 1, 1});
}

void Campaign::barricade() {
  for (const Town &t : towns())
    townLayout(int(std::floor(t.x / 16)), int(std::floor(t.y / 16)),
               [&](int tx, int ty, Piece piece) { edits.place(tx, ty, piece); });
}

bool Campaign::founding(int tx, int ty) const {
  bool hit = false;
  for (const Town &t : towns())
    townLayout(int(std::floor(t.x / 16)), int(std::floor(t.y / 16)),
               [&](int x, int y, Piece) { hit |= x == tx && y == ty; });
  return hit;
}

void Campaign::restore(Level &level) {
  Player &p = *level.player;
  p.x = x, p.y = y;
  p.lvl = lvl, p.exp = exp, p.expNext = expNext, p.gold = gold;
  for (int m = 0; m < MATS; m++)
    p.materials[m] = materials[m];
  for (const auto &[name, want] : upgrades)
    if (Upgrade *up = p.upgrade(name); up && up->skill)
      Tree::rank(*up, want);
    else if (up)
      while (up->level < want && !up->maxed())
        up->levelUp(p);
  if (Upgrade *up = active.empty() ? nullptr : p.upgrade(active); up && up->level > 0)
    up->levelUp(p);
  if (p.skill && p.skill->level == 0)
    Tree::rank(*p.skill, 1);
  p.bag.clear();
  for (int s = 0; s < GEAR; s++)
    if (worn[s])
      p.bag.push_back(gear[s]), p.equip(p.bag.size() - 1);
  p.bag = bag;
  p.hp = p.maxHp;
}

void Campaign::raise(Level &level) {
  waypoints.clear(), chests.clear();
  for (const Town &t : towns())
    post(level, t);
  staff(level);
  quests::begin(level, *this);
}

void Campaign::post(Level &level, const Town &t) {
  waypoints.push_back(level.add<Waypoint>(t.x, t.y - 6)->id);
  chests.push_back(level.add<Stash>(t.x + 40, t.y)->id);
}

void Campaign::staff(Level &level) {
  Town h = home();
  if (Elder *old = level.get<Elder>(elder))
    old->remove();
  Merchant::camp(level, h.x - 62, h.y + 34);
  elder = level.add<Elder>(h.x - 36, h.y - 26)->id;
}

void Campaign::awaken(Level &level) {
  nightmare++, act = 1, quest = 0, progress = 0, qsite = false;
  stage = QuestStage::Offered;
  threatScale = 1 + nightmare;
  delves.clear();
  staff(level);
  level.game.announce(fmt::format("NIGHTMARE {}  THE DARK RETURNS STRONGER", nightmare));
}

void Campaign::fall(Level &level) {
  delve.abort(level, *this);
  Player &p = *level.player;
  int lost = p.gold / 10;
  p.gold -= lost;
  p.hp = p.maxHp, p.invuln = 3;
  p.x = home().x, p.y = home().y + 20;
  p.status.clear();
  level.add<Shockwave>(p.x, p.bodyY(), 5, 50, 0.6f, Color{0.5f, 0.85f, 1, 1});
  level.game.announce(lost > 0 ? fmt::format("YOU WAKE BY THE WAYPOINT  -{} GOLD", lost)
                               : std::string("YOU WAKE BY THE WAYPOINT"));
}

void Campaign::regrow(int today) {
  for (auto it = edits.changes.begin(); it != edits.changes.end();)
    if (!it->second.placed && it->second.regrow > 0 && it->second.regrow <= float(today))
      it = edits.changes.erase(it);
    else
      ++it;
}

void Campaign::smoulder(Level &level) {
  if ((smoke -= DT) > 0 || !Drift::room())
    return;
  smoke = 0.22f;
  Town near = home();
  const Player &p = *level.player;
  for (const Town &t : towns())
    if (std::hypot(t.x - p.x, t.y - p.y) < std::hypot(near.x - p.x, near.y - p.y))
      near = t;
  double cx = std::floor(near.x / 16) * 16 + 8, cy = std::floor(near.y / 16) * 16 + 16;
  auto plume = [&](double px, double py, float shade) {
    Drift *d = level.add<Drift>(px + rng.range(-3, 3), py, rng.range(-4, 4), -rng.range(10, 16),
                                atlas::pt::Puff, Color{shade, shade, shade + 0.03f, 0.55f},
                                rng.range(2.4f, 3.4f));
    d->grow = 1.1f, d->sway = 5, d->drag = 0.4f, d->spin = rng.range(-0.6f, 0.6f);
  };
  plume(cx + 3 * 16, cy - 3 * 16 - 26, 0.32f);
  if (rng.chance(0.45f))
    plume(cx, cy + 2 * 16 - 8, 0.45f);
  if (rng.chance(0.3f)) {
    Drift *e = level.add<Drift>(cx + 3 * 16 + rng.range(-5, 5), cy - 3 * 16 - 20,
                                rng.range(-10, 10), -rng.range(22, 36), atlas::pt::Ember,
                                Color{1, 0.55f, 0.2f, 1}, rng.range(0.7f, 1.3f));
    e->sway = 8, e->drag = 0.6f;
  }
}

void Campaign::tick(Level &level) {
  smoulder(level);
  bool day = level.night.dawn;
  if (day && !wasDay)
    regrow(level.night.nights);
  wasDay = day;
  quests::tick(level, *this);
  delve.tick(level, *this);
}

void Campaign::leave() {
  if (world::edits == &edits)
    world::edits = nullptr;
}
