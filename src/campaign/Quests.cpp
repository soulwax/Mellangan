#include "campaign/Quests.hpp"

#include <spdlog/fmt/fmt.h>

#include <cmath>

#include "gui/Lang.hpp"
#include "Game.hpp"
#include "audio/Sound.hpp"
#include "item/craft/Recipes.hpp"
#include "item/loot/Loot.hpp"
#include "entity/pickup/ItemDrop.hpp"
#include "campaign/Campaign.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Boss.hpp"
#include "entity/enemy/Named.hpp"
#include "entity/enemy/WildFoe.hpp"
#include "campaign/Cache.hpp"
#include "campaign/Pilgrim.hpp"
#include "level/Level.hpp"
#include "level/World.hpp"
#include "level/biome/Biome.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"

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
          world::isWater(tx + dx, ty + dy) ||
          BIOMES[world::biomeAt(tx + dx, ty + dy)]->fixedGround(tx + dx, ty + dy))
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
     "Our reliquary sank with the old abbey, and the Drowned Abbot seals it still. Break his "
     "hold and bring it home. Do not fall on the way.",
     "Recover the reliquary from the ruins", "The reliquary is ours again. Bless you.",
     O::Fetch, 1, S::Ruin, 3, {NONE, NONE, NONE}, 400, 120},
    {"Cleanse the Nests",
     "Spore nests have taken root in the old ruins and breed puffers by the dozen. Burn out "
     "every nest.",
     "Destroy the spore nests", "The air is clean again, mostly.", O::Purge, 4, S::Ruin, -1,
     {K::SporePuffer, NONE, NONE}, 350, 110},
    {"The Bog Witch",
     "The witch herself waits in her camp. Unmake her and the snow road opens north of the "
     "marsh.",
     "Slay the Bog Witch", "The witch is ash and the marsh breathes. Frosthold waits beyond.",
     O::Boss, 1, S::Camp, -1, {K::BogLantern, NONE, NONE}, 800, 240},

    {"Fuel the Beacons",
     "The beacons on the walls burn through our wood faster than we can cut it. Bring me "
     "twelve good logs and the passes stay lit.",
     "Bring twelve wood to Warden Kell", "The beacons roar. The passes are ours again.",
     O::Gather, 12, S::Count, -1, {NONE, NONE, NONE}, 450, 140, int(Mat::Wood)},
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
     "A caravan waits at the camp with water for the whole town. Raiders and hyenas are "
     "circling. Stand with it until it can leave.",
     "Hold the caravan camp", "The caravan made it through. Dunewatch will drink tonight.",
     O::Defend, 45, S::Camp, -1, {K::DuneRaider, K::Hyena, NONE}, 650, 200},
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
     "Sister Imra must carry the last lantern to the shrine at the end of the burning road. "
     "She will not walk it alone. Stay close, and keep her alive.",
     "Escort Sister Imra to the shrine", "Her lantern burns at the shrine. The road is lit.",
     O::Escort, 100, S::Shrine, -1, {NONE, NONE, NONE}, 900, 280},
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

Rarity reward(int quest) {
  if (quest == QUEST_COUNT - 1)
    return Rarity::Unique;
  return QUESTS[quest].goal == Objective::Boss ? Rarity::Rare : Rarity::Magic;
}

unsigned rewardSlots(int quest) {
  constexpr unsigned SLOTS[ACT_COUNT] = {slot::Weapon, slot::Offhand, slot::Armour, slot::Charm,
                                         slot::Weapon};
  return SLOTS[quest / QUESTS_PER_ACT];
}

std::string rewardText(int quest) {
  constexpr const char *SLOT_NAMES[ACT_COUNT] = {"weapon", "off-hand", "armour", "charm",
                                                 "weapon"};
  return fmt::format("{} gold and a {} {}", QUESTS[quest].gold, rarityName(reward(quest)),
                     SLOT_NAMES[quest / QUESTS_PER_ACT]);
}

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
  c.nests.clear(), c.holding = false, c.holdTick = 0, c.waveTime = 0;
  c.pilgrim = entt::null, c.cache = entt::null, c.guarded = false;
}

void accept(Level &level, Campaign &c) {
  const QuestDef *q = current(c);
  if (!q || c.stage != QuestStage::Offered)
    return;
  c.stage = QuestStage::Active, c.progress = 0, c.qsite = false;
  if (q->site != StructureKind::Count && q->goal != Objective::Kindle)
    findSite(c, *q);
  if (q->goal == Objective::Escort) {
    Campaign::Town h = c.home();
    if (!c.qsite || std::hypot(c.qx - h.x, c.qy - h.y) < 300)
      c.qx = h.x + 700, c.qy = h.y + 300, c.qsite = true;
  }
  begin(level, c);
  level.game.announce(say(lang.questBanner, tr(q->title)));
}

void turnIn(Level &level, Campaign &c) {
  const QuestDef *q = current(c);
  if (!q || c.stage != QuestStage::Ready)
    return;
  Player &p = *level.player;
  if (q->goal == Objective::Gather) {
    int &have = p.materials[q->material];
    if (have < q->count) {
      c.stage = QuestStage::Active;
      return level.game.announce(say(lang.elderNeeds, tr(act(c).elder), q->count, tr(MATERIALS[q->material].plural)));
    }
    have -= q->count;
  }
  p.gold += q->gold;
  p.gainXp(float(q->xp));
  int ilvl = std::clamp(std::max(p.lvl, level.monsterLevel(c.qx, c.qy)) + 2, 1, 99);
  Item prize = rollItem(ilvl, p.bonus().magicFind, false, rng, reward(c.quest), rewardSlots(c.quest));
  for (int i = 0; i < 64 && !(prize.def().slots & rewardSlots(c.quest)); i++)
    prize = rollItem(ilvl, p.bonus().magicFind, false, rng, reward(c.quest), rewardSlots(c.quest));
  if (int(p.bag.size()) < Player::BAG)
    p.bag.push_back(prize);
  else
    level.add<ItemDrop>(p.x, p.y, Drop{DropKind::Item, 1, prize}, p.x + 14, p.y + 8);
  spdlog::info("quest: {} rewards {} [{}] ilvl {}", q->title, prize.name(),
               rarityName(prize.rarity), prize.ilvl);
  level.game.announce(say(lang.questDone, tr(q->title), q->gold));
  level.game.announce(say(lang.rewardBanner, prize.name(true)));
  Sound::levelUp.play(0.6f, 0);
  c.quest++, c.progress = 0, c.qsite = false;
  if (c.quest % QUESTS_PER_ACT != 0)
    return void(c.stage = QuestStage::Offered);
  level.game.announce(say(lang.actCleared, act(c).numeral));
  level.game.celebrate();
  for (int i = 0; i < 3; i++)
    level.add<Shockwave>(p.x, p.bodyY(), 6 + i * 4, 70 + i * 40, 0.6f + i * 0.2f, col::gold);
  Spark::burst(level, p.x, p.bodyY() - 10, col::gold, 40, 160, true);
  Sound::chest.play(0.8f, 0, 0.7f);
  if (c.quest >= QUEST_COUNT) {
    c.stage = QuestStage::Done;
    return level.game.announce(lang.darkBroken);
  }
  c.act++;
  if (int(c.towns().size()) >= c.act) {
    c.stage = QuestStage::Offered;
    c.staff(level);
    return level.game.announce(say(lang.returnTo, tr(act(c).town)));
  }
  c.stage = QuestStage::Road;
  level.game.announce(say(lang.followTo, tr(act(c).road), tr(act(c).town)));
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
      level.game.announce(say(lang.actBastion, act(c).numeral, tr(act(c).town)));
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
  case Objective::Purge: {
    double xd = p.x - c.qx, yd = p.y - c.qy;
    if (c.nests.empty() && !p.dead() && xd * xd + yd * yd < ARRIVE * ARRIVE) {
      for (int i = 0; i < q->count - c.progress; i++) {
        double a = TAU * i / std::max(1, q->count - c.progress) + 0.4, r = 70;
        Enemy *e = level.spawner.spawn(K::Nest, c.qx + std::cos(a) * r,
                                       c.qy + std::sin(a) * r * 0.7, true);
        if (auto *w = dynamic_cast<WildFoe *>(e)) {
          w->brood = q->prey[0];
          c.nests.push_back(e->id);
        }
      }
      if (!c.nests.empty())
        level.game.announce(lang.nestsPulse);
    }
    int alive = 0;
    for (entt::entity id : c.nests)
      if (level.reg.valid(id)) {
        Enemy *e = level.get<Enemy>(id);
        alive += !e || !e->removed;
      }
    if (!c.nests.empty())
      c.progress = q->count - alive;
    break;
  }
  case Objective::Defend: {
    double xd = p.x - c.qx, yd = p.y - c.qy, d2 = xd * xd + yd * yd;
    if (!c.holding) {
      if (!p.dead() && d2 < ARRIVE * ARRIVE)
        c.holding = true, c.waveTime = 1, level.game.announce(lang.holdCamp);
      break;
    }
    if (d2 < HOLD * HOLD && !p.dead() && (c.holdTick += DT) >= 1)
      c.holdTick -= 1, c.progress++;
    if ((c.waveTime -= DT) <= 0) {
      c.waveTime = WAVE;
      double a = rng.range(0, TAU);
      for (int i = 0; i < 3 + c.progress / 15; i++) {
        K kind = q->prey[i % 2] != NONE ? q->prey[i % 2] : q->prey[0];
        level.spawner.spawn(kind, c.qx + std::cos(a) * 230 + rng.range(-20, 20),
                            c.qy + std::sin(a) * 230 + rng.range(-20, 20), true);
      }
    }
    break;
  }
  case Objective::Fetch: {
    Cache *box = c.cache == entt::null ? nullptr : level.get<Cache>(c.cache);
    double xd = p.x - c.qx, yd = p.y - c.qy;
    if (c.cache == entt::null && !p.dead() && xd * xd + yd * yd < ARRIVE * ARRIVE) {
      c.cache = level.add<Cache>(c.qx, c.qy)->id;
      if (!c.guarded && q->named >= 0)
        if (Enemy *g = named::raise(level, q->named, c.qx, c.qy - 50))
          c.foe = g->id, c.guarded = true;
      level.game.announce(lang.sealedReliquary);
      break;
    }
    if (!box)
      break;
    bool keeper = c.foe != entt::null && level.reg.valid(c.foe);
    if (box->sealed && !keeper) {
      box->sealed = false, c.foe = entt::null;
      level.add<Shockwave>(box->x, box->y - 6, 4, 34, 0.5f, Color{1, 0.85f, 0.45f, 1}, false);
      level.game.announce(lang.sealBreaks);
    }
    if (!box->sealed && std::hypot(p.x - box->x, p.y - box->y) < Cache::REACH + p.radius) {
      box->remove(), c.cache = entt::null, c.progress = q->count;
      Sound::chest.play(0.6f, 0, 0.9f);
    }
    break;
  }
  case Objective::Gather:
    c.progress = std::min(q->count, p.materials[q->material]);
    break;
  case Objective::Escort: {
    Pilgrim *w = c.pilgrim == entt::null ? nullptr : level.get<Pilgrim>(c.pilgrim);
    if (c.pilgrim != entt::null && !w && !level.reg.valid(c.pilgrim)) {
      c.pilgrim = entt::null, c.progress = 0, c.stage = QuestStage::Offered, c.qsite = false;
      level.game.announce(lang.imraFallen);
      Sound::death.play(0.5f, 0);
      return;
    }
    if (c.pilgrim == entt::null) {
      Campaign::Town h = c.home();
      double sx = h.x, sy = std::floor(h.y / 16) * 16 + 7 * 16;
      c.journey = std::max(1.0, std::hypot(c.qx - sx, c.qy - sy));
      c.pilgrim = level.add<Pilgrim>(sx, sy, c.qx, c.qy)->id;
      level.game.announce(lang.imraSetsOut);
      break;
    }
    if (!w)
      break;
    double left = std::hypot(w->tx - w->x, w->ty - w->y);
    c.progress = w->arrived() ? q->count
                              : std::clamp(int(100 * (1 - left / c.journey)), 0, q->count - 1);
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
      level.game.announce(q->goal == Objective::Boss ? std::string(tr(bossOf(q->prey[0]).arrival))
                                                     : say(lang.questStirs, tr(q->title)));
    }
    break;
  }
  }
  if (c.progress >= q->count) {
    c.progress = q->count, c.stage = QuestStage::Ready;
    level.game.announce(q->goal == Objective::Fetch
                            ? say(lang.carryHome, tr(act(c).elder))
                            : say(lang.questComplete, tr(q->title), tr(act(c).elder)));
    Sound::chest.play(0.5f, 0, 0.9f);
  }
}

std::string status(const Campaign &c) {
  if (c.stage == QuestStage::Done)
    return lang.statusBroken;
  if (c.stage == QuestStage::Road)
    return say(lang.statusFollow, tr(act(c).road), tr(act(c).town));
  const QuestDef *q = current(c);
  if (!q)
    return "";
  switch (c.stage) {
  case QuestStage::Offered:
    return say(lang.statusOffered, tr(act(c).elder), tr(act(c).town));
  case QuestStage::Ready:
    return say(lang.statusReady, tr(q->title), tr(act(c).elder));
  default:
    if (q->goal == Objective::Escort)
      return say(lang.statusEscort, tr(q->task), c.progress);
    if (q->goal == Objective::Defend)
      return c.holding ? say(lang.statusHolding, tr(q->task), c.progress, q->count) : say(lang.statusReachCamp, tr(q->task));
    return q->count > 1 ? say(lang.statusCount, tr(q->task), c.progress, q->count) : std::string(tr(q->task));
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
