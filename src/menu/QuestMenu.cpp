#include "menu/QuestMenu.hpp"

#include <spdlog/fmt/fmt.h>

#include <string>
#include <vector>

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "campaign/Campaign.hpp"
#include "campaign/Quests.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"

namespace {

std::vector<std::string> wrap(const std::string &text, float width, float size) {
  std::vector<std::string> lines;
  std::string line, word;
  auto flush = [&] {
    std::string next = line.empty() ? word : line + " " + word;
    if (!line.empty() && Screen::textWidth(next, size) > width)
      lines.push_back(line), line = word;
    else
      line = next;
    word.clear();
  };
  for (char ch : text)
    if (ch == ' ')
      flush();
    else
      word += ch;
  flush();
  if (!line.empty())
    lines.push_back(line);
  return lines;
}

} // namespace

void QuestMenu::tick() {
  Menu::tick();
  Input &in = game.input;
  Campaign &c = *game.campaign;
  if (in.inventory.clicked || in.escape.clicked) {
    Sound::select.play(0.5f, 0);
    return game.setMenu(nullptr);
  }
  if (in.build.clicked && t > 0.2f) {
    told = c.fortify(*game.level);
    if (!told.empty())
      return Sound::hurt.play(0.3f, 0);
    game.saveCampaign();
    return game.setMenu(nullptr);
  }
  if (in.restart.clicked && t > 0.2f) {
    if (!game.forget())
      Sound::hurt.play(0.3f, 0);
    return;
  }
  if (!in.confirm.clicked || t < 0.2f)
    return;
  if (c.stage == QuestStage::Done)
    c.awaken(*game.level);
  else if (c.stage == QuestStage::Offered)
    quests::accept(*game.level, c);
  else if (c.stage == QuestStage::Ready)
    quests::turnIn(*game.level, c);
  Sound::confirm.play(0.5f, 0);
  game.saveCampaign();
  game.setMenu(nullptr);
}

void QuestMenu::render(Screen &s) {
  Campaign &c = *game.campaign;
  const QuestDef *q = quests::current(c);
  float u = s.u, small = std::max(1.0f, u - 1), in = std::min(1.0f, t * 4);
  float w = 300 * u, h = 150 * u, x0 = (s.w - w) / 2, y0 = (s.h - h) / 2;
  s.rect(x0, y0, w, h, {0.04f, 0.04f, 0.08f, 0.94f * in});
  s.bracket(x0, y0, w, h, u);
  s.text(quests::act(c).elder, x0 + 12 * u, y0 + 8 * u, small, col::muted.alpha(in));
  std::string title = q && c.stage != QuestStage::Road ? q->title : quests::act(c).town;
  std::string body = c.stage == QuestStage::Road ? quests::status(c)
                     : !q ? "It is over. The dark is broken, and the shrines will hold."
                     : c.stage == QuestStage::Offered ? q->offer
                     : c.stage == QuestStage::Ready   ? q->thanks
                                                 : quests::status(c);
  std::string action = c.stage == QuestStage::Done ? "Enter begin Nightmare: every foe stronger"
                       : !q                        ? "Esc close"
                       : c.stage == QuestStage::Offered ? fmt::format("Enter accept   reward {}", quests::rewardText(c.quest))
                       : c.stage == QuestStage::Ready   ? fmt::format("Enter claim {}", quests::rewardText(c.quest))
                                                   : "Esc close   the task is not done yet";
  s.text(title, x0 + 12 * u, y0 + 24 * u, u, col::gold.alpha(in));
  float y = y0 + 48 * u;
  for (const std::string &line : wrap(body, w - 24 * u, small))
    s.text(line, x0 + 12 * u, y, small, col::white.alpha(0.9f * in)), y += 14 * u;
  s.text(action, x0 + 12 * u, y0 + h - 22 * u, small,
         (c.stage == QuestStage::Active ? col::muted : col::gold).alpha(in));
  int town = std::clamp(c.act, 1, int(c.towns().size())) - 1;
  if (!c.fortified(town))
    s.text(told.empty() ? fmt::format("B  raise a watchtower: {} gold, {} stone, {} wood",
                                      Campaign::TOWER_GOLD, Campaign::TOWER_STONE,
                                      Campaign::TOWER_WOOD)
                        : told,
           x0 + 12 * u, y0 + h - 58 * u, small, (told.empty() ? col::muted : col::red * 0.8f).alpha(in));
  std::string forget = fmt::format("R  forget your path and relearn it: {} gold", c.forgetCost());
  bool afford = game.level->player->gold >= c.forgetCost();
  s.text(forget, x0 + 12 * u, y0 + h - 40 * u, small,
         (afford ? col::muted : col::red * 0.8f).alpha(in));
}
