#pragma once

#include <string>

class Game;

constexpr const char *RUN_HEADER =
    "when,version,world,mode,difficulty,hero,omen,seconds,dawn,cause,level,kills,shrines,metres,"
    "damage,weapons,picks,levels,biomes,heat\n";

std::string runRow(const Game &game, bool dawn);
