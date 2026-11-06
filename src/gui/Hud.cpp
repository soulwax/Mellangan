#include "gui/Hud.hpp"

#include "Game.hpp"
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

void Hud::announce(std::string text) { banners.push_back(std::move(text)); }

void Hud::clear() {
  bannerTime = 0;
  banners.clear();
}

void Hud::tick(bool running) {
  if (running)
    bannerTime = std::max(0.0f, bannerTime - DT);
  if (bannerTime <= 0 && !banners.empty())
    banner = std::move(banners.front()), banners.pop_front(), bannerTime = 3;
}

void Hud::render(float now, int xv, int yv) {
  time = now, xView = xv, yView = yv;
  draw();
}

void Hud::minimap() {
  Screen &s = game.screen;
  Player &p = *game.level->player;
  float u = s.u, small = std::max(1.0f, u - 1), side = 64 * u;
  float x = s.w - side - 4 * u, y = s.h - side - 6 * u;
  ChartView v{x, y, side, side, 2 * u, p.x / (16.0 * Chart::CELL),
              p.y / (16.0 * Chart::CELL)};
  Chart::border(s, v, u);
  game.level->chart.draw(s, *game.level, v, time);
  BiomeId b = world::biomeUnder(p.x, p.y);
  auto right = [&](std::string_view text, float ty, Color c) {
    s.text(text, s.w - Screen::textWidth(text, small) - 4 * u, ty, small, c);
  };
  right(FLAVOURS[b].note, y - 4 * u - 32 * small, col::gold * 0.8f);
  right(Chart::title(b), y - 2 * u - 16 * small, col::muted);
  right(game.level->sky.name(), y - 6 * u - 48 * small, Color{0.62f, 0.72f, 0.9f, 0.85f});
}

void Hud::experience() {
  Screen &s = game.screen;
  Player &p = *game.level->player;
  float u = s.u, bar = 6 * u;
  s.rect(0, 0, s.w, bar + 2 * u, col::panel);
  float fill = (s.w - 2 * u) * clampf(p.exp / p.expNext, 0, 1);
  s.rect(u, u, fill, bar, col::xp);
  s.rect(u, u, fill, u, {0.75f, 0.88f, 1, 0.6f});
}

void Hud::stats(float top) {
  Screen &s = game.screen;
  Level &l = *game.level;
  float u = s.u, small = std::max(1.0f, u - 1);
  auto right = [&](const std::string &text, float ty, float size, Color c) {
    s.text(text, s.w - Screen::textWidth(text, size) - 4 * u, ty, size, c);
  };
  right(fmt::format("LV {}", l.player->lvl), top, u, col::white);
  right(fmt::format("{} kills", l.kills), top + 18 * u, small, col::muted);
  if (l.streak >= 10)
    right(fmt::format("x{} STREAK", l.streak), top + 34 * u, small,
          col::gold.alpha(0.5f + 0.5f * std::min(1.0f, l.streakTime)));
}

void Hud::night(float top) {
  Screen &s = game.screen;
  Level &l = *game.level;
  Night &n = l.night;
  float u = s.u, small = std::max(1.0f, u - 1);
  bool cycle = !l.rules.endsAtDawn;
  std::string clock = cycle && n.dawn
                          ? fmt::format("DAY {}  DUSK {}", n.nights,
                                        clockText(std::max(0.0f, n.dayEnds - l.time)))
                      : cycle ? fmt::format("NIGHT {}  DAWN {}", n.nights, clockText(n.left()))
                              : fmt::format("DAWN {}", clockText(n.left()));
  s.textCenter(clock, s.w / 2, top, std::round(u * 1.5f), col::white);
  if (l.omen >= 0)
    s.textCenter(OMENS[l.omen].name, s.w / 2, top + 58 * u, small, OMENS[l.omen].tint);
  s.textCenter(fmt::format("{} shrine{} lit", n.lit, n.lit == 1 ? "" : "s"),
               s.w / 2, top + 26 * u, small, n.lit ? col::gold : col::muted);
  if (game.campaign && game.campaign->points > 0)
    s.textCenter(fmt::format("K  {} skill point{}", game.campaign->points,
                             game.campaign->points == 1 ? "" : "s"),
                 s.w / 2, top + 38 * u, small, col::gold.alpha(0.75f + 0.25f * std::sin(time * 4)));
  if (n.kindling > 0) {
    float kw = 120 * u, kx = (s.w - kw) / 2, ky = top + 46 * u;
    s.rect(kx, ky, kw, 4 * u, col::panel);
    s.rect(kx, ky, kw * n.kindling, 4 * u, col::gold);
  }
}

void Hud::loadout(float top) {
  Screen &s = game.screen;
  float u = s.u;
  int slot[2] = {0, 0};
  for (auto &up : game.level->player->upgrades) {
    if (!up->level)
      continue;
    int row = up->weapon ? 0 : 1;
    float x = 4 * u + slot[row]++ * 20 * u, y = top + row * 24 * u;
    s.rect(x, y, 18 * u, 18 * u, {0.07f, 0.06f, 0.11f, 0.7f});
    s.sprite(up->icon, x + u, y + u, 0, 0, u, u);
    if (up->rare || up->evolved) {
      s.frame(x, y, 18 * u, 18 * u, u, (up->evolved ? col::xp : col::gold).alpha(0.85f));
      continue;
    }
    for (int i = 0; i < up->level; i++)
      s.rect(x + u + i * 3.4f * u, y + 19 * u, 2 * u, 2 * u, col::gold);
  }
}

void Hud::statuses(float top) {
  Screen &s = game.screen;
  Player &p = *game.level->player;
  float u = s.u, small = std::max(1.0f, u - 1), sx = 4 * u, sy = top + 54 * u;
  for (int i = 0; i < int(StatusKind::Count); i++) {
    StatusKind kind = StatusKind(i);
    const StatusEffect &effect = p.status.effect(kind);
    if (effect.time <= 0)
      continue;
    std::string label = fmt::format("{} {:.1f}s", statusName(kind), effect.time);
    s.text(label, sx, sy, small, statusColor(kind));
    sx += Screen::textWidth(label, small) + 8 * u;
  }
}

void Hud::bossBar() {
  Screen &s = game.screen;
  Level &l = *game.level;
  float u = s.u, small = std::max(1.0f, u - 1);
  auto &ids = l.bossIds;
  ids.erase(std::remove_if(ids.begin(), ids.end(),
                           [&](entt::entity id) {
                             Enemy *e = l.get<Enemy>(id);
                             return e ? e->removed : !l.reg.valid(id);
                           }),
            ids.end());
  if (l.player->dead())
    return;
  Enemy *top = nullptr;
  for (entt::entity id : ids)
    if (Enemy *e = l.get<Enemy>(id); e && (!top || (e->def && !top->def)))
      top = e;
  if (!top)
    return;
  Enemy &b = *top;
  const char *name = b.name();
  float bw = 160 * u, bx = (s.w - bw) / 2, by = s.h - 22 * u;
  float k = clampf(b.hp / b.maxHp, 0, 1);
  s.textCenter(ids.size() > 1 ? fmt::format("{}  +{}", name, ids.size() - 1)
                              : std::string(name),
               s.w / 2, by - 15 * small, small, Color{1, 0.45f, 0.45f, 1});
  s.rect(bx - u, by - u, bw + 2 * u, 6 * u, col::panel);
  s.rect(bx, by, bw * k, 4 * u, Color{0.85f, 0.12f, 0.18f, 1});
  s.rect(bx, by, bw * k, u, Color{1, 0.5f, 0.5f, 0.6f});
  if (b.def)
    for (int i = 1; i < 3; i++)
      s.rect(bx + bw * i / 3 - u * 0.5f, by - u, u, 6 * u,
             (i > 2 - b.phase ? col::muted : col::white).alpha(0.8f));
}

void Hud::heartbeat() {
  Screen &s = game.screen;
  Player &p = *game.level->player;
  float u = s.u, low = 1 - clampf(p.hp / (p.maxHp * 0.3f), 0, 1);
  if (low <= 0 || p.dead())
    return;
  float rate = 4 + 6 * low, beat = std::pow(std::abs(std::sin(time * rate)), 6.0f);
  float a = (0.12f + 0.28f * beat) * low * game.settings.flash / 100.0f;
  for (int i = 0; i < 4; i++) {
    float band = (4 - i) * 6 * u, k = a * (i + 1) / 4;
    Color c = col::red.alpha(k * 0.6f);
    s.rect(0, 0, s.w, band, c);
    s.rect(0, s.h - band, s.w, band, c);
    s.rect(0, band, band, s.h - 2 * band, c);
    s.rect(s.w - band, band, band, s.h - 2 * band, c);
  }
}

void Hud::vitals() {
  Screen &s = game.screen;
  Player &p = *game.level->player;
  float u = s.u, small = std::max(1.0f, u - 1), hw = 80 * u, hy = s.h - 12 * u;
  if (const Hero *h = p.hero) {
    float box = 22 * u, bx = 4 * u, by = hy - box - 8 * u;
    const Skill *sk = p.skill;
    float wait = p.skillTime / std::max(0.01f, (sk ? sk->cooldown : h->cooldown) * p.haste);
    s.rect(bx, by, box, box, col::panel);
    s.sprite(sk ? sk->icon : h->sprite(), bx + 3 * u, by + 3 * u, 0, 0, u, u);
    if (wait > 0)
      s.rect(bx, by, box, box * wait, {0.02f, 0.02f, 0.05f, 0.7f});
    s.frame(bx, by, box, box, u, wait > 0 ? col::muted.alpha(0.4f) : h->tint.alpha(0.9f));
    s.text(sk ? sk->name : h->skill, bx + box + 5 * u, by + 2 * u, small,
           wait > 0 ? col::muted : h->tint);
    s.text(wait > 0 ? fmt::format("{:.0f}s", std::ceil(p.skillTime)) : std::string("SPACE"),
           bx + box + 5 * u, by + 2 * u + 16 * small, small, col::muted * 0.8f);
  }
  s.rect(4 * u, hy, hw, 6 * u, col::panel);
  s.rect(5 * u, hy + u, (hw - 2 * u) * clampf(p.hp / p.maxHp, 0, 1), 4 * u, col::red);
  s.text(fmt::format("{}/{}", int(std::ceil(p.hp)), int(p.maxHp)), 8 * u + hw,
         hy - 5 * small, small, col::white);
}

SDL_FRect Hud::bannerRect() const {
  const Screen &s = game.screen;
  float u = s.u, pw = Screen::textWidth(banner, u) + 14 * u;
  return {std::floor(s.w / 2 - pw / 2), s.h * 0.22f - 2 * u, pw, 22 * u};
}

void Hud::bannerBox() {
  Screen &s = game.screen;
  float u = s.u;
  float a = std::min({1.0f, bannerTime * 2, (3 - bannerTime) * 4});
  SDL_FRect r = bannerRect();
  s.rect(r.x, r.y, r.w, r.h, col::panel.alpha(0.72f * a));
  s.rect(r.x, r.y, r.w, u, col::gold.alpha(0.55f * a));
  s.rect(r.x, r.y + r.h - u, r.w, u, col::gold.alpha(0.55f * a));
  s.textCenter(banner, s.w / 2, r.y + 2 * u, u, col::gold.alpha(a));
}

void Hud::draw() {
  tags.clear();
  if (bannerTime > 0 && !game.menu)
    tags.push_back(bannerRect());
  float u = game.screen.u, top = 6 * u + 5 * u;
  if (!game.menu)
    lootLabels();
  experience();
  stats(top);
  night(top);
  landmarks();
  champions();
  merchant();
  handwork();
  questLog();
  compass();
  minimap();
  loadout(top);
  statuses(top);
  bossBar();
  heartbeat();
  vitals();
  if (bannerTime > 0 && !game.menu)
    bannerBox();
}
