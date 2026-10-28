#include "level/ExpansionEncounter.hpp"

#include "Game.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/Enemy.hpp"
#include "entity/enemy/ai/Plans.hpp"
#include "entity/pickup/Chest.hpp"
#include "entity/pickup/Potion.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

namespace {
const char *const NAMES[] = {"AMBUSH CACHE", "GUARDED ALTAR", "ROAMING CARAVAN",
                            "CORRUPTED SPRING"};
const char *const TASKS[] = {"DEFEAT THE GUARDS", "HOLD THE ALTAR: 12S",
                            "ESCORT AND PROTECT", "SURVIVE THE VENOM: 10S"};
const Color COLORS[] = {col::gold, {0.7f, 0.6f, 1, 1}, {0.4f, 0.8f, 1, 1},
                        {0.5f, 1, 0.45f, 1}};

class EncounterGuard : public Enemy {
public:
  EncounterGuard(EnemyKind kind, float hp, float damage,
                 std::shared_ptr<ExpansionChallengeState> state, int slot)
      : Enemy(Enemy::typeOf(kind), hp, damage), state(std::move(state)), slot(slot) {
    plan = planOf(kind);
  }

  ~EncounterGuard() override {
    if (!state->defeated[slot])
      state->escaped = true;
  }

protected:
  void die() override {
    state->defeated[slot] = true;
    Enemy::die();
  }

private:
  std::shared_ptr<ExpansionChallengeState> state;
  int slot;
};

void openPosition(Level &level, double &x, double &y) {
  if (!level.blocks(x, y))
    return;
  for (int r = 16; r <= 112; r += 16)
    for (int i = 0; i < 12; i++) {
      double a = TAU * i / 12;
      double nx = x + std::cos(a) * r, ny = y + std::sin(a) * r;
      if (!level.blocks(nx, ny)) {
        x = nx, y = ny;
        return;
      }
    }
}
}

ExpansionChallenge::ExpansionChallenge(const Structure &site, int biome)
    : kind(int((site.seed >> 4) & 3)), biome(biome),
      heading((site.seed & 255) / 256.0 * TAU),
      state(std::make_shared<ExpansionChallengeState>()), site(site) {
  x = site.centerX();
  y = site.centerY();
  guardIds.fill(entt::null);
}

void ExpansionChallenge::onAdded() {
  openPosition(*level, x, y);
  if (level->blocks(x, y))
    return finish(false);
  int room = std::max(0, 550 - int(level->count<Enemy>()));
  defenders = std::min(room, 3 + int(site.seed % 3));
  if (defenders == 0)
    return finish(false);
  static const EnemyKind KINDS[] = {EnemyKind::Spider, EnemyKind::Knight,
                                     EnemyKind::Wolf, EnemyKind::Scorpion};
  for (int i = 0; i < defenders; i++) {
    auto e = std::make_unique<EncounterGuard>(
        KINDS[(kind + i) & 3], 1 + std::min(level->time / 240, 3.0f),
        1 + level->time / 900, state, i);
    double a = heading + TAU * i / defenders;
    e->x = x + std::cos(a) * 90;
    e->y = y + std::sin(a) * 90;
    openPosition(*level, e->x, e->y);
    e->aware = true;
    guardIds[i] = level->add(std::move(e))->id;
  }
  level->game.announce(std::string(NAMES[kind]) + ": " + TASKS[kind]);
}

bool ExpansionChallenge::complete() const {
  if (failed || state->escaped || defenders <= 0)
    return false;
  for (int i = 0; i < defenders; i++)
    if (!state->defeated[i])
      return false;
  return kind == 0 || (kind == 1 && progress >= 12) ||
         (kind == 2 && distance >= 120) || (kind == 3 && progress >= 10);
}

void ExpansionChallenge::finish(bool success) {
  if (removed)
    return;
  rewarded = success;
  failed = !success;
  if (success) {
    Player &p = *level->player;
    level->add<Chest>(x, y);
    if (kind == 2 || kind == 3)
      level->add<Potion>(x + 12, y);
    if (kind == 3)
      p.status.clear();
    Spark::burst(*level, x, y, COLORS[kind], 20, 100, true);
    level->game.announce(std::string(NAMES[kind]) + " SECURED");
  } else {
    level->game.announce(std::string(NAMES[kind]) + " LOST");
  }
  remove();
}

void ExpansionChallenge::tick() {
  if (removed)
    return;
  Player &p = *level->player;
  elapsed += DT;
  double px = p.x - x, py = p.y - y;
  double d2 = px * px + py * py;
  if (p.dead() || state->escaped || elapsed > 45 || d2 > 300 * 300)
    return finish(false);
  for (int i = 0; i < defenders; i++) {
    if (!state->defeated[i] && !level->get<Enemy>(guardIds[i]))
      return finish(false);
  }
  if (kind == 1) {
    progress = d2 < 68 * 68 ? progress + DT : std::max(0.0f, progress - DT * 0.5f);
  } else if (kind == 2) {
    if (d2 < 110 * 110 && distance < 120) {
      for (int i = 0; i < 8; i++) {
        double a = heading + i * PI * 0.25;
        double nx = x + std::cos(a) * 10 * DT;
        double ny = y + std::sin(a) * 10 * DT;
        if (level->blocks(nx + std::cos(a) * 8, ny + std::sin(a) * 8))
          continue;
        distance += std::hypot(nx - x, ny - y);
        x = nx, y = ny, heading = a;
        break;
      }
    }
    int pressure = 0;
    level->enemiesNear(x, y, 40, [&](Enemy &e) {
      double dx = e.x - x, dy = e.y - y;
      if (dx * dx + dy * dy < 28 * 28)
        pressure++;
    });
    integrity -= std::min(pressure, 4) * 7 * DT;
    if (integrity <= 0)
      return finish(false);
  } else if (kind == 3 && progress < 10 && d2 < 70 * 70) {
    progress += DT;
    p.afflict(StatusKind::Poison, 0.55f, 3);
  }
  if (complete())
    finish(true);
}

void ExpansionChallenge::render(Screen &s) {
  s.center(atlas::biomeProp(30 + biome), xp, yp - 6);
  s.color(COLORS[kind].alpha(0.35f));
  s.center(atlas::Ring, xp, yp, kind == 2 ? 1.2f : 2.0f, elapsed * 0.2f);
  s.color(col::white);
  float amount = kind == 0 ? 0 : kind == 1 ? progress / 12 :
                 kind == 2 ? float(distance / 120) : progress / 10;
  if (kind == 0)
    for (int i = 0; i < defenders; i++)
      amount += state->defeated[i] ? 1.0f / defenders : 0;
  s.rect(xp - 16, yp + 4, 32, 3, col::panel);
  s.rect(xp - 16, yp + 4, 32 * std::min(amount, 1.0f), 3, COLORS[kind]);
  if (kind == 2) {
    s.rect(xp - 16, yp + 8, 32, 2, col::panel);
    s.rect(xp - 16, yp + 8, 32 * std::max(integrity, 0.0f) / 100, 2, col::heal);
  }
}

void ExpansionChallenge::renderLight(Screen &s) {
  s.glow(xp, yp - 5, 100, COLORS[kind] * 0.12f);
}

bool expansionEncounter(Level &level, const Structure &site) {
  if (site.kind == StructureKind::Shrine)
    return false;
  int biome = int(world::biomeAt(site.x + site.w / 2, site.y + site.h / 2));
  if (biome < BIOME_SALT)
    return false;
  level.add<ExpansionChallenge>(site, biome - BIOME_SALT);
  return true;
}
