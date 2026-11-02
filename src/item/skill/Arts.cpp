#include "item/skill/Skill.hpp"

#include "entity/Ability.hpp"
#include "entity/Mob.hpp"
#include "item/weapon/Recipe.hpp"

namespace {

const Recipe SCRAP{"Scrap Barrage", "TINKERER", "", "bomb", 0, Form::Lob,
                   pay::Burn | pay::Stagger, 20, 0, 220, 0, 6, {1, 0.7f, 0.35f, 1},
                   atlas::pt::Pebble};
const Recipe WHIRLPOOL{"Whirlpool", "CORSAIR", "", "tide", 0, Form::Field,
                       pay::Pull | pay::Chill, 7, 0, 70, 0, 1,
                       {0.4f, 0.85f, 0.9f, 1}, atlas::pt::Bubble};

struct Art {
  const char *name, *owner, *desc;
  int icon;
  float cooldown;
  void (*use)(Mob &);
};

const Art ARTS[] = {
    {"ARROW STORM", "RANGER", "Loose sixteen arrows in every direction", 196, 9,
     [](Mob &p) { ability::volley(p, 16, TAU / 16, 230, 16, 2); }},
    {"SHIELD CHARGE", "KNIGHT", "Charge ahead, battering every foe in your path",
     197, 7, [](Mob &p) { ability::charge(p, 110, 40); }},
    {"MIASMA", "ALCHEMIST", "Poison every foe within 120 for 5s", 198, 10,
     [](Mob &p) { ability::miasma(p, 120, 10, 5); }},
    {"CHAIN STORM", "STORMCALLER", "Lightning leaps between eight nearby foes",
     199, 7, [](Mob &p) { ability::chain(p, 8, 260, 34); }},
    {"DEATH GRIP", "REAPER", "Drag foes within 140 to you and chill them", 200,
     9, [](Mob &p) { ability::grasp(p, 140, 22); }},
    {"SACRED LIGHT", "PRIESTESS", "Call pillars of light down on six foes", 201,
     8, [](Mob &p) { ability::smite(p, 6, 42); }},
    {"LUNGE", "DUELIST", "Lunge forward with a long piercing thrust", 202, 5,
     [](Mob &p) { ability::lunge(p, 100, 55); }},
    {"BRAMBLES", "WARDEN", "Root foes within 110 in thorns for 2.5s", 203, 10,
     [](Mob &p) { ability::entangle(p, 110, 16); }},
    {"MINEFIELD", "TINKERER", "Scatter eight mines in a ring around you", 204,
     12, [](Mob &p) { ability::mines(p, 8, 32); }},
    {"COMET", "ORACLE", "Call three comets down on the nearest foes", 205, 11,
     [](Mob &p) { ability::comet(p, 3, 60); }},
    {"POWDER KEG", "CORSAIR", "Lob a keg that bursts into a wide fire", 206, 9,
     [](Mob &p) { ability::keg(p, 70); }},
    {"WAR BANNER", "KNIGHT", "Plant a banner: heal 4/s and slow foes near it for 8s",
     215, 16, [](Mob &p) { ability::banner(p, 90, 8); }},
    {"HUNTER'S MARK", "RANGER", "Mark six foes and loose an arrow at each", 216, 8,
     [](Mob &p) { ability::hunt(p, 6, 26); }},
    {"FLASK BARRAGE", "ALCHEMIST", "Lob five fire flasks at the nearest foes", 217,
     10, [](Mob &p) { ability::barrage(p, 5, 24); }},
    {"TEMPEST", "STORMCALLER", "Lightning strikes foes around you for 5s", 218, 14,
     [](Mob &p) { ability::tempest(p, 5, 22); }},
    {"BLOOD PACT", "REAPER", "Strike foes within 110 and heal 3 for each", 219, 10,
     [](Mob &p) { ability::drain(p, 110, 30); }},
    {"HALLOWED GROUND", "PRIESTESS", "Hallow the ground: heal 6/s and burn foes for 6s",
     220, 16, [](Mob &p) { ability::sanctuary(p, 90, 6); }},
    {"FLASH STEP", "DUELIST", "Flash between four foes, striking and marking each",
     221, 7, [](Mob &p) { ability::flash(p, 4, 45); }},
    {"OVERGROWTH", "WARDEN", "Wild growth poisons and slows foes for 7s", 222, 14,
     [](Mob &p) { ability::overgrowth(p, 100, 7); }},
    {"SCRAP BARRAGE", "TINKERER", "Lob six scrap bombs that burn and stagger", 223,
     9, [](Mob &p) { castRecipe(SCRAP, p, nullptr, p.power()); }},
    {"ECLIPSE", "ORACLE", "A shadow follows you, marking and slowing foes for 6s",
     224, 15, [](Mob &p) { ability::eclipse(p, 110, 6); }},
    {"WHIRLPOOL", "CORSAIR", "Open a whirlpool that drags foes in and chills them",
     225, 12, [](Mob &p) { castRecipe(WHIRLPOOL, p, nullptr, p.power()); }},
    {"WAR CRY", nullptr, "Hurl back foes within 130; they take 30% more damage for 5s",
     207, 10, [](Mob &p) { ability::warcry(p, 130, 5); }},
    {"BANDAGE", nullptr, "Heal 30 and shake off burns, poison and chill", 208,
     14, [](Mob &p) { ability::bandage(p, 30); }},
    {"TUMBLE", nullptr, "Roll ahead, untouchable for a moment", 209, 4,
     [](Mob &p) { ability::tumble(p, 70); }},
    {"WHIRLWIND", nullptr, "Spin, cutting every foe within 64", 210, 6,
     [](Mob &p) { ability::whirlwind(p, 64, 40); }},
    {"GROUND SLAM", nullptr, "Shake the earth: damage and stun foes within 100",
     211, 9, [](Mob &p) { ability::quake(p, 100, 35); }},
    {"FIRE RING", nullptr, "Ring yourself in flame for 4s", 212, 12,
     [](Mob &p) { ability::firering(p, 8, 14); }},
    {"LODESTAR", nullptr, "Draw in every gem within 420", 213, 20,
     [](Mob &p) { ability::lodestar(p, 420); }},
    {"SNARE NET", nullptr, "Cast a net that binds foes ahead for 3s", 214, 8,
     [](Mob &p) { ability::snare(p, 120, 10); }},
    {"SMOKE BOMB", nullptr, "Vanish in smoke: foes within 90 flee, you are untouchable for 1.5s",
     226, 10, [](Mob &p) { ability::smoke(p, 90, 2.5f); }},
    {"BLOOD RITE", nullptr, "Pay 15% of your health for 30% more damage for 6s", 227, 14,
     [](Mob &p) { ability::rite(p, 0.15f, 0.3f, 6); }},
    {"GRAVITY WELL", nullptr, "Collapse space ahead, dragging foes together and slowing them",
     228, 9, [](Mob &p) { ability::well(p, 90, 100, 14); }},
    {"MIRROR IMAGE", nullptr, "Leave a decoy that draws foes to it for 4s", 229, 12,
     [](Mob &p) { ability::mirror(p, 4); }},
    {"RALLYING HORN", nullptr, "Allies strike 30% faster and you run 15% faster for 6s", 230,
     15, [](Mob &p) { ability::horn(p, 140, 6); }},
    {"SHATTER WAVE", nullptr, "Shatter every chilled foe within 160 and chill the rest", 231,
     8, [](Mob &p) { ability::shatter(p, 160, 24); }},
    {"SECOND SIGHT", nullptr, "Reveal the land around you and mark elites for 8s", 232, 18,
     [](Mob &p) { ability::sight(p, 400); }},
    {"SPRINT", nullptr, "Run 50% faster for 3s", 233, 8, [](Mob &p) { ability::sprint(p, 3); }},
    {"PRISMATIC PRISON", "MAGE", "Trap every foe within 100 in a prism for 3s", 234, 12,
     [](Mob &p) { ability::prison(p, 100, 3, 12); }},
    {"CALL THE HAWK", "RANGER", "A hawk fights beside you and bleeds foes for 8s", 235, 10,
     [](Mob &p) { ability::hawk(p, 8, 16); }},
    {"LAST STAND", "KNIGHT", "Below 30% health: untouchable for 4s and heal 25", 236, 20,
     [](Mob &p) { ability::stand(p, 80, 20); }},
    {"TRANSMUTATION CIRCLE", "ALCHEMIST", "Foes slain in the circle drop twice the gems for 6s",
     237, 18, [](Mob &p) { ability::transmute(p, 110, 6); }},
    {"EYE OF THE STORM", "STORMCALLER", "A storm follows you and strikes three foes a second",
     238, 16, [](Mob &p) { ability::storm(p, 6, 20); }},
    {"RAISE DEAD", "REAPER", "Raise three skeletons to fight for you for 10s", 239, 16,
     [](Mob &p) { ability::raise(p, 3, 10, 16); }},
    {"HALO OF MERCY", "PRIESTESS", "A halo blunts the next three hits and heals 10", 240, 14,
     [](Mob &p) { ability::mercy(p, 3); }},
    {"PERFECT PARRY", "DUELIST", "Turn the next hit within 1s back as a double blow", 241, 6,
     [](Mob &p) { ability::parry(p, 1); }},
    {"EARTHEN WALL", "WARDEN", "Raise a stone wall ahead that batters foes back", 242, 9,
     [](Mob &p) { ability::rampart(p, 60, 30); }},
    {"OVERCLOCK", "TINKERER", "Weapons fire twice as fast for 4s, then jam for 2s", 243, 16,
     [](Mob &p) { ability::overclock(p, 4, 2); }},
    {"FORESIGHT", "ORACLE", "Mark three foes; your next three hits are certain crits", 244, 12,
     [](Mob &p) { ability::foresight(p, 3); }},
    {"CANNONADE", "CORSAIR", "Six cannonballs land in a line ahead", 245, 10,
     [](Mob &p) { ability::cannonade(p, 6, 30); }},
};

class ArtSkill : public Skill {
public:
  explicit ArtSkill(const Art &art)
      : Skill(art.name, atlas::arsenal(art.icon), art.desc, art.owner,
              art.cooldown),
        art(art) {}
  void use(Mob &p) override { art.use(p); }

  const Art &art;
};

} // namespace

void addArtSkills(std::vector<std::unique_ptr<Upgrade>> &u) {
  for (const Art &a : ARTS)
    u.push_back(std::make_unique<ArtSkill>(a));
}
