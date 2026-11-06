#include "gui/Hud.hpp"

#include "Game.hpp"
#include "entity/Player.hpp"
#include "level/Level.hpp"

namespace {
struct Lesson {
  bool (*ready)(const Level &l);
  const char *text;
};

const Lesson LESSONS[] = {
    {[](const Level &l) { return l.time > 1; },
     "WASD/Arrows move   your weapons strike on their own"},
    {[](const Level &l) { return l.time > 8 && l.player->lvl == 1; },
     "Walk over the green gems to gain experience"},
    {[](const Level &l) { return l.player->lvl >= 2; },
     "Every level brings an upgrade   a weapon at level 5 can evolve"},
    {[](const Level &l) { return l.time > 15; }, "SPACE class skill   it recharges on its own"},
    {[](const Level &l) { return l.rules.endsAtDawn && l.time > 25; },
     "Survive until the clock at the top reaches dawn"},
    {[](const Level &l) { return l.rules.endsAtDawn && l.night.hasBeacon && l.time > 40; },
     "Follow the gold arrow to a shrine and stand in its ring   each lit shrine brings dawn sooner"},
    {[](const Level &l) { return l.time > 90; }, "Tab map   the world is charted as you walk"}};
constexpr float SHOW = 7, GAP = 3;
} // namespace

void Hud::teach() {
  if ((hintTime -= DT) > 0 || !game.level)
    return;
  if (hintTime > -GAP)
    return hint.clear();
  for (size_t i = 0; i < sizeof LESSONS / sizeof *LESSONS; i++) {
    uint32_t bit = 1u << i;
    if ((game.codex.hints & bit) || !LESSONS[i].ready(*game.level))
      continue;
    game.codex.hints |= bit;
    hint = LESSONS[i].text, hintTime = SHOW;
    return;
  }
}

void Hud::tutor() {
  if (hint.empty() || hintTime <= 0)
    return;
  Screen &s = game.screen;
  float u = s.u, a = std::min({1.0f, (SHOW - hintTime) * 3, hintTime * 2});
  std::string text = game.input.prompt(hint);
  float w = std::min(Screen::textWidth(text, u), s.w - 40 * u) + 16 * u, h = 16 * u + 8 * u;
  float x = std::floor((s.w - w) / 2), y = std::floor(s.h * 0.78f);
  s.rect(x, y, w, h, col::panel.alpha(0.85f * a));
  s.rect(x, y, 2 * u, h, col::gold.alpha(0.8f * a));
  s.textCenter(Screen::fit(text, w - 16 * u, u), s.w / 2, y + 4 * u, u, col::white.alpha(a));
}
