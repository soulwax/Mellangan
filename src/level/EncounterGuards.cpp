#include "level/EncounterGuards.hpp"

#include "entity/enemy/Enemy.hpp"
#include "entity/enemy/ExpansionEnemy.hpp"
#include "entity/enemy/Ghost.hpp"
#include "entity/enemy/Wolf.hpp"
#include "entity/enemy/Burrower.hpp"
#include "entity/enemy/Necromancer.hpp"
#include "entity/enemy/ai/Plans.hpp"
#include "level/Level.hpp"

namespace {
template <class T> class EncounterGuard : public T {
public:
  template <class... Args>
  EncounterGuard(std::shared_ptr<ExpansionChallengeState> state, int slot,
                 Args &&...args)
      : T(std::forward<Args>(args)...), state(std::move(state)), slot(slot) {
  }

  ~EncounterGuard() override {
    if (!state->defeated[slot])
      state->escaped = true;
  }

protected:
  void die() override {
    state->defeated[slot] = true;
    T::die();
  }

private:
  std::shared_ptr<ExpansionChallengeState> state;
  int slot;
};

std::unique_ptr<Enemy> guard(EnemyKind kind, float hp, float damage,
                             std::shared_ptr<ExpansionChallengeState> state, int slot) {
  const EnemyType &type = Enemy::typeOf(kind);
  std::unique_ptr<Enemy> out;
  if (kind >= EnemyKind::MireLeech)
    out = std::make_unique<EncounterGuard<ExpansionEnemy>>(
        state, slot, type, hp, damage, int(kind) - int(EnemyKind::MireLeech));
  else if (kind == EnemyKind::Ghost)
    out = std::make_unique<EncounterGuard<Ghost>>(state, slot, type, hp, damage);
  else if (kind == EnemyKind::Wolf)
    out = std::make_unique<EncounterGuard<Wolf>>(state, slot, type, hp, damage);
  else if (kind == EnemyKind::Burrower)
    out = std::make_unique<EncounterGuard<Burrower>>(state, slot, type, hp, damage);
  else if (kind == EnemyKind::Necromancer)
    out = std::make_unique<EncounterGuard<Necromancer>>(state, slot, type, hp, damage);
  else
    out = std::make_unique<EncounterGuard<Enemy>>(state, slot, type, hp, damage);
  out->plan = planOf(kind);
  return out;
}

}

bool encounterOpenPosition(Level &level, double &x, double &y) {
  if (!level.blocks(x, y))
    return true;
  for (int r = 16; r <= 112; r += 16)
    for (int i = 0; i < 12; i++) {
      double a = TAU * i / 12;
      double nx = x + std::cos(a) * r, ny = y + std::sin(a) * r;
      if (!level.blocks(nx, ny)) {
        x = nx, y = ny;
        return true;
      }
    }
  return false;
}

int spawnEncounterGuards(Level &level, BiomeId biome, const Structure &site,
                         double x, double y, double heading,
                         std::shared_ptr<ExpansionChallengeState> state,
                         std::array<entt::entity, 6> &ids) {
  int room = std::max(0, 550 - int(level.count<Enemy>()));
  int defenders = std::min(room, 3 + int(site.seed % 3));
  if (defenders == 0)
    return 0;
  EnemyKind local[int(EnemyKind::Count)];
  int localCount = 0;
  uint32_t foes = BIOMES[biome]->foes;
  for (int k = 0; k < int(EnemyKind::Count); k++) {
    EnemyKind enemy = EnemyKind(k);
    if ((foes & foe(enemy)) && enemy != EnemyKind::Cyclops &&
        enemy != EnemyKind::Alpha && enemy != EnemyKind::Thief &&
        Enemy::typeOf(enemy).unlockAt <= level.time)
      local[localCount++] = enemy;
  }
  if (!localCount) {
    for (int k = 0; k < int(EnemyKind::Count); k++) {
      EnemyKind enemy = EnemyKind(k);
      if ((foes & foe(enemy)) && enemy != EnemyKind::Cyclops &&
          enemy != EnemyKind::Alpha && enemy != EnemyKind::Thief)
        local[localCount++] = enemy;
    }
    if (!localCount)
      local[localCount++] = EnemyKind::Rat;
  }
  for (int i = 0; i < defenders; i++) {
    EnemyKind enemy = local[(int(site.seed % uint32_t(localCount)) + i) % localCount];
    auto e = guard(enemy, 1 + std::min(level.time / 240, 3.0f),
                   1 + level.time / 900, state, i);
    double a = heading + TAU * i / defenders;
    e->x = x + std::cos(a) * 90;
    e->y = y + std::sin(a) * 90;
    encounterOpenPosition(level, e->x, e->y);
    e->aware = true;
    ids[i] = level.add(std::move(e))->id;
  }
  return defenders;
}


bool encounterGuardsDefeated(const ExpansionChallengeState &state, int count) {
  if (state.escaped || count <= 0)
    return false;
  for (int i = 0; i < count; i++)
    if (!state.defeated[i])
      return false;
  return true;
}

bool encounterGuardsPresent(Level &level, const ExpansionChallengeState &state,
                            const std::array<entt::entity, 6> &ids, int count) {
  for (int i = 0; i < count; i++)
    if (!state.defeated[i] && !level.get<Enemy>(ids[i]))
      return false;
  return true;
}
