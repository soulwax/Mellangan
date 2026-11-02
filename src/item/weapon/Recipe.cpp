#include "item/weapon/Recipe.hpp"

#include "item/weapon/form/FormBehavior.hpp"

#include "entity/Player.hpp"
#include "level/Level.hpp"

const std::vector<Recipe> RECIPES = {
    {"Ember Lance", "MAGE", "A searing lance pierces and ignites foes", "lance",
     0, Form::Bolt, pay::Burn | pay::Pierce, 14, 1.6f, 260, 330, 1,
     {1, 0.55f, 0.2f, 1}, atlas::pt::Ember},
    {"Hoarfrost Fan", "MAGE", "Fans out ice shards that chill foes", "shard", 1,
     Form::Volley, pay::Chill, 8, 1.8f, 150, 260, 5, {0.6f, 0.85f, 1, 1},
     atlas::pt::IceShard},
    {"Arcane Orbit", "MAGE", "Arcane orbs circle you and mark foes", "orb", 2,
     Form::Orbit, pay::Mark, 9, 4.0f, 34, 3.2f, 2, {0.78f, 0.55f, 1, 1},
     atlas::pt::ArcaneStar},
    {"Starfall", "MAGE", "Stars crash down on distant foes", "star", 3,
     Form::Rain, pay::Explode, 26, 3.2f, 200, 0, 3, {1, 0.92f, 0.6f, 1},
     atlas::pt::FallingStar},
    {"Void Rift", "MAGE", "A rift drags foes inward and grinds them", "rift", 4,
     Form::Field, pay::Pull, 5, 5.0f, 48, 0, 1, {0.55f, 0.3f, 0.95f, 1},
     atlas::pt::Wisp},
    {"Seeker Runes", "MAGE", "Runes hunt foes and split on impact", "rune", 5,
     Form::Seeker, pay::Split, 11, 2.4f, 240, 150, 2, {0.5f, 0.9f, 1, 1},
     atlas::pt::Rune},
    {"Flame Ring", "MAGE", "A ring of fire bursts outward", "ring", 6,
     Form::Nova, pay::Burn, 12, 3.0f, 90, 0, 1, {1, 0.45f, 0.15f, 1},
     atlas::pt::Ember},
    {"Lionheart Cleave", "KNIGHT", "A broad cleave staggers foes before you",
     "sweep", 7, Form::Cleave, pay::Stagger, 18, 1.5f, 52, 0, 1,
     {1, 0.86f, 0.5f, 1}, atlas::pt::Crescent},
    {"Bastion Shields", "KNIGHT", "Iron shields circle you and batter foes back",
     "shield", 8, Form::Orbit, pay::Stagger, 10, 4.5f, 30, 2.6f, 2,
     {0.8f, 0.85f, 0.95f, 1}, atlas::pt::Coin},
    {"Hawk Talon", "RANGER", "A curved blade flies out and wheels back",
     "talon", 9, Form::Boomerang, pay::Pierce, 14, 2.0f, 170, 240, 1,
     {0.75f, 0.95f, 0.6f, 1}, atlas::pt::Feather},
    {"Bear Trap", "RANGER", "Hidden jaws snap shut and pin foes", "trap", 10,
     Form::Trap, pay::Stagger, 30, 2.6f, 34, 0, 1, {0.85f, 0.8f, 0.7f, 1},
     atlas::pt::Pebble},
    {"Caustic Flask", "ALCHEMIST", "Lobbed flasks burst into choking poison",
     "flask", 11, Form::Lob, pay::Poison, 16, 2.2f, 200, 0, 1,
     {0.55f, 0.95f, 0.35f, 1}, atlas::pt::Bubble},
    {"Witchfire Phial", "ALCHEMIST", "Phials of witchfire splash, burn and mark",
     "phial", 12, Form::Lob, pay::Burn | pay::Mark, 14, 2.6f, 200, 0, 2,
     {0.95f, 0.45f, 0.9f, 1}, atlas::pt::Ember},
    {"Ball Lightning", "STORMCALLER", "Crackling orbs hunt foes and arc onward",
     "orb", 13, Form::Seeker, pay::Chain, 10, 2.2f, 240, 140, 2,
     {0.55f, 0.75f, 1, 1}, atlas::pt::Wisp},
    {"Arc Lance", "STORMCALLER", "A bolt lances a line of foes and leaps on",
     "lance", 14, Form::Beam, pay::Chain, 16, 1.8f, 180, 0, 1,
     {0.6f, 0.85f, 1, 1}, atlas::pt::Rune},
    {"Soul Harvest", "REAPER", "A wide reaping arc that drinks their life",
     "sweep", 15, Form::Cleave, pay::Leech, 16, 1.6f, 58, 0, 1,
     {0.75f, 0.3f, 0.55f, 1}, atlas::pt::Crescent},
    {"Bone Wheel", "REAPER", "A wheel of bone carves out and back, poisoning",
     "wheel", 16, Form::Boomerang, pay::Poison, 12, 2.2f, 160, 220, 1,
     {0.95f, 0.9f, 0.8f, 1}, atlas::pt::Bone},
    {"Lance of Dawn", "PRIESTESS", "A ray of dawn sears and marks a line of foes",
     "ray", 17, Form::Beam, pay::Burn | pay::Mark, 15, 2.0f, 190, 0, 1,
     {1, 0.9f, 0.55f, 1}, atlas::pt::HolyGlint},
    {"Seraph Wings", "PRIESTESS", "Feathers of light circle you and mend you",
     "feather", 18, Form::Orbit, pay::Leech, 8, 4.5f, 40, 2.4f, 3,
     {1, 0.97f, 0.85f, 1}, atlas::pt::Feather},
    {"Crescent Cut", "DUELIST", "Quick twin arcs that mark foes for the kill",
     "cut", 19, Form::Cleave, pay::Mark, 12, 1.2f, 46, 0, 2,
     {1, 0.8f, 0.9f, 1}, atlas::pt::Crescent},
    {"Flicker Darts", "DUELIST", "Darts seek out foes and pierce through", "dart",
     20, Form::Seeker, pay::Pierce, 9, 1.6f, 220, 200, 3, {0.95f, 0.95f, 1, 1},
     atlas::pt::Petal},
    {"Thornwhip", "WARDEN", "A lashing vine poisons and staggers foes", "lash",
     21, Form::Cleave, pay::Poison | pay::Stagger, 14, 1.5f, 60, 0, 1,
     {0.5f, 0.85f, 0.4f, 1}, atlas::pt::Leaf},
    {"Root Snare", "WARDEN", "Seeds sprout roots that seize passing foes",
     "seed", 22, Form::Trap, pay::Chill | pay::Poison, 24, 2.8f, 40, 0, 2,
     {0.45f, 0.75f, 0.3f, 1}, atlas::pt::MapleLeaf},
    {"Clockwork Mine", "TINKERER", "Mines tick under your feet and blow apart",
     "mine", 23, Form::Trap, pay::Burn, 34, 2.4f, 44, 0, 1,
     {1, 0.7f, 0.35f, 1}, atlas::pt::Coin},
    {"Flash Grenade", "TINKERER", "A lobbed grenade dazzles and marks foes",
     "grenade", 24, Form::Lob, pay::Mark | pay::Stagger, 18, 2.4f, 210, 0, 1,
     {1, 0.95f, 0.7f, 1}, atlas::pt::Pebble},
    {"Prophecy Ray", "ORACLE", "Rays of foresight chill and mark foes", "ray",
     25, Form::Beam, pay::Mark | pay::Chill, 13, 1.8f, 210, 0, 2,
     {0.75f, 0.65f, 1, 1}, atlas::pt::ArcaneStar},
    {"Fate Wheel", "ORACLE", "Runes of fate circle you and arc to their kin",
     "rune", 26, Form::Orbit, pay::Chain, 9, 4.2f, 38, 2.8f, 2,
     {0.7f, 0.6f, 1, 1}, atlas::pt::Rune},
    {"Chain Shot", "CORSAIR", "Chained shot tears through and staggers foes",
     "shot", 27, Form::Bolt, pay::Pierce | pay::Stagger, 20, 1.9f, 240, 300, 1,
     {0.7f, 0.72f, 0.78f, 1}, atlas::pt::Rock},
    {"Riptide", "CORSAIR", "A riptide drags foes in and chills them", "tide",
     28, Form::Field, pay::Pull | pay::Chill, 6, 5.0f, 46, 0, 1,
     {0.4f, 0.85f, 0.9f, 1}, atlas::pt::Bubble},
    {"Bloodletter Fan", nullptr, "Fans out barbed blades that open bleeding wounds", "blade",
     29, Form::Volley, pay::Bleed, 10, 1.8f, 160, 270, 4, {0.9f, 0.3f, 0.35f, 1},
     atlas::pt::Crescent},
    {"Hex Whip", nullptr, "A cursed lash weakens and marks foes", "lash", 30, Form::Whip,
     pay::Weaken | pay::Mark, 8, 1.4f, 72, 0, 1, {0.7f, 0.4f, 0.9f, 1}, atlas::pt::Rune},
    {"Bone Spear", nullptr, "A heavy spear pierces a line and finishes the weak", "spear", 31,
     Form::Bolt, pay::Pierce | pay::Execute, 18, 1.9f, 280, 360, 1, {0.95f, 0.9f, 0.78f, 1},
     atlas::pt::Bone},
    {"Thunder Drum", nullptr, "Drumbeats boom outward, stagger foes and arc onward", "beat",
     32, Form::Nova, pay::Stagger | pay::Chain, 20, 2.4f, 90, 0, 1, {0.65f, 0.8f, 1, 1},
     atlas::pt::Gust},
    {"Frost Lantern", nullptr, "Frozen lanterns circle you and shatter chilled foes",
     "lantern", 33, Form::Orbit, pay::Chill | pay::Shatter, 16, 4.2f, 38, 2.8f, 3,
     {0.7f, 0.9f, 1, 1}, atlas::pt::Snowflake},
    {"Rot Bellows", nullptr, "Bellows puff a rotting cloud that poisons and weakens",
     "cloud", 34, Form::Field, pay::Poison | pay::Weaken, 5, 5.0f, 46, 0, 1,
     {0.6f, 0.75f, 0.3f, 1}, atlas::pt::Spore},
    {"Grave Bell", nullptr, "A tolling bell sends nearby foes fleeing", "toll", 35, Form::Nova,
     pay::Fear, 14, 3.2f, 100, 0, 1, {0.8f, 0.82f, 0.9f, 1}, atlas::pt::Puff},
    {"Tidal Ring", nullptr, "Water spirals outward, dragging and chilling foes", "wave", 36,
     Form::Spiral, pay::Pull | pay::Chill, 18, 2.4f, 110, 150, 4, {0.35f, 0.75f, 0.95f, 1},
     atlas::pt::Bubble},
    {"Ember Wall", nullptr, "Raises a burning wall that holds foes back", "wall", 37,
     Form::Wall, pay::Burn, 26, 3.6f, 90, 0, 2, {1, 0.5f, 0.2f, 1}, atlas::pt::Ember},
    {"Sky Lance", nullptr, "Lances fall from the sky, bursting and staggering", "lance", 38,
     Form::Rain, pay::Explode | pay::Stagger, 14, 3.4f, 210, 0, 2, {0.75f, 0.9f, 1, 1},
     atlas::pt::FallingStar},
    {"Wolf Totem", nullptr, "A totem calls spirit wolves that bleed foes", "wolf", 39,
     Form::Summon, pay::Bleed, 16, 6.0f, 0, 0, 2, {0.75f, 0.85f, 1, 1}, atlas::pt::Wisp},
    {"Mirror Shards", nullptr, "Glass shards spiral out, pierce and split apart", "shard", 40,
     Form::Spiral, pay::Split | pay::Pierce, 6, 2.2f, 120, 160, 4, {0.9f, 0.95f, 1, 1},
     atlas::pt::IceShard},
    {"Runic Wall", "MAGE", "Raises a wall of runes that marks and chills", "wall", 41,
     Form::Wall, pay::Mark | pay::Chill, 24, 3.6f, 90, 0, 2, {0.7f, 0.55f, 1, 1},
     atlas::pt::Rune},
    {"Hawk Companion", "RANGER", "Hawks dive on foes and leave them bleeding", "hawk", 42,
     Form::Summon, pay::Bleed, 8, 6.0f, 0, 0, 2, {0.95f, 0.85f, 0.6f, 1},
     atlas::pt::Feather},
    {"Shield Wall", "KNIGHT", "A wall of shields batters foes back", "wall", 43, Form::Wall,
     pay::Stagger, 26, 3.6f, 90, 0, 2, {0.8f, 0.85f, 0.95f, 1}, atlas::pt::Coin},
    {"Philosopher's Spiral", "ALCHEMIST", "Gilded droplets spiral out, burn and poison",
     "drop", 44, Form::Spiral, pay::Poison | pay::Burn, 14, 2.4f, 110, 150, 4,
     {0.95f, 0.85f, 0.35f, 1}, atlas::pt::Bubble},
    {"Storm Whip", "STORMCALLER", "A crackling lash arcs on to nearby foes", "lash", 45,
     Form::Whip, pay::Chain, 10, 1.4f, 76, 0, 1, {0.55f, 0.75f, 1, 1}, atlas::pt::Wisp},
    {"Ghoul Pack", "REAPER", "Ghouls claw at foes and feed on them", "ghoul", 46,
     Form::Summon, pay::Leech, 20, 6.0f, 0, 0, 2, {0.6f, 0.9f, 0.55f, 1}, atlas::pt::Bone},
    {"Sun Spiral", "PRIESTESS", "Motes of sunlight spiral out, burn and mark", "mote", 47,
     Form::Spiral, pay::Burn | pay::Mark, 14, 2.4f, 110, 150, 4, {1, 0.9f, 0.55f, 1},
     atlas::pt::HolyGlint},
    {"Riposte Whip", "DUELIST", "A flicking lash opens wounds and finishes the weak",
     "lash", 48, Form::Whip, pay::Bleed | pay::Execute, 5, 1.4f, 70, 0, 1,
     {1, 0.8f, 0.9f, 1}, atlas::pt::Petal},
    {"Bramble Wall", "WARDEN", "A hedge of thorns poisons and staggers foes", "hedge", 49,
     Form::Wall, pay::Poison | pay::Stagger, 24, 3.6f, 90, 0, 2, {0.5f, 0.85f, 0.4f, 1},
     atlas::pt::Leaf},
    {"Clockwork Drone", "TINKERER", "Drones hover near you and fire at foes", "drone", 50,
     Form::Summon, pay::Pierce, 14, 6.0f, 0, 0, 2, {1, 0.8f, 0.45f, 1}, atlas::pt::Coin},
    {"Fate Spiral", "ORACLE", "Runes of fate spiral out, mark and weaken", "rune", 51,
     Form::Spiral, pay::Mark | pay::Weaken, 14, 2.4f, 110, 150, 4, {0.75f, 0.65f, 1, 1},
     atlas::pt::ArcaneStar},
    {"Powder Crew", "CORSAIR", "A powder crew brawls beside you and blows foes apart",
     "sailor", 52, Form::Summon, pay::Explode, 16, 6.0f, 0, 0, 2, {0.9f, 0.75f, 0.6f, 1},
     atlas::pt::Puff},
};

RecipeWeapon::RecipeWeapon(const Recipe &r)
    : Upgrade(r.name, atlas::arsenal(r.icon), {}, true), r(r) {
  owner = r.owner;
  int add = int(std::lround(r.damage * 0.5f));
  bool wide = FormBehavior::of(r.form).area();
  text = {r.pitch, fmt::format("+{} damage", add),
          wide ? std::string("+30% radius") : fmt::format("+1 {}", r.unit),
          "25% faster",
          wide ? fmt::format("+{} damage, +30% radius", add)
               : fmt::format("+{} damage, +1 {}", add, r.unit)};
  for (int i = 0; i < MAX_LEVEL; i++)
    desc[i] = text[i].c_str();
}

int RecipeWeapon::count() const {
  if (FormBehavior::of(r.form).area())
    return r.count;
  return r.count + at(level, 3) + at(level, 5) + evolved + int(holder ? holder->bonus.shots : 0);
}

float RecipeWeapon::reach() const {
  if (!FormBehavior::of(r.form).area())
    return r.reach;
  return r.reach * (1 + 0.3f * (at(level, 3) + at(level, 5)) + 0.25f * evolved) *
         (1 + (holder ? holder->bonus.area : 0) / 100);
}

float RecipeWeapon::damage(const Mob &p) const {
  return r.damage * (1 + 0.5f * (at(level, 2) + at(level, 5))) * p.might * power;
}

void RecipeWeapon::tick(Mob &p) {
  if ((cd -= DT) > 0 || p.dead())
    return;
  cd = r.cooldown * (level >= 4 ? 0.75f : 1) * p.haste * pace;
  fireRecipe(r, p, this, damage(p), reach(), count());
}

void fireRecipe(const Recipe &r, Mob &p, Upgrade *source, float damage,
                float reach, int count) {
  Level &level = *p.level;
  auto targets = level.nearestFoes(p, p.x, p.y, std::max(reach, 240.0f), 8);
  double dx = p.xAim, dy = p.yAim;
  if (!targets.empty())
    dx = targets.front()->x - p.x, dy = targets.front()->bodyY() - p.bodyY();
  unit(dx, dy);
  FormBehavior::of(r.form).fire(
      {r, p, source, damage, reach, count, targets, dx, dy, std::atan2(dy, dx)});
}
