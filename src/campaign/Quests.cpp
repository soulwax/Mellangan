#include "campaign/Quests.hpp"

#include <spdlog/fmt/fmt.h>

#include <cmath>

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "campaign/Campaign.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Boss.hpp"
#include "entity/enemy/Named.hpp"
#include "level/Level.hpp"
#include "level/World.hpp"
#include "level/biome/Biome.hpp"
#include "particle/Shockwave.hpp"

namespace {

using K = EnemyKind;
using S = StructureKind;
using O = Objective;
constexpr K NONE = K::Count;

bool ofKind(const Structure *s, const void *kind) {
  return s->kind == *static_cast<const StructureKind *>(kind);
}

int slainOf(const Level &level, const QuestDef &q) {
  int n = 0;
  for (EnemyKind k : q.prey)
    if (k != NONE)
      n += level.slain[int(k)];
  return n;
}

void findSite(Campaign &c, const QuestDef &q) {
  const Campaign::Town &home = c.home();
  Structure s;
  if (world::nearestStructure(home.x, home.y, 8, s, ofKind, &q.site))
    c.qx = s.centerX(), c.qy = s.centerY();
  else
    c.qx = home.x + 900, c.qy = home.y;
  c.qsite = true;
}

bool deep(int biome, int tx, int ty) {
  for (int k = 0; k < 4; k++)
    if (world::biomeAt(tx + (k & 1 ? 6 : -6), ty + (k & 2 ? 6 : -6)) != biome)
      return false;
  Structure site;
  if (world::biomeAt(tx, ty) != biome || world::isWater(tx, ty) ||
      world::structureAt(tx, ty, site, 12))
    return false;
  int ground = world::heightAt(tx, ty);
  for (int dy = -7; dy <= 7; dy++)
    for (int dx = -8; dx <= 8; dx++)
      if (world::heightAt(tx + dx, ty + dy) != ground || world::isRamp(tx + dx, ty + dy) ||
          world::isWater(tx + dx, ty + dy))
        return false;
  return true;
}

} // namespace

const ActDef ACTS[ACT_COUNT] = {
    {"I", "HEARTHWICK", "ELDER MAREN", "",
     {BIOME_MEADOW, BIOME_FOREST, BIOME_BLOSSOM, BIOME_AUTUMN, BIOME_HEATH, -1}, 0},
    {"II", "SALTMOOR", "ABBESS WREN", "the road into the marshes",
     {BIOME_MIRE, BIOME_FLOOD, BIOME_RUINS, BIOME_JUNGLE, BIOME_AMBER, BIOME_FUNGAL}, 70},
    {"III", "FROSTHOLD", "WARDEN KELL", "the road into the snow",
     {BIOME_SNOW, BIOME_TAIGA, BIOME_MOON, BIOME_CRYSTAL, BIOME_BARROW, -1}, 140},
    {"IV", "DUNEWATCH", "CARAVANEER IDRIS", "the road into the dunes",
     {BIOME_DESERT, BIOME_SAVANNA, BIOME_BADLANDS, BIOME_SALT, BIOME_GLASS, BIOME_STORM}, 210},
    {"V", "LAST EMBER", "KEEPER ASH", "the burning road",
     {BIOME_ASHLANDS, BIOME_EMBER, BIOME_HOLLOW, BIOME_STARFALL, -1, -1}, 280},
};

const QuestDef QUESTS[QUEST_COUNT] = {
    {"Rekindle the Greenwood",
     "The shrines have gone cold, and the night grows bolder for it. Light three of them and "
     "the dark will fear us again.",
     "Light three shrines", "Three fires burn again. The night will think twice.", O::Kindle, 3,
     S::Shrine, -1, {NONE, NONE, NONE}, 150, 40},
    {"The Barrow Keeper",
     "The miller went down into the old barrow and never came up. Something keeps him there. "
     "Bring him home.",
     "Free the miller from the barrow", "The miller is home, pale but breathing. Thank you.",
     O::Slay, 1, S::Graveyard, 5, {NONE, NONE, NONE}, 250, 70},
    {"Hunt the Pack",
     "Wolves circle the fields every night and they grow bolder. Thin them out. Twenty should "
     "teach them.",
     "Slay twenty wolves", "The fields are quieter. But the pack answers to someone.", O::Hunt,
     20, S::Count, -1, {K::Wolf, K::Alpha, K::MoonHound}, 200, 60},
    {"The Pack Lord",
     "Their master is the Pack Lord. He dens in a camp not far from here. End him, and the "
     "road south is ours.",
     "Slay the Pack Lord in his den",
     "The Pack Lord is dead. The road opens to the marshes, where Saltmoor still holds.", O::Boss,
     1, S::Camp, -1, {K::Alpha, NONE, NONE}, 500, 150},

    {"Drain the Lanterns",
     "Lights dance over the water and lead our people into the mire. They are the witch's eyes. "
     "Put out a dozen.",
     "Put out twelve bog lanterns", "Darker water, and safer for it.", O::Hunt, 12, S::Count, -1,
     {K::BogLantern, K::MireLeech, NONE}, 300, 90},
    {"The Sunken Reliquary",
     "Our reliquary sank with the old abbey. The Drowned Abbot guards it still. Bring back what "
     "he keeps.",
     "Recover the reliquary from the Drowned Abbot", "The reliquary is ours again. Bless you.",
     O::Slay, 1, S::Ruin, 3, {NONE, NONE, NONE}, 400, 120},
    {"Cleanse the Nests",
     "Spores drift into Saltmoor from the fungal wood. Burn out the puffers and their kin.",
     "Destroy twenty-five spore beasts", "The air is clean again, mostly.", O::Hunt, 25,
     S::Count, -1, {K::SporePuffer, K::Sporeling, K::MycelialBrute}, 350, 110},
    {"The Bog Witch",
     "The witch herself waits in her camp. Unmake her and the snow road opens north of the "
     "marsh.",
     "Slay the Bog Witch", "The witch is ash and the marsh breathes. Frosthold waits beyond.",
     O::Boss, 1, S::Camp, -1, {K::BogLantern, NONE, NONE}, 800, 240},

    {"Relight the Beacons",
     "The frozen beacons went dark one by one. Light three and the passes will be ours again.",
     "Light three shrines", "Three beacons burn over the snow.", O::Kindle, 3, S::Shrine, -1,
     {NONE, NONE, NONE}, 450, 140},
    {"Old Rimefang",
     "A yeti older than the walls hunts by the standing stones. He took our lens-bearer. End "
     "him.",
     "Slay Old Rimefang at the stones", "Rimefang is dead and the lens is ours.", O::Slay, 1,
     S::Stones, 4, {NONE, NONE, NONE}, 550, 170},
    {"Break the Phylactery",
     "The lich's servants carry pieces of his soul. Destroy thirty of them and he will be "
     "mortal.",
     "Slay thirty of the risen dead", "His servants are dust. He is only a man now, almost.",
     O::Hunt, 30, S::Count, -1, {K::Skeleton, K::Necromancer, K::BarrowWight}, 500, 160},
    {"The Lich",
     "Go to the barrow where the lich sleeps and finish what the winter began.",
     "Slay the Lich", "The lich is broken. The dunes call beyond the pass.", O::Boss, 1,
     S::Graveyard, -1, {K::Necromancer, NONE, NONE}, 1200, 360},

    {"Guard the Caravan",
     "Raiders and hyenas tear at every caravan. Clear the routes, twenty-five of them.",
     "Slay twenty-five raiders and hyenas", "The caravan made it through.", O::Hunt, 25, S::Count,
     -1, {K::DuneRaider, K::Hyena, NONE}, 650, 200},
    {"The Red Mesa",
     "A golem of salt and fire walks the flats. The glass spires answer to it. Shatter it.",
     "Shatter the Red Mesa", "The Red Mesa is gravel.", O::Slay, 1, S::Ruin, 6,
     {NONE, NONE, NONE}, 750, 230},
    {"Skyreaver",
     "Skyreaver rides the storm over the mesa. Bring it down and the herald will lose its eyes.",
     "Bring down Skyreaver", "The sky is quieter.", O::Slay, 1, S::Camp, 7, {NONE, NONE, NONE},
     800, 240},
    {"The Storm Herald",
     "The Storm Herald calls the lightning from its camp on the steppe. Silence it.",
     "Slay the Storm Herald", "The herald is silent. Smoke rises in the far south.", O::Boss, 1,
     S::Camp, -1, {K::Cultist, NONE, NONE}, 1600, 480},

    {"The Burning Road",
     "The road south is lined with dead fires. Light three and we can walk it.",
     "Light three shrines", "The burning road has lights again.", O::Kindle, 3, S::Shrine, -1,
     {NONE, NONE, NONE}, 900, 280},
    {"The Hollow Gate",
     "The Hollow King holds the gate below the ash. Break his court and open the way.",
     "Slay the Hollow King", "The gate stands open.", O::Slay, 1, S::Ruin, 9, {NONE, NONE, NONE},
     1100, 330},
    {"Ashmother",
     "Ashmother breeds the wraiths that choke the fen. Find her and end the brood.",
     "Slay Ashmother", "The fen is empty of wraiths.", O::Slay, 1, S::Camp, 8, {NONE, NONE, NONE},
     1200, 360},
    {"The Cinder King",
     "This is where the dark began. The Cinder King burns at its heart. End this.",
     "Slay the Cinder King", "It is over. The dark is broken, and the shrines will hold.",
     O::Boss, 1, S::Camp, -1, {K::CinderGolem, NONE, NONE}, 3000, 900},
};

namespace quests {

const QuestDef *current(const Campaign &c) {
  return c.quest >= 0 && c.quest < QUEST_COUNT ? &QUESTS[c.quest] : nullptr;
}

const ActDef &act(const Campaign &c) {
  return ACTS[std::clamp(c.act, 1, ACT_COUNT) - 1];
}

bool road(int act, double &x, double &y) {
  if (act < 2 || act > ACT_COUNT)
    return false;
  const ActDef &a = ACTS[act - 1];
  for (int r = a.reach; r < a.reach + 1200; r += 10)
    for (int i = 0, n = std::max(8, r / 2); i < n; i++) {
      double ang = TAU * i / n + act;
      int tx = int(std::cos(ang) * r), ty = int(std::sin(ang) * r);
      for (int b : a.biomes)
        if (b >= 0 && deep(b, tx, ty))
          return x = tx * 16 + 8.0, y = ty * 16 + 16.0, true;
    }
  return false;
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
  if (c.quest % QUESTS_PER_ACT != 0)
    return void(c.stage = QuestStage::Offered);
  level.game.announce(fmt::format("ACT {} CLEARED", act(c).numeral));
  if (c.quest >= QUEST_COUNT) {
    c.stage = QuestStage::Done;
    return level.game.announce("THE DARK IS BROKEN");
  }
  c.act++;
  if (int(c.towns().size()) >= c.act) {
    c.stage = QuestStage::Offered;
    c.staff(level);
    return level.game.announce(fmt::format("RETURN TO {}", act(c).town));
  }
  c.stage = QuestStage::Road;
  level.game.announce(fmt::format("FOLLOW {} TO {}", act(c).road, act(c).town));
}

void tick(Level &level, Campaign &c) {
  Player &p = *level.player;
  if (c.stage == QuestStage::Road) {
    double rx, ry;
    if (!road(c.act, rx, ry))
      return;
    double xd = p.x - rx, yd = p.y - ry;
    if (!p.dead() && xd * xd + yd * yd < SETTLE * SETTLE) {
      c.settle(level, rx, ry);
      c.stage = QuestStage::Offered;
      level.game.announce(fmt::format("ACT {}  {}", act(c).numeral, act(c).town));
    }
    return;
  }
  const QuestDef *q = current(c);
  if (!q || c.stage != QuestStage::Active)
    return;
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
                             : level.spawner.spawn(q->prey[0], c.qx, c.qy - 40, true, true);
    if (e) {
      c.foe = e->id;
      level.add<Shockwave>(c.qx, c.qy, 8, 90, 0.7f, Color{0.9f, 0.2f, 0.25f, 1});
      level.game.announce(q->goal == Objective::Boss ? bossOf(q->prey[0]).arrival
                                                     : fmt::format("{} STIRS", q->title));
    }
    break;
  }
  }
  if (c.progress >= q->count) {
    c.progress = q->count, c.stage = QuestStage::Ready;
    level.game.announce(fmt::format("{} COMPLETE  RETURN TO {}", q->title, act(c).elder));
    Sound::chest.play(0.5f, 0, 0.9f);
  }
}

std::string status(const Campaign &c) {
  if (c.stage == QuestStage::Done)
    return "The dark is broken";
  if (c.stage == QuestStage::Road)
    return fmt::format("Follow {} to {}", act(c).road, act(c).town);
  const QuestDef *q = current(c);
  if (!q)
    return "";
  switch (c.stage) {
  case QuestStage::Offered:
    return fmt::format("{} in {} has work for you", act(c).elder, act(c).town);
  case QuestStage::Ready:
    return fmt::format("{}: return to {}", q->title, act(c).elder);
  default:
    return q->count > 1 ? fmt::format("{}  {}/{}", q->task, c.progress, q->count) : q->task;
  }
}

bool marker(const Campaign &c, double &x, double &y) {
  if (c.stage == QuestStage::Road)
    return road(c.act, x, y);
  if (c.stage == QuestStage::Ready || c.stage == QuestStage::Offered)
    return x = c.home().x, y = c.home().y - 30, true;
  if (c.stage == QuestStage::Active && c.qsite)
    return x = c.qx, y = c.qy, true;
  return false;
}

} // namespace quests
