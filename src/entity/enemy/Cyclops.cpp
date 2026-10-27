#include "entity/enemy/Cyclops.hpp"

#include "Game.hpp"
#include "entity/enemy/Enemy.hpp"
#include "entity/pickup/Chest.hpp"
#include "level/Level.hpp"
#include "particle/Explosion.hpp"
#include "particle/Spark.hpp"

void Cyclops::renderLight(Screen &s) { s.glow(xp, yp - 7 * scale, 130, {0.35f, 0.08f, 0.04f, 1}); }

void Cyclops::die() {
    Enemy::die();
    level->add<Chest>(x, y);
    Spark::burst(*level, x, bodyY(), type.blood, 40, 140, true);
    level->add<Explosion>(x, y, 10, 9);
    level->game.announce("THE CYCLOPS FALLS");
}
