#include "level/Level.hpp"

#include <cmath>

#include "Game.hpp"
#include "entity/Player.hpp"
#include "gui/Lang.hpp"
#include "hero/Affliction.hpp"
#include "level/World.hpp"
#include "particle/Spark.hpp"

namespace {

constexpr Color CHANGE{0.72f, 0.5f, 1, 1};

} // namespace

void Level::whispers() {
  if (!rules.mutate || player->dead())
    return;
  if (offer.tier) {
    offer.time -= DT;
    if (offer.tier == 1 &&
        (offer.time <= 0 || std::hypot(player->x - offer.x, player->y - offer.y) > mutation::WALK_AWAY))
      offer = {}, whisperCool = mutation::WHISPER_COOL;
    else if (offer.tier == 2 && offer.time <= 0)
      accept(offer.b != Mutation::Count && rng.chance(0.5f) ? 1 : 0);
    return;
  }
  if ((whisperCool -= DT) > 0)
    return;
  whisperCool = 0;
  if (!player->changeling || world::layer() != world::Layer::Surface)
    return;
  Mutation m = mutation::whisper(world::biomeUnder(player->x, player->y));
  if (m == Mutation::Count || mutation::has(*player, m)) {
    linger = 0;
    return;
  }
  if ((linger += DT) < mutation::LINGER)
    return;
  linger = 0;
  offer = {m, Mutation::Count, 1, mutation::OFFER_LIFE, player->x, player->y};
  game.announce(say(lang.whisperFelt, tr(MUTATIONS[int(m)].name)));
}

void Level::surgeFrom(CaveBiome biome) {
  if (!rules.mutate || !player->changeling || offer.tier)
    return;
  Mutation a, b;
  mutation::surge(biome, a, b);
  if (a != Mutation::Count && mutation::has(*player, a))
    a = Mutation::Count;
  if (b != Mutation::Count && mutation::has(*player, b))
    b = Mutation::Count;
  if (a == Mutation::Count)
    std::swap(a, b);
  if (a == Mutation::Count)
    return;
  offer = {a, b, 2, mutation::SURGE_LIFE, player->x, player->y};
  game.announce(lang.surgeFelt);
}

bool Level::upheaval() {
  if (!rules.mutate || !rng.chance(mutation::UPHEAVAL_ODDS))
    return false;
  if (!player->changeling) {
    Affliction a = Affliction(rng.irange(0, int(Affliction::Count) - 1));
    if (affliction::has(*player, a))
      return false;
    affliction::grant(*player, a);
    game.announce(say(lang.afflictCaught, tr(AFFLICTIONS[int(a)].name)));
    return true;
  }
  Mutation m = mutation::random(rng.next(), false, player->mutations);
  if (m == Mutation::Count)
    return false;
  mutation::grant(*player, m);
  if (rng.chance(0.5f))
    mutation::grant(*player, mutation::random(rng.next(), true, player->mutations));
  suspect(mutation::FORCED_SCRUTINY);
  Spark::burst(*this, player->x, player->y - 8, CHANGE, 16, 70, false);
  game.announce(say(lang.upheavalFelt, tr(MUTATIONS[int(m)].name)));
  return true;
}

void Level::accept(int choice) {
  Mutation m = choice ? offer.b : offer.a;
  if (m == Mutation::Count)
    m = offer.a;
  if (offer.tier >= 2)
    suspect(mutation::FORCED_SCRUTINY);
  mutation::grant(*player, m);
  Spark::burst(*this, player->x, player->y - 8, CHANGE, 12, 60, false);
  game.announce(say(lang.changed, tr(MUTATIONS[int(m)].name)));
  offer = {}, whisperCool = mutation::WHISPER_COOL;
}

void Level::exposure() {
  if (!rules.mutate || player->dead())
    return;
  Player &p = *player;
  Affliction here = world::layer() == world::Layer::Surface
                        ? affliction::source(world::biomeUnder(p.x, p.y), !night.dawn)
                        : affliction::source(world::caveBiomeAt(world::tileOf(p.x), world::tileOf(p.y)));
  for (int i = 0; i < int(Affliction::Count); i++) {
    Affliction a = Affliction(i);
    float &e = p.exposure[i];
    if (a != here || affliction::has(p, a)) {
      e = std::max(0.0f, e - affliction::FADE * DT);
      if (e < affliction::WARN)
        p.warned &= uint8_t(~(1u << i));
      continue;
    }
    e += DT;
    if (e >= affliction::WARN && !(p.warned >> i & 1)) {
      p.warned |= uint8_t(1u << i);
      game.announce(say(lang.afflictWarn, tr(AFFLICTIONS[i].name)));
    }
    if (e >= affliction::CATCH) {
      affliction::grant(p, a);
      e = 0, p.warned &= uint8_t(~(1u << i));
      suspect(affliction::CAUGHT_SCRUTINY);
      game.announce(say(lang.afflictCaught, tr(AFFLICTIONS[i].name)));
    }
  }
}
