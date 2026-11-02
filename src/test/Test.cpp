#undef NDEBUG
#include <cassert>
#ifdef _WIN32
#include <stdlib.h>
#endif

#include "test/Tests.hpp"

#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>
#include "Game.hpp"
#include "entity/Player.hpp"
#include "entity/Pool.h"
#include "entity/effect/GlaiveThrust.hpp"
#include "entity/effect/IncenseCloud.hpp"
#include "particle/DeathEffect.hpp"
#include "particle/VisualDetail.hpp"
#include "gfx/StatusVisual.hpp"
#include "entity/enemy/Enemy.hpp"
#include "item/skill/Skill.hpp"
#include "item/weapon/Recipe.hpp"
#include "item/talent/ExpansionTalents.hpp"
#include "entity/pickup/Gem.hpp"
#include "entity/Ability.hpp"
#include "level/Flavour.hpp"
#include "entity/enemy/Boss.hpp"
#include "entity/Trait.hpp"
#include "entity/pickup/Chest.hpp"
#include "entity/enemy/Renegade.hpp"
#include "entity/enemy/Hazard.hpp"
#include "entity/projectile/Arrow.hpp"
#include "entity/projectile/Bomb.hpp"
#include "entity/effect/Sweep.hpp"
#include "entity/effect/Strike.hpp"
#include "entity/projectile/Slash.hpp"
#include "entity/projectile/ChakramDisc.hpp"
#include "level/Level.hpp"
#include "level/Noise.h"
#include "level/World.hpp"
#include "level/Prop.hpp"
#include "level/Structure.hpp"
#include "item/Upgrade.hpp"
#include "hero/Hero.hpp"
#include "menu/LevelUpMenu.hpp"
#include "particle/Spark.hpp"
#include "particle/TextParticle.hpp"
#include "particle/debris/Debris.hpp"

int runTests() {
#ifdef _WIN32
  _set_error_mode(_OUT_TO_STDERR);
  _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
  std::cout << "[TEST] Running memory leak & stability tests..." << std::endl;
  testPoolAllocAndFree();
  std::cout << "[TEST] Pool alloc/free & edge cases: PASS" << std::endl;
  testPoolConcurrency();
  std::cout << "[TEST] Pool multithreaded concurrency: PASS" << std::endl;
  testStatuses();
  testStatusDamage();
  testNewStatuses();
  testNewPayloads();
  testShapeForms();
  testItems();
  testLootDrops();
  testInventory();
  testEquipStats();
  testAllies();
  testSupportArts();
  testTalents();
  testRelicsAndUniques();
  testExpansionEnemies();
  testExpansionContent();
  testArsenal();
  testBiomeTransitions();
  testBiomeEncounters();
  testBalance();
  testExperienceCurve();
  testAutomaticLevelUp();
  testCombatReadability();
  testGuiChoices();
  testEnemyDeathEffects();
  testSidedAbilities();
  testSidedEffects();
  testFoeKits();
  testArtsBothSides();
  testGeneralSkills();
  testRunVariety();
  testNewTraits();
  testChampionTraits();
  testClassKits();
  testSpawnShore();
  testHills();
  testNewFoes();
  testGreenwoodFoes();
  testRunRules();
  std::cout << "[TEST] Automatic level-up and treasure choices: PASS" << std::endl;
  std::cout << "[TEST] Status stacking, expiry, immunity, damage credit and cleanup: PASS"
            << std::endl;
  testLevelLifecycleAndSimulation();
  std::cout << "[TEST] Level lifecycle, multithreaded AI & entity cleanup: PASS"
            << std::endl;
  pool_shutdown();
  std::cout << "[TEST] Pool shutdown & slab reclamation: PASS" << std::endl;
  std::cout << "[TEST] ALL TESTS PASSED." << std::endl;
  return 0;
}
