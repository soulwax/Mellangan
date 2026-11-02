#include "campaign/Quests.hpp"

#include <spdlog/fmt/fmt.h>

#include <cmath>

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "campaign/Campaign.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Named.hpp"
#include "level/Level.hpp"
#include "level/World.hpp"
#include "particle/Shockwave.hpp"

namespace {

using K = EnemyKind;
constexpr int PALE_WARDEN = 5;

bool ofKind(const Structure *s, const void *kind) {
  return s->kind == *static_cast<const StructureKind *>(kind);
}

int slainOf(const Level &level, const QuestDef &q) {
  int n = 0;
  for (EnemyKind k : q.prey)
    if (k != K::Count)
      n += level.slain[int(k)];
  return n;
}

void findSite(Campaign &c, const QuestDef &q) {
  Structure s;
  if (world::nearestStructure(c.xTown, c.yTown, 8, s, ofKind, &q.site))
    c.qx = s.centerX(), c.qy = s.centerY();
  else
    c.qx = c.xTown + 900, c.qy = c.yTown;
  c.qsite = true;
}

} // namespace

const QuestDef QUESTS[QUEST_COUNT] = {
    {"Rekindle the Greenwood",
     "The shrines have gone cold, and the night grows bolder for it. Light three of them and "
     "the dark will fear us again.",
     "Light three shrines", "Three fires burn again. The night will think twice.",
     Objective::Kindle, 3, StructureKind::Shrine, -1, {K::Count, K::Count, K::Count}, 150, 40},
    {"The Barrow Keeper",
     "The miller went down into the old barrow and never came up. Something keeps him there. "
     "Bring him home.",
     "Free the miller from the barrow", "The miller is home, pale but breathing. Thank you.",
     Objective::Slay, 1, StructureKind::Graveyard, PALE_WARDEN, {K::Count, K::Count, K::Count}, 250,
     70},
    {"Hunt the Pack",
     "Wolves circle the fields every night and they grow bolder. Thin them out. Twenty should "
     "teach them.",
     "Slay twenty wolves", "The fields are quieter. But the pack answers to someone.",
     Objective::Hunt, 20, StructureKind::Count, -1, {K::Wolf, K::Alpha, K::MoonHound}, 200, 60},
    {"The Pack Lord",
     "Their master is the Pack Lord. He dens in a camp not far from here. End him, and the "
     "road south is ours.",
     "Slay the Pack Lord in his den", "The Pack Lord is dead. Act I is yours. The road south opens.",
     Objective::Boss, 1, StructureKind::Camp, -1, {K::Count, K::Count, K::Count}, 500, 150},
};

namespace quests {

const QuestDef *current(const Campaign &c) {
  return c.quest >= 0 && c.quest < QUEST_COUNT ? &QUESTS[c.quest] : nullptr;
}

void begin(Level &level, Campaign &c) {
  c.litSeen = level.night.lit;
  if (const QuestDef *q = current(c))
    c.slainSeen = slainOf(level, *q);
  c.foe = entt::null;
}

void accept(Level &level, Campaign &c) {
  const QuestDef *q = current(c);
  if (!q || c.stage != QuestStage::Offered)
    return;
  c.stage = QuestStage::Active, c.progress = 0, c.qsite = false;
  if (q->site != StructureKind::Count && q->goal != Objective::Kindle)
    findSite(c, *q);
  begin(level, c);
  level.game.announce(fmt::format("QUEST  {}", q->title));
}

void turnIn(Level &level, Campaign &c) {
  const QuestDef *q = current(c);
  if (!q || c.stage != QuestStage::Ready)
    return;
  Player &p = *level.player;
  p.gold += q->gold;
  p.gainXp(float(q->xp));
  level.game.announce(fmt::format("{} DONE  +{} GOLD", q->title, q->gold));
  Sound::levelUp.play(0.6f, 0);
  c.quest++, c.progress = 0, c.qsite = false;
  c.stage = c.quest < QUEST_COUNT ? QuestStage::Offered : QuestStage::Done;
  if (c.stage == QuestStage::Done) {
    c.act = std::max(c.act, 2);
    level.game.announce("ACT I CLEARED");
  }
}

void tick(Level &level, Campaign &c) {
  const QuestDef *q = current(c);
  if (!q || c.stage != QuestStage::Active)
    return;
  Player &p = *level.player;
  switch (q->goal) {
  case Objective::Kindle:
    if (level.night.lit > c.litSeen)
      c.progress += level.night.lit - c.litSeen;
    c.litSeen = level.night.lit;
    break;
  case Objective::Hunt: {
    int now = slainOf(level, *q);
    c.progress += std::max(0, now - c.slainSeen);
    c.slainSeen = now;
    break;
  }
  case Objective::Slay:
  case Objective::Boss: {
    if (c.foe != entt::null) {
      if (!level.get<Enemy>(c.foe))
        c.progress = q->count, c.foe = entt::null;
      break;
    }
    double xd = p.x - c.qx, yd = p.y - c.qy;
    if (p.dead() || xd * xd + yd * yd > ARRIVE * ARRIVE)
      break;
    Enemy *e = q->named >= 0 ? named::raise(level, q->named, c.qx, c.qy - 40)
                             : level.spawner.spawn(K::Alpha, c.qx, c.qy - 40, true, true);
    if (e) {
      c.foe = e->id;
      level.add<Shockwave>(c.qx, c.qy, 8, 90, 0.7f, Color{0.9f, 0.2f, 0.25f, 1});
      level.game.announce(q->goal == Objective::Boss ? "THE PACK LORD HOWLS"
                                                : "THE BARROW KEEPER WAKES");
    }
    break;
  }
  }
  if (c.progress >= q->count) {
    c.progress = q->count, c.stage = QuestStage::Ready;
    level.game.announce(fmt::format("{} COMPLETE  RETURN TO THE ELDER", q->title));
    Sound::chest.play(0.5f, 0, 0.9f);
  }
}

std::string status(const Campaign &c) {
  const QuestDef *q = current(c);
  if (!q)
    return c.stage == QuestStage::Done ? "Act I cleared" : "";
  switch (c.stage) {
  case QuestStage::Offered:
    return "The elder in Hearthwick has work for you";
  case QuestStage::Ready:
    return fmt::format("{}: return to the elder", q->title);
  default:
    return q->count > 1 ? fmt::format("{}  {}/{}", q->task, c.progress, q->count) : q->task;
  }
}

bool marker(const Campaign &c, double &x, double &y) {
  if (c.stage == QuestStage::Ready || c.stage == QuestStage::Offered)
    return x = c.xTown, y = c.yTown - 30, true;
  if (c.stage == QuestStage::Active && c.qsite)
    return x = c.qx, y = c.qy, true;
  return false;
}

} // namespace quests
