#include "meta/Content.hpp"

#include <set>
#include <string_view>

#include <spdlog/fmt/fmt.h>

#include "entity/Ally.hpp"
#include "hero/Hero.hpp"
#include "item/Upgrade.hpp"
#include "item/loot/ItemBase.hpp"
#include "item/weapon/Recipe.hpp"
#include "level/Structure.hpp"
#include "level/biome/Biome.hpp"

std::vector<std::string> contentFaults() {
  std::vector<std::string> out;
  auto fault = [&](std::string s) { out.push_back(std::move(s)); };
  auto ups = Upgrade::all();
  std::set<std::string_view> upgrades, heroes, recipes, bases;
  for (auto &u : ups)
    if (!u->name || !*u->name)
      fault("an upgrade has no name");
    else if (!upgrades.insert(u->name).second)
      fault(fmt::format("upgrade \"{}\" is defined twice", u->name));
  for (int i = 0; i < HERO_COUNT; i++)
    if (!HEROES[i] || !HEROES[i]->name)
      fault(fmt::format("hero row {} is empty", i));
    else if (!heroes.insert(HEROES[i]->name).second)
      fault(fmt::format("hero \"{}\" is defined twice", HEROES[i]->name));
  for (const Recipe &r : RECIPES)
    if (!recipes.insert(r.name).second)
      fault(fmt::format("recipe \"{}\" is defined twice", r.name));
  for (const ItemBase &b : itemBases())
    if (!bases.insert(b.key).second)
      fault(fmt::format("item base \"{}\" is defined twice", b.key));

  auto upgrade = [&](const char *name, std::string_view from) {
    if (name && !upgrades.count(name))
      fault(fmt::format("{} names unknown upgrade \"{}\"", from, name));
  };
  auto hero = [&](const char *name, std::string_view from) {
    if (name && !heroes.count(name))
      fault(fmt::format("{} names unknown hero \"{}\"", from, name));
  };
  for (const Hero *h : HEROES) {
    if (!h)
      continue;
    std::string from = fmt::format("hero \"{}\"", h->name);
    upgrade(h->weapon, from);
    upgrade(h->skill, from);
    for (const char *a : h->affinity)
      upgrade(a, from);
  }
  for (auto &u : ups)
    hero(u->owner, fmt::format("upgrade \"{}\"", u->name));
  for (const Recipe &r : RECIPES)
    hero(r.owner, fmt::format("recipe \"{}\"", r.name));
  std::set<std::string_view> evolving;
  for (const auto &e : evolutionTable()) {
    std::string from = fmt::format("evolution \"{}\"", e[2]);
    upgrade(e[0], from);
    upgrade(e[1], from);
    if (!evolving.insert(e[0]).second)
      fault(fmt::format("\"{}\" evolves twice", e[0]));
  }
  for (const char *a : areaWeapons())
    upgrade(a, "area weapon list");
  for (int i = 0; i < int(AllyKind::Count); i++) {
    const AllyDef &a = allyDef(AllyKind(i));
    if (a.recipe && !recipes.count(a.recipe) && !upgrades.count(a.recipe))
      fault(fmt::format("ally \"{}\" names unknown recipe \"{}\"", a.name, a.recipe));
  }
  for (int i = 0; i < int(StructureKind::Count); i++)
    if (!STRUCTURES[i].name)
      fault(fmt::format("structure row {} is empty", i));
  for (int i = 0; i < BIOME_COUNT; i++)
    if (!BIOMES[i])
      fault(fmt::format("biome row {} is empty", i));
  return out;
}
