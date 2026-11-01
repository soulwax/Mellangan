#include "entity/pickup/ItemDrop.hpp"

#include <spdlog/spdlog.h>

#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/pickup/Potion.hpp"
#include "level/Level.hpp"
#include "particle/Spark.hpp"

namespace {

constexpr Color GOLD_INK{1, 0.84f, 0.35f, 1}, STUFF{0.75f, 0.68f, 0.58f, 1};

bool isItem(const Drop &d) { return d.kind == DropKind::Item; }

} // namespace

ItemDrop::ItemDrop(double x, double y, Drop drop, double tx, double ty)
    : Pickup(x, y), drop(std::move(drop)), sx(x), sy(y), tx(tx), ty(ty) {}

void ItemDrop::onAdded() { level->tag(this); }

void ItemDrop::tick() {
  if (t < FLY) {
    t += DT;
    float k = std::min(1.0f, t / FLY);
    x = sx + (tx - sx) * k, y = sy + (ty - sy) * k;
    z = std::sin(k * PI) * 14;
    if (t >= FLY && isItem(drop) && int(drop.item.rarity) >= int(Rarity::Rare))
      Spark::burst(*level, x, y, color(), 8, 40, true);
    return;
  }
  Pickup::tick();
}

bool ItemDrop::beams() const {
  return isItem(drop) && (int(drop.item.rarity) >= int(Rarity::Rare) || drop.item.night());
}

bool ItemDrop::labelled(bool held) const {
  if (!isItem(drop))
    return held;
  return held || int(drop.item.rarity) >= int(Rarity::Magic) || drop.item.night();
}

std::string ItemDrop::label() const {
  if (drop.kind == DropKind::Gold)
    return std::to_string(drop.amount) + " gold";
  if (drop.kind == DropKind::Material)
    return "material";
  return drop.item.name();
}

Color ItemDrop::color() const {
  if (drop.kind == DropKind::Gold)
    return GOLD_INK;
  if (drop.kind == DropKind::Material)
    return STUFF;
  return drop.item.color();
}

float ItemDrop::reach() const {
  if (!isItem(drop))
    return level->player->magnet;
  return int(level->player->bag.size()) < Player::BAG ? 9.0f : -1.0f;
}

void ItemDrop::take(Player &p) {
  if (drop.kind == DropKind::Gold) {
    p.gold += drop.amount;
    return Sound::gem.play(0.25f, 0.1f, 1.4f);
  }
  if (drop.kind == DropKind::Material) {
    p.materials++;
    return Sound::gem.play(0.2f, 0.1f, 0.8f);
  }
  p.bag.push_back(drop.item);
  int r = int(drop.item.rarity);
  if (r >= int(Rarity::Rare) || drop.item.night())
    Sound::chest.play(0.5f, 0, r >= int(Rarity::Unique) ? 0.7f : 1.1f);
  else
    Sound::select.play(0.4f, 0.05f, r >= int(Rarity::Magic) ? 1.2f : 0.9f);
  spdlog::info("loot: {} [{}] ilvl {}", drop.item.name(), rarityName(drop.item.rarity),
               drop.item.ilvl);
}

void ItemDrop::render(Screen &s) {
  Color c = color();
  if (beams() && t < 6) {
    float a = std::min(1.0f, t * 2) * std::min(1.0f, (6 - t) / 2);
    for (int i = 0; i < 4; i++)
      s.rect(xp - 1, yp - 14 - i * 10, 2, 10, c.alpha(a * (0.55f - i * 0.12f)));
  }
  if (drop.kind == DropKind::Gold)
    return s.particle(atlas::pt::Coin, xp, yp, col::white, 1, t * 3);
  if (drop.kind == DropKind::Material)
    return s.particle(atlas::pt::Pebble, xp, yp, col::white, 1, 0);
  s.sprite(atlas::Shadow, xp, y + 1, 8, 4, 0.6f, 0.6f);
  s.center(drop.item.def().icon, xp, yp, 0.8f, std::sin(t * 2) * 0.05f);
}

void ItemDrop::renderLight(Screen &s) {
  if (isItem(drop) && int(drop.item.rarity) >= int(Rarity::Magic))
    s.glow(xp, yp, beams() ? 46 : 28, color() * (beams() ? 0.22f : 0.12f));
}

void ItemDrop::spill(Level &level, double x, double y, Source source, int mlvl) {
  LootRoll roll{source, mlvl};
  roll.night = !level.night.dawn;
  auto drops = rollLoot(roll, rng);
  bool crowded = level.count<ItemDrop>() >= 120;
  for (Drop &d : drops) {
    if (crowded && !(isItem(d) && int(d.item.rarity) >= int(Rarity::Rare)))
      continue;
    if (d.kind == DropKind::Potion) {
      level.add<Potion>(x + rng.range(-12, 12), y + rng.range(-8, 8));
      continue;
    }
    float a = rng.range(0, TAU), r = rng.range(8, 28);
    level.add<ItemDrop>(x, y, std::move(d), x + std::cos(a) * r, y + std::sin(a) * r * 0.7f);
  }
}
