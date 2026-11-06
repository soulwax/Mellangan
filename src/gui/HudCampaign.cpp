#include "gui/Hud.hpp"

#include "Game.hpp"
#include "gui/Lang.hpp"
#include "entity/Merchant.hpp"
#include "entity/Workbench.hpp"
#include "campaign/Harvest.hpp"
#include "campaign/Elder.hpp"
#include "campaign/Quests.hpp"
#include "campaign/Townsfolk.hpp"
#include "campaign/Sentry.hpp"
#include "level/Prop.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "gfx/StatusVisual.hpp"
#include "hero/Hero.hpp"
#include "item/Upgrade.hpp"
#include "entity/pickup/ItemDrop.hpp"
#include "item/skill/Skill.hpp"
#include "level/Flavour.hpp"
#include "level/World.hpp"

void Hud::handwork() {
  Campaign *c = game.campaign.get();
  if (!c || game.menu)
    return;
  Level &l = *game.level;
  Screen &s = game.screen;
  float k = s.scale, u = s.u, small = std::max(1.0f, u - 1);
  if (c->building >= 0) {
    int tx, ty;
    harvest::aim(*l.player, tx, ty);
    std::string why = harvest::blocked(l, *c, c->building, tx, ty);
    Prop prop;
    world::pieceProp(PIECES[c->building].piece, tx, ty, prop);
    SDL_FPoint g = gui(tx * 16, ty * 16);
    float gx = g.x, gy = g.y;
    Color tint = why.empty() ? Color{0.6f, 1, 0.6f, 0.7f} : Color{1, 0.45f, 0.4f, 0.6f};
    s.color(tint);
    for (int i = 0; i < prop.count; i++)
      s.sprite(prop.parts[i].sprite, gx, gy + prop.parts[i].yo * k, 0, 0, k, k);
    s.color(col::white);
    s.frame(gx, gy, 16 * k, 16 * k, u, tint);
    plate(tx * 16 + 8.0, ty * 16 - 18.0,
          why.empty() ? game.input.prompt(say(lang.buildPrompt, PIECES[c->building].name)) : why,
          why.empty() ? col::gold : Color{1, 0.5f, 0.45f, 1});
    return;
  }
  if (c->delve.active) {
    std::string depth = c->delve.floor == 0
                            ? say(lang.caveIn, c->delve.inside(l))
                            : say(lang.delveFloor, c->delve.floor, Delve::FLOORS, c->delve.inside(l));
    s.textCenter(depth, s.w / 2, 70 * u, small, Color{0.9f, 0.75f, 0.5f, 0.9f});
  } else if (Structure mine; Delve::entrance(l, *c, mine)) {
    return plate(mine.centerX(), mine.centerY() - 26, game.input.prompt(lang.delveMine),
                 Color{0.9f, 0.75f, 0.5f, 1});
  }
  harvest::Target t;
  if (!harvest::target(l, *c, t))
    return;
  if (int cost = t.placed ? harvest::relight(c->edits.at(t.tx, t.ty)->piece) : -1; cost > 0)
    return plate(t.x, t.y - 22, game.input.prompt(say(lang.relight, cost)), col::gold);
  if (t.placed)
    return plate(t.x, t.y - 22, game.input.prompt(lang.takeDown), col::muted);
  auto it = c->chops.find(Edits::key(t.tx, t.ty));
  int hits = it == c->chops.end() ? 0 : it->second;
  std::string text = game.input.prompt(say(lang.gather, MATERIALS[int(t.yield->mat)].name));
  if (hits > 0)
    text += fmt::format("  {}/{}", hits, t.yield->hits);
  plate(t.x, t.y - 22, text, MATERIALS[int(t.yield->mat)].color);
}

void Hud::questLog() {
  Campaign *c = game.campaign.get();
  if (!c || game.menu)
    return;
  Level &l = *game.level;
  Screen &s = game.screen;
  float k = s.scale, u = s.u, small = std::max(1.0f, u - 1);
  if (!l.night.dawn) {
    int fell = 0;
    l.each<Sentry>([&](Sentry &k) { fell += k.kills; });
    Campaign::Town near = c->nearest(l.player->x, l.player->y);
    if (fell > 0 && std::hypot(near.x - l.player->x, near.y - l.player->y) < 320)
      s.textCenter(say(lang.bridgeHolds, fell), s.w / 2, 70 * u, small,
                   Color{0.85f, 0.88f, 0.95f, 0.85f});
  }
  l.each<Townsfolk>([&](Townsfolk &f) {
    if (f.talk <= 0 || f.removed || !inView(f.x, f.y, -8))
      return;
    std::string_view text = f.spoken;
    float a = std::min(1.0f, f.talk * 2) * std::min(1.0f, (Townsfolk::SAY - f.talk) * 4 + 0.2f);
    SDL_FPoint g = gui(f.x, f.y);
    float bx = g.x, by = g.y - 22 * k;
    float w = Screen::textWidth(text, small) + 8 * u, h = 16 * small + 4 * u;
    s.rect(bx - w / 2, by - h, w, h, Color{0.95f, 0.92f, 0.85f, 0.9f * a});
    s.rect(bx - u, by, 2 * u, 2 * u, Color{0.95f, 0.92f, 0.85f, 0.9f * a});
    s.text(text, bx - w / 2 + 4 * u, by - h + 2 * u, small, Color{0.15f, 0.1f, 0.12f, a});
  });
  std::vector<Campaign::Town> all = c->towns();
  for (size_t i = 0; i < all.size(); i++) {
    double gx = all[i].x, gy = std::floor(all[i].y / 16) * 16 + 5 * 16 + 34;
    if (!inView(gx, gy, -8))
      continue;
    SDL_FPoint g = gui(gx, gy);
    float sx = g.x, sy = g.y;
    std::string name = ACTS[i].town;
    float w = Screen::textWidth(name, small) + 10 * u, h = 16 * small + 4 * u;
    s.rect(sx - w / 2, sy - h, w, h, Color{0.12f, 0.08f, 0.05f, 0.85f});
    s.frame(sx - w / 2, sy - h, w, h, u, Color{0.55f, 0.38f, 0.2f, 0.9f});
    s.text(name, sx - w / 2 + 5 * u, sy - h + 2 * u, small, col::gold.alpha(0.9f));
  }
  std::string line = quests::status(*c);
  if (!line.empty())
    s.text(say(lang.questLine, line), 8 * u, s.h - 74 * u, small,
           (c->stage == QuestStage::Ready ? col::gold : col::white).alpha(0.85f));
  if (Elder *e = l.get<Elder>(c->elder); e && inView(e->x, e->y, -8)) {
    bool near = Elder::near(l) != nullptr, news = c->stage == QuestStage::Offered ||
                                                  c->stage == QuestStage::Ready;
    if (near || news) {
      plate(e->x, e->y - 22, near ? game.input.prompt(lang.talk) : "!",
            col::gold.alpha(0.75f + 0.25f * std::sin(time * 5)));
    }
  }
  if (const QuestDef *q = quests::current(*c);
      q && q->goal == Objective::Fetch && c->stage == QuestStage::Ready) {
    const Player &hero = *l.player;
    SDL_FPoint g = gui(hero.x, hero.y);
    float gx = g.x, gy = g.y - 30 * k;
    s.sprite(atlas::tile(90), gx - 4 * k, gy + std::sin(time * 3) * k, 0, 0, k * 0.5f, k * 0.5f);
  }
  l.each<Townsfolk>([&](Townsfolk &f) {
    if (f.mode != Townsfolk::Mode::Lost)
      return;
    Pin pin;
    if (inView(f.x, f.y, -8)) {
      SDL_FPoint g = gui(f.x, f.y);
      float gx = g.x, gy = g.y - 26 * k;
      s.textCenter(lang.help, gx, gy - 8 * small, small,
                   Color{1, 0.9f, 0.6f, 0.6f + 0.4f * std::sin(time * 6)});
    } else if (edge(f.x, f.y, pin)) {
      Color tone{1, 0.9f, 0.6f, 0.8f};
      arrow(pin, 7, 4, tone);
      Player &hero = *l.player;
      tag(say(lang.helpAt, std::hypot(f.x - hero.x, f.y - hero.y) / 16), pin, 18, tone);
    }
  });
  double tx, ty;
  if (c->stage != QuestStage::Active || !quests::marker(*c, tx, ty) || inView(tx, ty, -8))
    return;
  Pin pin;
  if (!edge(tx, ty, pin))
    return;
  Color tone{1, 0.55f, 0.3f, 0.85f};
  arrow(pin, 8, 5, tone);
  Player &p = *l.player;
  double xs = tx - p.x, ys = ty - p.y;
  tag(say(lang.questAt, std::sqrt(xs * xs + ys * ys) / 16), pin, 20, tone);
}
