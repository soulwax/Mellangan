#include "entity/enemy/Boss.hpp"

#include "entity/Ability.hpp"
#include "level/biome/Biome.hpp"

namespace {
constexpr uint32_t land(BiomeId b) { return 1u << int(b); }

template <int N> constexpr Phase phase(const Cast (&c)[N]) { return {c, N}; }

constexpr Color VIOLET{0.72f, 0.45f, 1, 1}, CRIMSON{0.95f, 0.25f, 0.3f, 1},
    EARTH{0.85f, 0.65f, 0.4f, 1}, FROST{0.6f, 0.85f, 1, 1},
    GLASS{0.6f, 0.95f, 0.9f, 1}, SPORE{0.6f, 0.95f, 0.4f, 1},
    CINDER{1, 0.5f, 0.15f, 1}, BOG{0.45f, 0.75f, 0.45f, 1},
    STORM{0.55f, 0.75f, 1, 1};

const Cast LICH1[] = {
    {[](Enemy &e) { ability::blink(e, 80, 0.6f * e.type.damage, VIOLET); }, 6, 320, VIOLET},
    {[](Enemy &e) { ability::thunder(e, 4, 0.5f * e.type.damage); }, 7, 320, STORM}};
const Cast LICH2[] = {
    {[](Enemy &e) { ability::thunder(e, 5, 0.5f * e.type.damage); }, 6, 320, STORM},
    {[](Enemy &e) { ability::comet(e, 3, 0.5f * e.type.damage); }, 7, 320, VIOLET},
    {[](Enemy &e) { ability::blink(e, 80, 0.6f * e.type.damage, VIOLET); }, 6, 320, VIOLET}};
const Cast LICH3[] = {
    {[](Enemy &e) { ability::comet(e, 4, 0.5f * e.type.damage); }, 6, 320, VIOLET},
    {[](Enemy &e) { ability::grasp(e, 150, 0.4f * e.type.damage); }, 6, 150, VIOLET},
    {[](Enemy &e) { ability::chain(e, 1, 260, 0.5f * e.type.damage); }, 5, 260, STORM}};

const Cast LORD1[] = {
    {[](Enemy &e) { ability::nova(e, 70, 0.5f * e.type.damage, e.maxHp * 0.03f, CRIMSON); },
     6, 120, CRIMSON},
    {[](Enemy &e) { ability::dance(e, 70, 0.5f * e.type.damage, CRIMSON); }, 6, 200, CRIMSON}};
const Cast LORD2[] = {
    {[](Enemy &e) { ability::dance(e, 80, 0.5f * e.type.damage, CRIMSON); }, 5, 200, CRIMSON},
    {[](Enemy &e) { ability::warcry(e, 140, 4); }, 8, 140, CRIMSON},
    {[](Enemy &e) { ability::nova(e, 80, 0.5f * e.type.damage, e.maxHp * 0.03f, CRIMSON); },
     6, 120, CRIMSON}};
const Cast LORD3[] = {
    {[](Enemy &e) { ability::charge(e, 120, 0.6f * e.type.damage); }, 5, 200, CRIMSON},
    {[](Enemy &e) { ability::dance(e, 90, 0.5f * e.type.damage, CRIMSON); }, 5, 200, CRIMSON},
    {[](Enemy &e) { ability::warcry(e, 160, 5); }, 7, 160, CRIMSON}};

const Cast CYC1[] = {
    {[](Enemy &e) { ability::volley(e, 7, 0.16, 200, 0.4f * e.type.damage, 1); }, 7, 320, EARTH},
    {[](Enemy &e) { ability::quake(e, 110, 0.6f * e.type.damage); }, 8, 110, EARTH}};
const Cast CYC2[] = {
    {[](Enemy &e) { ability::volley(e, 9, 0.14, 210, 0.4f * e.type.damage, 1); }, 6, 320, EARTH},
    {[](Enemy &e) { ability::quake(e, 120, 0.6f * e.type.damage); }, 7, 120, EARTH},
    {[](Enemy &e) { ability::charge(e, 120, 0.6f * e.type.damage); }, 7, 220, EARTH}};
const Cast CYC3[] = {
    {[](Enemy &e) { ability::charge(e, 140, 0.6f * e.type.damage); }, 5, 240, EARTH},
    {[](Enemy &e) { ability::quake(e, 130, 0.7f * e.type.damage); }, 6, 130, EARTH},
    {[](Enemy &e) { ability::keg(e, 0.8f * e.type.damage); }, 6, 220, CINDER}};

const Cast FROST1[] = {
    {[](Enemy &e) { ability::quake(e, 110, 12); }, 7, 110, FROST},
    {[](Enemy &e) { ability::snare(e, 140, 5); }, 8, 140, FROST}};
const Cast FROST2[] = {
    {[](Enemy &e) { ability::charge(e, 130, 14); }, 6, 220, FROST},
    {[](Enemy &e) { ability::quake(e, 120, 14); }, 7, 120, FROST},
    {[](Enemy &e) { ability::entangle(e, 120, 6); }, 9, 120, FROST}};
const Cast FROST3[] = {
    {[](Enemy &e) { ability::charge(e, 150, 16); }, 5, 240, FROST},
    {[](Enemy &e) { ability::comet(e, 3, 16); }, 6, 300, FROST},
    {[](Enemy &e) { ability::snare(e, 160, 6); }, 7, 160, FROST}};

const Cast GLASS1[] = {
    {[](Enemy &e) { ability::lunge(e, 110, 14); }, 5, 140, GLASS},
    {[](Enemy &e) { ability::volley(e, 5, 0.2, 220, 8, 1); }, 6, 300, GLASS}};
const Cast GLASS2[] = {
    {[](Enemy &e) { ability::charge(e, 140, 14); }, 5, 240, GLASS},
    {[](Enemy &e) { ability::volley(e, 9, 0.15, 230, 8, 1); }, 6, 300, GLASS},
    {[](Enemy &e) { ability::lunge(e, 120, 14); }, 5, 150, GLASS}};
const Cast GLASS3[] = {
    {[](Enemy &e) { ability::volley(e, 16, TAU / 16, 220, 8, 1); }, 5, 300, GLASS},
    {[](Enemy &e) { ability::dance(e, 90, 12, GLASS); }, 5, 200, GLASS},
    {[](Enemy &e) { ability::charge(e, 160, 16); }, 5, 260, GLASS}};

const Cast SPORE1[] = {
    {[](Enemy &e) { ability::miasma(e, 120, 4, 4); }, 7, 120, SPORE},
    {[](Enemy &e) { ability::nova(e, 80, 8, e.maxHp * 0.02f, SPORE); }, 8, 100, SPORE}};
const Cast SPORE2[] = {
    {[](Enemy &e) { ability::miasma(e, 140, 5, 5); }, 6, 140, SPORE},
    {[](Enemy &e) { ability::entangle(e, 130, 8); }, 8, 130, SPORE},
    {[](Enemy &e) { ability::grasp(e, 150, 8); }, 8, 150, SPORE}};
const Cast SPORE3[] = {
    {[](Enemy &e) { ability::miasma(e, 170, 6, 5); }, 5, 170, SPORE},
    {[](Enemy &e) { ability::grasp(e, 170, 10); }, 6, 170, SPORE},
    {[](Enemy &e) { ability::bandage(e, e.maxHp * 0.05f); }, 10, 300, SPORE}};

const Cast CINDER1[] = {
    {[](Enemy &e) { ability::keg(e, 20); }, 6, 220, CINDER},
    {[](Enemy &e) { ability::firering(e, 8, 6); }, 9, 110, CINDER}};
const Cast CINDER2[] = {
    {[](Enemy &e) { ability::comet(e, 3, 16); }, 6, 300, CINDER},
    {[](Enemy &e) { ability::keg(e, 22); }, 5, 220, CINDER},
    {[](Enemy &e) { ability::firering(e, 10, 7); }, 8, 120, CINDER}};
const Cast CINDER3[] = {
    {[](Enemy &e) { ability::comet(e, 5, 16); }, 5, 300, CINDER},
    {[](Enemy &e) { ability::mines(e, 8, 14); }, 7, 140, CINDER},
    {[](Enemy &e) { ability::quake(e, 120, 14); }, 6, 120, EARTH}};

const Cast BOG1[] = {
    {[](Enemy &e) { ability::grasp(e, 150, 8); }, 7, 150, BOG},
    {[](Enemy &e) { ability::miasma(e, 120, 4, 4); }, 7, 120, BOG}};
const Cast BOG2[] = {
    {[](Enemy &e) { ability::grasp(e, 170, 9); }, 6, 170, BOG},
    {[](Enemy &e) { ability::blink(e, 90, 10, BOG); }, 6, 240, BOG},
    {[](Enemy &e) { ability::smite(e, 4, 12); }, 7, 260, BOG}};
const Cast BOG3[] = {
    {[](Enemy &e) { ability::smite(e, 6, 12); }, 5, 280, BOG},
    {[](Enemy &e) { ability::grasp(e, 190, 10); }, 5, 190, BOG},
    {[](Enemy &e) { ability::miasma(e, 160, 6, 5); }, 6, 160, BOG}};

const Cast STORM1[] = {
    {[](Enemy &e) { ability::thunder(e, 4, 10); }, 6, 320, STORM},
    {[](Enemy &e) { ability::blink(e, 90, 10, STORM); }, 6, 240, STORM}};
const Cast STORM2[] = {
    {[](Enemy &e) { ability::chain(e, 1, 280, 14); }, 5, 280, STORM},
    {[](Enemy &e) { ability::thunder(e, 6, 10); }, 6, 320, STORM},
    {[](Enemy &e) { ability::warcry(e, 140, 4); }, 9, 140, STORM}};
const Cast STORM3[] = {
    {[](Enemy &e) { ability::thunder(e, 8, 11); }, 5, 320, STORM},
    {[](Enemy &e) { ability::chain(e, 1, 300, 16); }, 4, 300, STORM},
    {[](Enemy &e) { ability::comet(e, 3, 14); }, 6, 300, VIOLET}};
} // namespace

const BossDef BOSS_DEFS[BOSS_COUNT] = {
    {EnemyKind::Cyclops, "THE CYCLOPS", "A CYCLOPS APPROACHES", "THE CYCLOPS FALLS",
     EARTH, {phase(CYC1), phase(CYC2), phase(CYC3)}, EnemyKind::Rat, 0,
     land(BIOME_MEADOW) | land(BIOME_HEATH) | land(BIOME_SAVANNA) |
         land(BIOME_BLOSSOM)},
    {EnemyKind::Alpha, "THE PACK LORD", "THE PACK LORD HOWLS", "THE PACK LORD FALLS",
     CRIMSON, {phase(LORD1), phase(LORD2), phase(LORD3)}, EnemyKind::Wolf, 3,
     land(BIOME_FOREST) | land(BIOME_AUTUMN) | land(BIOME_AMBER)},
    {EnemyKind::Necromancer, "THE LICH", "THE LICH RISES", "THE LICH CRUMBLES", VIOLET,
     {phase(LICH1), phase(LICH2), phase(LICH3)}, EnemyKind::Skeleton, 4,
     land(BIOME_BARROW) | land(BIOME_RUINS) | land(BIOME_MOON)},
    {EnemyKind::Iceback, "THE FROST COLOSSUS", "THE GROUND FREEZES OVER",
     "THE FROST COLOSSUS SHATTERS", FROST,
     {phase(FROST1), phase(FROST2), phase(FROST3)}, EnemyKind::FrostWisp, 3,
     land(BIOME_SNOW) | land(BIOME_TAIGA)},
    {EnemyKind::GlassMantis, "THE GLASS TYRANT", "THE SANDS SING",
     "THE GLASS TYRANT BREAKS", GLASS, {phase(GLASS1), phase(GLASS2), phase(GLASS3)},
     EnemyKind::DustVulture, 2,
     land(BIOME_DESERT) | land(BIOME_BADLANDS) | land(BIOME_SALT) | land(BIOME_GLASS)},
    {EnemyKind::MycelialBrute, "THE SPORE MOTHER", "THE SPORE MOTHER WAKES",
     "THE SPORE MOTHER WITHERS", SPORE, {phase(SPORE1), phase(SPORE2), phase(SPORE3)},
     EnemyKind::Sporeling, 5,
     land(BIOME_FUNGAL) | land(BIOME_HOLLOW) | land(BIOME_JUNGLE)},
    {EnemyKind::CinderGolem, "THE CINDER KING", "THE EARTH BURNS",
     "THE CINDER KING GUTTERS OUT", CINDER,
     {phase(CINDER1), phase(CINDER2), phase(CINDER3)}, EnemyKind::EmberImp, 3,
     land(BIOME_ASHLANDS) | land(BIOME_EMBER)},
    {EnemyKind::BogLantern, "THE BOG WITCH", "LIGHTS DANCE ON THE WATER",
     "THE BOG WITCH SINKS", BOG, {phase(BOG1), phase(BOG2), phase(BOG3)},
     EnemyKind::MireLeech, 4,
     land(BIOME_MIRE) | land(BIOME_FLOOD)},
    {EnemyKind::Cultist, "THE STORM HERALD", "THE SKY SPLITS",
     "THE STORM HERALD IS SILENCED", STORM,
     {phase(STORM1), phase(STORM2), phase(STORM3)}, EnemyKind::Cultist, 2,
     land(BIOME_STORM) | land(BIOME_STARFALL) | land(BIOME_CRYSTAL)},
};

const BossDef &bossOf(EnemyKind kind) {
  for (const BossDef &d : BOSS_DEFS)
    if (d.kind == kind)
      return d;
  return BOSS_DEFS[0];
}

const BossDef *bossFor(int biome) {
  for (const BossDef &d : BOSS_DEFS)
    if (d.lands >> biome & 1)
      return &d;
  return nullptr;
}
