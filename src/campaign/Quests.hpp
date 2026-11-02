#pragma once

#include <string>

#include "entity/enemy/Enemy.hpp"
#include "level/Structure.hpp"

class Level;
struct Campaign;

enum class Objective { Kindle, Slay, Hunt, Boss };
enum class QuestStage { Offered, Active, Ready, Done };

struct QuestDef {
  const char *title, *offer, *task, *thanks;
  Objective goal;
  int count;
  StructureKind site;
  int named;
  EnemyKind prey[3];
  int gold, xp;
};

constexpr int QUEST_COUNT = 4;
extern const QuestDef QUESTS[QUEST_COUNT];

namespace quests {

constexpr float ARRIVE = 150;

const QuestDef *current(const Campaign &c);
void accept(Level &level, Campaign &c);
void turnIn(Level &level, Campaign &c);
void tick(Level &level, Campaign &c);
void begin(Level &level, Campaign &c);
std::string status(const Campaign &c);
bool marker(const Campaign &c, double &x, double &y);

} // namespace quests
