#include "Game.hpp"

#include <spdlog/spdlog.h>

#include <cmath>

#include "gui/Lang.hpp"
#include "campaign/Campaign.hpp"
#include "campaign/Delve.hpp"
#include "campaign/Harvest.hpp"
#include "campaign/Quests.hpp"
#include "entity/Player.hpp"
#include "hero/Hero.hpp"
#include "item/Upgrade.hpp"
#include "level/Level.hpp"
#include "level/World.hpp"

void Game::beginCampaign() {
  dawnRules = rules;
  campaign = std::make_unique<Campaign>();
  std::string text = saves.dir().empty() ? std::string() : saves.read("campaign.txt", Campaign::valid);
  bool fresh = text.empty() || !Campaign::load(text, *campaign);
  if (fresh) {
    *campaign = Campaign{};
    campaign->hero = start.hero >= 0 && start.hero < HERO_COUNT ? start.hero : codex.start;
    campaign->hardcore = hardcoreNext;
    for (int i = 0; i < 8; i++)
      campaign->world += CODE_CHARS[rng.next() % (sizeof CODE_CHARS - 1)];
  }
  codex.start = campaign->hero;
  worldMode = WorldMode::Custom, seedCode = campaign->world;
  rules = campaign->rules();
  loadBalance();
  world::openCaves(true);
  world::enter(world::Layer(campaign->layer));
  build();
  campaign->enter(*level);
  if (dev) {
    for (int &m : level->player->materials)
      m += start.mats;
    level->player->gold += std::max(0, start.merchant);
  }
  for (int act = campaign->act + 1; dev && act <= std::min(start.act, ACT_COUNT); act++) {
    double rx, ry;
    if (!quests::road(act, rx, ry))
      break;
    campaign->act = act, campaign->quest = (act - 1) * QUESTS_PER_ACT;
    campaign->settle(*level, rx, ry);
    campaign->stage = QuestStage::Offered;
    level->player->x = rx, level->player->y = ry + 20;
  }
  if (dev && start.quest >= 0 && start.quest < QUEST_COUNT) {
    campaign->quest = start.quest, campaign->stage = QuestStage::Offered;
    quests::accept(*level, *campaign);
    Campaign::Town h = campaign->home();
    if (QUESTS[start.quest].goal == Objective::Escort)
      level->player->x = h.x + 20, level->player->y = std::floor(h.y / 16) * 16 + 7 * 16;
    else if (campaign->qsite)
      level->player->x = campaign->qx, level->player->y = campaign->qy + 90;
  }
  if (Structure mine; dev && start.delve &&
                      world::nearestStructure(level->player->x, level->player->y, 60, mine,
                                              [](const Structure *s, const void *) {
                                                return s->kind == StructureKind::Mine;
                                              }))
    level->player->x = mine.centerX(), level->player->y = mine.centerY() + 24;
  world::CaveCell cell{};
  bool caved = false;
  int ox = int(std::floor(level->player->x / 16 / world::CAVE_CELL)),
      oy = int(std::floor(level->player->y / 16 / world::CAVE_CELL));
  for (int r = 0; dev && start.cave && world::layer() == world::Layer::Surface && r < 12 && !caved; r++)
    for (int cy = oy - r; cy <= oy + r && !caved; cy++)
      for (int cx = ox - r; cx <= ox + r && !caved; cx++)
        caved = (std::abs(cx - ox) == r || std::abs(cy - oy) == r) && world::caveCell(cx, cy, cell);
  if (caved)
    travel(world::Layer::Caves, cell.mx[0] * 16 + 8.0, cell.my[0] * 16 + 12.0);
  snapCamera();
  setMenu(nullptr);
  autosave = Campaign::AUTOSAVE;
  saveCampaign();
  spdlog::info("campaign: {} world {} as hero {}, day {}", fresh ? "new" : "loaded",
               campaign->world, campaign->hero, campaign->day);
  announce(fresh ? lang.hearthwickHolds : lang.hearthwickStands);
}

bool Game::campaignSaved() const {
  if (saves.dir().empty())
    return false;
  Campaign probe;
  return Campaign::load(saves.read("campaign.txt", Campaign::valid), probe);
}

void Game::saveCampaign() {
  if (!campaign || !level || level->player->dead())
    return;
  campaign->capture(*level);
  if (writes())
    saves.write("campaign.txt", campaign->save());
}

bool Game::forget() {
  if (!campaign || !level)
    return false;
  Player &p = *level->player;
  int cost = campaign->forgetCost();
  if (p.gold < cost)
    return false;
  campaign->capture(*level);
  int before = campaign->spent(), relics = 0;
  std::vector<std::pair<std::string, int>> keep;
  for (const auto &[name, lvl] : campaign->upgrades) {
    Upgrade *u = p.upgrade(name);
    if (u && u->rare)
      keep.emplace_back(name, lvl), relics += lvl;
    else if (u && u->evolved)
      before++;
  }
  campaign->upgrades = keep, campaign->active.clear();
  campaign->gold -= cost;
  campaign->delve.abort(*level, *campaign);
  build();
  campaign->enter(*level);
  campaign->capture(*level);
  campaign->points += std::max(0, before - campaign->spent());
  snapCamera();
  setMenu(nullptr);
  saveCampaign();
  spdlog::info("campaign: forgot the path for {} gold, {} points to spend", cost,
               campaign->points);
  announce(say(lang.pathForgotten, campaign->points));
  return true;
}

void Game::endCampaign() {
  if (!campaign)
    return;
  saveCampaign();
  campaign->leave();
  campaign.reset();
  rules = dawnRules;
  world::openCaves(false);
  world::enter(world::Layer::Surface);
}

void Game::place() {
  int tx, ty, piece = campaign->building;
  harvest::aim(*level->player, tx, ty);
  std::string why = harvest::blocked(*level, *campaign, piece, tx, ty);
  if (why.empty() && harvest::build(*level, *campaign, piece, tx, ty)) {
    if (!harvest::affordable(*level->player, piece))
      campaign->building = -1;
    saveCampaign();
  } else {
    announce(why);
  }
}

void Game::travel(world::Layer to, double x, double y) {
  campaign->capture(*level);
  campaign->x = x, campaign->y = y, campaign->layer = int(to);
  campaign->leave();
  rules = campaign->rules();
  world::enter(to);
  build();
  campaign->enter(*level);
  snapCamera();
  saveCampaign();
  spdlog::info("campaign: {} at tile ({}, {})", to == world::Layer::Surface ? "climbed out" : "went below",
               int(std::floor(x / 16)), int(std::floor(y / 16)));
  announce(to == world::Layer::Surface
               ? std::string(lang.caveSurface)
               : tr(CAVES[int(world::caveBiomeAt(world::tileOf(x), world::tileOf(y)))].name));
}

void Game::gather() {
  if (int mx, my; input.gather.clicked && !campaign->delve.active &&
                  world::caveMouthNear(level->player->x, level->player->y, mx, my))
    travel(world::layer() == world::Layer::Surface ? world::Layer::Caves : world::Layer::Surface,
           mx * 16 + 8.0, my * 16 + 12.0);
  else if (Structure mine; input.gather.clicked && !campaign->delve.active &&
                      Delve::entrance(*level, *campaign, mine))
    campaign->delve.start(*level, *campaign, mine);
  else if (input.gather.clicked || (input.gather.down && (gatherTime -= DT) <= 0)) {
    harvest::strike(*level, *campaign);
    gatherTime = GATHER_REPEAT;
  }
}

void Game::keep() {
  campaign->tick(*level);
  if ((autosave -= DT) <= 0)
    autosave = Campaign::AUTOSAVE, saveCampaign();
}

void Game::bankLevels() {
  Player &p = *level->player;
  if (p.pendingLevels <= p.pendingTreasures)
    return;
  int gained = p.pendingLevels - p.pendingTreasures;
  campaign->points += gained, p.pendingLevels = p.pendingTreasures;
  celebrate();
  announce(say(gained == 1 ? lang.levelPoint : lang.levelPoints, p.lvl, gained));
}
