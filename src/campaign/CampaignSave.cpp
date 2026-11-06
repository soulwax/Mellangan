#include "campaign/Campaign.hpp"

#include <spdlog/fmt/fmt.h>

#include <algorithm>
#include <cstdio>
#include <sstream>

namespace {

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
  out += fmt::format("mode {} {}\nrescued {}\nhound {} {}\n", int(hardcore), nightmare,
                     rescued, int(hound), int(houndDown));
  for (const auto &[dx, dy] : delves)
    out += fmt::format("delved {} {}\n", dx, dy);
  for (int t : towers)
    out += fmt::format("tower {}\n", t);
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

bool Campaign::valid(std::string_view text) {
  Campaign probe;
  return load(text, probe);
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
    else if (key == "rescued")
      words >> out.rescued;
    else if (key == "hound")
      words >> out.hound >> out.houndDown;
    else if (key == "tower") {
      int t = -1;
      words >> t;
      out.towers.push_back(t);
    }
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
