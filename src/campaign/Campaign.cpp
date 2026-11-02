#include "campaign/Campaign.hpp"

#include <spdlog/fmt/fmt.h>

#include <cmath>
#include <sstream>

#include "Game.hpp"
#include "campaign/Stash.hpp"
#include "campaign/Waypoint.hpp"
#include "entity/Player.hpp"
#include "hero/Hero.hpp"
#include "item/Upgrade.hpp"
#include "level/Level.hpp"
#include "level/World.hpp"
#include "particle/Shockwave.hpp"

namespace {

double townX = 0, townY = 0;

float areaThreat(double x, double y) {
  double xd = x - townX, yd = y - townY;
  return float(std::sqrt(xd * xd + yd * yd) / 8);
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
  out += fmt::format("world {}\nhero {}\nlevel {} {:.2f} {:.2f}\ngold {}\nday {}\n", world, hero,
                     lvl, exp, expNext, gold, day);
  out += fmt::format("pos {:.1f} {:.1f}\ntown {:.1f} {:.1f} {}\n", x, y, xTown, yTown,
                     int(founded));
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
    else if (key == "pos")
      words >> out.x >> out.y;
    else if (key == "town")
      words >> out.xTown >> out.yTown >> out.founded;
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
  r.loot = true, r.bosses = false, r.omen = -1;
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
  townX = xTown, townY = yTown;
  level.rules.safe.assign(1, {xTown, yTown, SAFE});
  level.night.dawn = true, level.night.dayEnds = level.time + DAY;
  level.night.nights = day;
  restore(level);
  raise(level);
}

void Campaign::found(Level &level) {
  Player &p = *level.player;
  xTown = p.x, yTown = p.y, x = p.x, y = p.y, founded = true;
  int cx = int(std::floor(xTown / 16)), cy = int(std::floor(yTown / 16));
  for (int ty = cy - 5; ty <= cy + 5; ty++)
    for (int tx = cx - 6; tx <= cx + 6; tx++)
      edits.harvest(tx, ty);
  for (int tx = cx - 6; tx <= cx + 6; tx++)
    if (std::abs(tx - cx) > 1)
      edits.place(tx, cy - 5, Piece::Rail), edits.place(tx, cy + 5, Piece::Rail);
  for (int ty = cy - 4; ty <= cy + 4; ty++)
    if (std::abs(ty - cy) > 1)
      edits.place(cx - 6, ty, Piece::Post), edits.place(cx + 6, ty, Piece::Post);
  edits.place(cx, cy + 2, Piece::Campfire);
  for (int sx : {-4, 4})
    for (int sy : {-3, 3})
      edits.place(cx + sx, cy + sy, Piece::Torch);
}

void Campaign::restore(Level &level) {
  Player &p = *level.player;
  p.x = x, p.y = y;
  p.lvl = lvl, p.exp = exp, p.expNext = expNext, p.gold = gold;
  for (int m = 0; m < MATS; m++)
    p.materials[m] = materials[m];
  for (const auto &[name, want] : upgrades)
    if (Upgrade *up = p.upgrade(name))
      while (up->level < want && !up->maxed())
        up->levelUp(p);
  p.bag.clear();
  for (int s = 0; s < GEAR; s++)
    if (worn[s])
      p.bag.push_back(gear[s]), p.equip(p.bag.size() - 1);
  p.bag = bag;
  p.hp = p.maxHp;
}

void Campaign::raise(Level &level) {
  waypoint = level.add<Waypoint>(xTown, yTown - 6)->id;
  chest = level.add<Stash>(xTown + 40, yTown)->id;
}

void Campaign::fall(Level &level) {
  Player &p = *level.player;
  int lost = p.gold / 10;
  p.gold -= lost;
  p.hp = p.maxHp, p.invuln = 3;
  p.x = xTown, p.y = yTown + 20;
  p.status.clear();
  level.add<Shockwave>(p.x, p.bodyY(), 5, 50, 0.6f, Color{0.5f, 0.85f, 1, 1});
  level.game.announce(lost > 0 ? fmt::format("YOU WAKE BY THE WAYPOINT  -{} GOLD", lost)
                               : std::string("YOU WAKE BY THE WAYPOINT"));
}

void Campaign::leave() {
  if (world::edits == &edits)
    world::edits = nullptr;
}
