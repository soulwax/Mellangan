#pragma once

#include <string>

#include "entity/enemy/Enemy.hpp"
#include "level/Structure.hpp"

class Level;
struct Campaign;

enum class Objective { Kindle, Slay, Hunt, Boss };
enum class QuestStage { Offered, Active, Ready, Done, Road };

struct QuestDef {
  const char *title, *offer, *task, *thanks;
  Objective goal;
  int count;
  StructureKind site;
  int named;
  EnemyKind prey[3];
  int gold, xp;
};

struct ActDef {
  const char *numeral, *town, *elder, *road;
  int biomes[6];
  int reach;
};

constexpr int ACT_COUNT = 5, QUESTS_PER_ACT = 4, QUEST_COUNT = ACT_COUNT * QUESTS_PER_ACT;
extern const QuestDef QUESTS[QUEST_COUNT];
extern const ActDef ACTS[ACT_COUNT];

namespace quests {

constexpr float ARRIVE = 150, SETTLE = 200;

const QuestDef *current(const Campaign &c);
const ActDef &act(const Campaign &c);
void accept(Level &level, Campaign &c);
void turnIn(Level &level, Campaign &c);
void tick(Level &level, Campaign &c);
void begin(Level &level, Campaign &c);
bool road(int act, double &x, double &y);
std::string status(const Campaign &c);
bool marker(const Campaign &c, double &x, double &y);

} // namespace quests
