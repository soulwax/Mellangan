#pragma once

#include <cassert>
#include <cmath>
#include <iostream>

#include "Game.hpp"
#include "entity/Player.hpp"
#include "entity/enemy/ExpansionEnemy.hpp"
#include "level/ExpansionEncounter.hpp"
#include "level/Level.hpp"

inline void testBiomeEncounters() {
  {
    Game game;
    game.level = std::make_unique<Level>(game);
    Level &level = *game.level;
    Player &p = *level.player;
    double x = p.x, y = p.y;
    EncounterObjective altar(16);
    for (int i = 0; i < 720; i++)
      assert(altar.tick(level, p, x, y, 0));
    assert(altar.progress > 11.9f && altar.fraction(0, 1) > 0.99f);
    while (!altar.complete())
      assert(altar.tick(level, p, x, y, 0));
    assert(altar.complete());
    float held = altar.progress;
    altar.tick(level, p, x, y, 10000);
    assert(altar.progress < held);
    EncounterObjective spring(48);
    assert(spring.tick(level, p, x, y, 0));
    assert(p.status.power(StatusKind::Poison) > 0);
    for (int i = 0; i < 600; i++)
      assert(spring.tick(level, p, x, y, 0));
    while (!spring.complete())
      assert(spring.tick(level, p, x, y, 0));
    assert(spring.complete());
    EncounterObjective caravan(32);
    assert(caravan.tick(level, p, x, y, 0));
    assert(caravan.distance > 0);
    double before = caravan.distance;
    assert(caravan.tick(level, p, x, y, 20000));
    assert(caravan.distance == before);
    caravan.distance = 120;
    assert(caravan.complete() && caravan.fraction(0, 1) == 1);
    caravan.integrity = 0;
    assert(!caravan.tick(level, p, x, y, 0));
  }
  for (int i = 0; i < BIOME_COUNT; i++) {
    Game game;
    game.level = std::make_unique<Level>(game);
    Level &level = *game.level;
    Player &p = *level.player;
    p.god = true;
    Structure site{StructureKind::Ruin, -1, -1, 2, 2, 0, 0, 0};
    ExpansionChallenge challenge(site, BiomeId(i));
    challenge.level = &level;
    challenge.x = p.x;
    challenge.y = p.y;
    challenge.objective.kind = 0;
    challenge.defenders = 1;
    challenge.state->defeated[0] = true;
    float might = p.might, hp = p.maxHp, armor = p.armor, speed = p.speed,
          magnet = p.magnet, regen = p.regen;
    float strength = i >= BIOME_SALT ? 2.0f : 1.0f;
    assert(biomeRewardText(BiomeId(i))[0]);
    challenge.tick();
    assert(challenge.removed && challenge.rewarded && !challenge.failed);
    switch (biomeReward(BiomeId(i))) {
    case BiomeReward::Might: assert(std::abs(p.might - might - 0.03f * strength) < 0.001f); break;
    case BiomeReward::Vitality: assert(std::abs(p.maxHp - hp - 5 * strength) < 0.001f); break;
    case BiomeReward::Armor: assert(std::abs(p.armor - armor - 0.25f * strength) < 0.001f); break;
    case BiomeReward::Speed: assert(std::abs(p.speed - speed - 2 * strength) < 0.001f); break;
    case BiomeReward::Magnet: assert(std::abs(p.magnet - magnet - 4 * strength) < 0.001f); break;
    case BiomeReward::Recovery: assert(std::abs(p.regen - regen - 0.1f * strength) < 0.001f); break;
    }
    might = p.might, hp = p.maxHp, armor = p.armor, speed = p.speed,
    magnet = p.magnet, regen = p.regen;
    challenge.tick();
    assert(p.might == might && p.maxHp == hp && p.armor == armor &&
           p.speed == speed && p.magnet == magnet && p.regen == regen);
    ExpansionChallenge failed(site, BiomeId(i));
    failed.level = &level;
    failed.state->escaped = true;
    failed.tick();
    assert(failed.failed && !failed.rewarded && p.might == might && p.maxHp == hp &&
           p.armor == armor && p.speed == speed && p.magnet == magnet && p.regen == regen);
  }
  {
    Game game;
    game.level = std::make_unique<Level>(game);
    Level &level = *game.level;
    level.player->god = true;
    level.night.dawn = true;
    level.time = 300;
    Structure site{StructureKind::Ruin, -1, -1, 2, 2, 0, 0, 0};
    auto *challenge = level.add<ExpansionChallenge>(site, BIOME_SALT);
    level.tick();
    assert(!challenge->removed && challenge->defenders >= 3);
    level.tick();
    bool foundSpecial = false;
    for (int i = 0; i < challenge->defenders; i++) {
      Enemy *e = level.get<Enemy>(challenge->guardIds[i]);
      assert(e && (BIOMES[BIOME_SALT]->foes & foe(EnemyKind(e->kind()))));
      foundSpecial |= dynamic_cast<ExpansionEnemy *>(e) != nullptr;
      e->hurt(100000, 0, 0, 0);
      assert(challenge->state->defeated[i]);
    }
    assert(foundSpecial && !challenge->state->escaped);
  }
  std::cout << "[TEST] Biome rewards, failure, local guards and specialized deaths: PASS" << std::endl;
}
