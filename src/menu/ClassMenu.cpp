#include "menu/ClassMenu.hpp"

#include "Game.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "item/Upgrade.hpp"
#include "menu/HeroCard.hpp"
#include "menu/TitleMenu.hpp"
#include "meta/Heat.hpp"

namespace {
uint32_t dayHash(const std::string &code) {
  uint32_t h = 2166136261u;
  for (char c : code)
    h = (h ^ uint8_t(c)) * 16777619u;
  return h;
}
} // namespace

bool ClassMenu::daily() const { return !campaign && game.worldMode == WorldMode::Daily; }

ClassMenu::ClassMenu(Game &game) : Menu(game) {
  game.heat = std::min(game.heat, game.codex.heat);
  pick = game.codex.start;
  if (pick < 0 || pick >= HERO_COUNT || !HEROES[pick]->met(game.codex))
    pick = 0;
  grow[pick] = 1;
}

SDL_FRect ClassMenu::slot(int i) const {
  const Screen &s = game.screen;
  float u = s.u, w = std::min(s.w * 0.54f, 640 * u), step = w / 6;
  float x = s.w * 0.69f - w / 2 + step * (i % 6);
  return {x, s.h * (i < 6 ? 0.25f : 0.53f), step, s.h * 0.25f};
}

void ClassMenu::choose(int i) {
  i = (i % HERO_COUNT + HERO_COUNT) % HERO_COUNT;
  if (i == pick)
    return;
  pick = i, since = 0;
  Sound::select.play(0.5f, 0);
  if (HEROES[pick]->met(game.codex)) {
    game.codex.start = pick;
    game.level->player->tile = HEROES[pick]->tile;
  }
}

void ClassMenu::begin() {
  if (!HEROES[pick]->met(game.codex))
    return Sound::hurt.play(0.4f, 0);
  game.codex.start = pick;
  Sound::confirm.play(0.6f, 0);
  if (campaign)
    return game.beginCampaign();
  game.newGame(false);
}

void ClassMenu::tick() {
  Menu::tick();
  since += DT;
  game.level->player->animT += DT;
  for (int i = 0; i < HERO_COUNT; i++)
    grow[i] += ((i == pick ? 1.0f : 0.0f) - grow[i]) * damp(12, DT);
  Input &in = game.input;
  if (in.escape.clicked) {
    Sound::select.play(0.5f, 0);
    return game.setMenu(std::make_unique<TitleMenu>(game));
  }
  if (campaign && in.restart.clicked) {
    game.hardcoreNext = !game.hardcoreNext;
    return Sound::select.play(0.5f, 0, game.hardcoreNext ? 0.7f : 1.2f);
  }
  if (daily()) {
    uint32_t h = dayHash(game.seedCode);
    int open[HERO_COUNT], n = 0;
    for (int i = 0; i < HERO_COUNT; i++)
      if (HEROES[i]->met(game.codex))
        open[n++] = i;
    if (n && pick != open[h % n])
      pick = open[h % n], since = 0, game.level->player->tile = HEROES[pick]->tile;
    game.heat = int(h >> 8 & 3);
    if (in.confirm.clicked && t > 0.2f)
      return begin();
    return;
  }
  if (!campaign && (in.up.clicked || in.down.clicked)) {
    int h = std::clamp(game.heat + (in.up.clicked ? 1 : -1), 0, std::min(HEAT_MAX, game.codex.heat));
    if (h == game.heat)
      return Sound::hurt.play(0.3f, 0);
    game.heat = h;
    return Sound::select.play(0.5f, 0, 1 + 0.05f * h);
  }
  if (in.left.clicked)
    return choose(pick - 1);
  if (in.right.clicked)
    return choose(pick + 1);
  queued |= in.confirm.clicked;
  if (queued && t > 0.2f)
    return queued = false, begin();
  SDL_FPoint m{in.mouseX, in.mouseY};
  for (int i = 0; i < HERO_COUNT; i++) {
    SDL_FRect r = slot(i);
    if (!in.mouseClicked || !SDL_PointInRectFloat(&m, &r))
      continue;
    if (i == pick)
      return begin();
    return choose(i);
  }
}

ClassMenu::Pose ClassMenu::pose() const {
  Pose p;
  p.cycle = fract(since / 2.2f);
  p.wind = smoothstep01(p.cycle / 0.5f);
  p.strike = smoothstep01((p.cycle - 0.5f) / 0.12f);
  p.recover = smoothstep01((p.cycle - 0.62f) / 0.38f);
  p.lunge = p.strike * (1 - p.recover);
  return p;
}

void ClassMenu::renderAura(Screen &s, const Hero &k, float cx, float foot,
                           float height, const Pose &p, float in) const {
  float u = s.u;
  float pulse = 0.75f + 0.25f * std::sin(t * 3);
  s.color(k.tint.alpha(0.22f * pulse * in));
  s.center(atlas::Glow, cx, foot - height * 0.5f, height * 2.6f / atlas::Glow.w);
  s.color(k.tint.alpha((0.25f + 0.5f * p.wind * (1 - p.recover)) * in));
  s.center(atlas::Ring, cx, foot, (16 + 14 * p.wind) * u * 2 / atlas::Ring.w, t * 2.5f);
  s.center(atlas::Ring, cx, foot, (10 + 8 * p.wind) * u * 2 / atlas::Ring.w, -t * 3.5f);
  for (int m = 0; m < 16; m++) {
    float rise = fract(t * 0.35f + m * 0.137f);
    float mx = cx + std::sin(m * 2.3f + t * 1.3f) * (14 + 6 * (m % 3)) * u;
    float my = foot - rise * height * 1.3f;
    float size = (m % 2 ? 1.5f : 1) * u;
    s.rect(mx, my, size, size, k.tint.alpha((1 - rise) * 0.8f * in));
  }
}

void ClassMenu::renderSwing(Screen &s, const Hero &k, const Upgrade &up, float hx,
                            float hy, float scale, const Pose &p, float in) const {
  float u = s.u;
  float swing = -1.1f * p.wind * (1 - p.strike) + 1.5f * p.lunge;
  s.sprite(up.icon, hx, hy, up.icon.w * 0.5f, float(up.icon.h), scale * 0.85f,
           scale * 0.85f, swing);
  if (p.cycle <= 0.5f || p.cycle >= 0.85f)
    return;
  float burst = smoothstep01((p.cycle - 0.5f) / 0.35f);
  for (int m = 0; m < 12; m++) {
    float a = m * TAU / 12 + t;
    float d = burst * 34 * u;
    s.rect(hx + std::cos(a) * d, hy - 6 * u + std::sin(a) * d, 1.5f * u, 1.5f * u,
           k.tint.alpha((1 - burst) * in));
  }
  s.color(k.tint.alpha(0.5f * (1 - burst) * in));
  s.center(atlas::Glow, hx, hy - 6 * u, (10 + 30 * burst) * u / atlas::Glow.w);
  s.color(col::white);
}

void ClassMenu::renderHero(Screen &s, int i, const Pose &p, float in) const {
  const Hero &k = *HEROES[i];
  float u = s.u, small = std::max(1.0f, u - 1);
  SDL_FRect r = slot(i);
  bool open = k.met(game.codex), on = i == pick, hot = on && open;
  float g = grow[i], cx = r.x + r.w / 2, foot = r.y + r.h * 0.72f;
  float scale = (3 + 2.5f * g) * u * 0.5f + u;
  Sprite body = k.sprite();

  if (hot)
    renderAura(s, k, cx, foot, body.h * scale, p, in);
  else {
    s.color((open ? col::gold : col::muted).alpha((0.12f + 0.3f * g) * in));
    s.center(atlas::Ring, cx, foot, 12 * u * 2 / atlas::Ring.w);
  }
  s.color(col::white);

  float bob = std::sin(t * 2 + i) * u * (on ? 1 : 0.4f);
  float lean = hot ? -0.12f * p.wind * (1 - p.strike) + 0.1f * p.lunge : 0;
  float push = hot ? (p.lunge * 8 - p.wind * (1 - p.strike) * 2) * u : 0;
  float swell = 1 + (hot ? 0.07f * p.lunge : 0);
  s.color(open ? col::white.alpha((0.5f + 0.5f * g) * in)
               : Color{0.05f, 0.04f, 0.09f, in});
  s.sprite(body, cx + push, foot + bob, body.w * 0.5f, float(body.h),
           scale * swell, scale * swell, lean);
  s.color(col::white);

  if (hot)
    if (const Upgrade *up = game.level->player->upgrade(k.weapon))
      renderSwing(s, k, *up, cx + push + body.w * scale * 0.42f,
                  foot + bob - body.h * scale * 0.5f, scale, p, in);
  Color label = on ? (open ? k.tint : col::muted)
                   : open ? col::white * 0.75f
                          : col::muted * 0.6f;
  s.textCenter(open || on ? k.name : "???", cx, foot + 7 * u, small, label.alpha(in));
}

void ClassMenu::render(Screen &s) {
  float u = s.u, small = std::max(1.0f, u - 1), in = std::min(1.0f, t * 3);
  s.rect(0, 0, s.w, s.h, {0.015f, 0.012f, 0.03f, 0.9f * in});
  float edge = std::min(s.w, s.h) * 0.025f;
  for (int i = 0; i < 10; i++)
    s.frame(i * edge, i * edge, s.w - 2 * i * edge, s.h - 2 * i * edge, edge,
            {0, 0, 0, 0.42f * (1 - i / 10.0f) * in});
  s.textCenter("CHOOSE YOUR CLASS", s.w * 0.69f, s.h * 0.1f, small,
               col::muted.alpha(in));
  Pose p = pose();
  for (int i = 0; i < HERO_COUNT; i++)
    renderHero(s, i, p, in);
  HeroCard(game, s, pick, t, in).render(s);
  s.textCenter(game.input.prompt(daily() ? "Enter begin   Esc back"
                                 : !campaign && game.codex.heat > 0
                                     ? "Left/Right choose   Up/Down heat   Enter begin   Esc back"
                                     : "Left/Right choose   Enter begin   Esc back"),
               s.w * 0.69f,
               s.h - 22 * u, u, col::muted.alpha(in));
  if (daily()) {
    s.textCenter(Screen::fit(fmt::format("DAILY  {} AT HEAT {}   the same run for everyone today",
                                         HEROES[pick]->name, game.heat),
                             s.w * 0.6f, small),
                 s.w * 0.69f, s.h - 44 * u, small, col::gold.alpha(in));
  } else if (!campaign) {
    Color hot = Color{1, 0.82f - 0.06f * game.heat, 0.32f - 0.03f * game.heat, 1};
    std::string line = game.codex.heat == 0 ? std::string("HEAT unlocks at your first dawn")
                       : game.heat == 0
                           ? game.input.prompt("HEAT 0   Up/Down to raise the stakes")
                           : fmt::format("HEAT {}   {}", game.heat, heatNames(game.heat));
    s.textCenter(Screen::fit(line, s.w * 0.6f, small), s.w * 0.69f, s.h - 44 * u, small,
                 (game.heat ? hot : col::muted).alpha(in));
  }
  if (campaign)
    s.textCenter(game.hardcoreNext ? game.input.prompt("R  HARDCORE: one life, death deletes the hero")
                                   : game.input.prompt("R  HARDCORE: off"),
                 s.w * 0.69f, s.h - 44 * u, small,
                 (game.hardcoreNext ? col::red : col::muted).alpha(in));
}
