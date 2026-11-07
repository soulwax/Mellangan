#pragma once

#include <memory>

#include "entity/Mob.hpp"
#include "entity/enemy/ai/Goal.hpp"

class Sound;

enum class EnemyKind {
  Bat,
  Rat,
  Slime,
  Scorpion,
  Spider,
  Ghost,
  Cultist,
  Demon,
  Knight,
  Cyclops,
  Wolf,
  Alpha,
  Burrower,
  Necromancer,
  Skeleton,
  Thief,
  MireLeech,
  BogLantern,
  GlassMantis,
  DustVulture,
  FrostWisp,
  Iceback,
  Sporeling,
  MycelialBrute,
  EmberImp,
  CinderGolem,
  Mimic,
  Outlaw,
  Berserker,
  PitFighter,
  Deserter,
  Thornling,
  Boar,
  GoblinArcher,
  GoblinShaman,
  Sapling,
  Treant,
  MothWisp,
  Cutthroat,
  BogToad,
  Drowned,
  Snapjaw,
  StranglerVine,
  AmberBeetle,
  SporePuffer,
  SunkenPriest,
  Yeti,
  IceWraith,
  BarrowWight,
  CrystalGolem,
  CrystalShard,
  MoonHound,
  Lynx,
  BarrowArcher,
  SandWyrm,
  Hyena,
  SaltGolem,
  GlassSpitter,
  StormKite,
  DuneRaider,
  TotemShaman,
  WarTotem,
  MagmaSlug,
  AshWraith,
  CaveHorror,
  StarSpawn,
  Starlet,
  FlameCultist,
  HollowSentinel,
  EmberDrake,
  NightStalker,
  GraveRobber,
  CaveBat,
  RockCrawler,
  OreGolem,
  KoboldMiner,
  Shade,
  Nest,
  Lamplighter,
  Penitent,
  Censer,
  Limer,
  Confessor,
  Prioress,
  Count
};

class Enemy;
class Player;
struct BossDef;
struct NamedPack;

struct Cast {
  void (*fn)(Enemy &);
  float every, range;
  Color tone;
};

struct EnemyType {
  int tile;
  float hp, speed, damage, radius, scale, xp, knockResist;
  Color blood;
  bool floats;
  float unlockAt;
  Sprite (*art)(int) = nullptr;
};

class Enemy : public Mob {
public:
  const EnemyType &type;
  float damageMul;
  bool elite = false;
  const Plan *plan = nullptr;
  GoalQueue goals;
  float goalTime = 0, cooldown = 0, frenzy = 0, age = 0;
  double xMark = 0, yMark = 0, xWant = 0, yWant = 0, xPush = 0, yPush = 0,
         side = 1, xOld = NAN, yOld = NAN;
  entt::entity leader = entt::null, rival = entt::null;
  float feud = 0, brawl = 0;
  uint32_t seed;
  bool aware = true, wantsFire = false, wantsLeader = false, faltered = false,
       alarmed = false, buried = false, gilded = false, boss = false, pounced = false,
       garrison = false;

  static constexpr float WIND = 0.6f;

  Enemy(const EnemyType &type, float hpMul, float damageMul);
  static const EnemyType &typeOf(EnemyKind kind);
  static const char *nameOf(EnemyKind kind);
  static std::unique_ptr<Enemy> create(EnemyKind kind, float hpMul,
                                       float damageMul);

  void onAdded() override;
  void tick() override;
  void render(Screen &s) override;
  void renderShadow(Screen &s) override;
  float occlusion(float &width) const override {
    return type.floats ? (width = 0, 0.0f) : Mob::occlusion(width);
  }
  void renderLight(Screen &s) override;
  void hurt(float damage, double xd, double yd, float force,
            Upgrade *src = nullptr, bool periodic = false);
  const char *name() const;
  int kind() const;
  bool is(EnemyKind k) const { return kind() == int(k); }
  Sprite sprite() const;
  void makeElite(float health = 4);
  bool hostile() const override { return true; }
  void split() override;
  bool bossLike() const override { return boss || def; }
  virtual float contactBonus(const Player &) const { return 1; }
  virtual float shell(double, double) { return 1; }
  virtual void bit(Player &) {}
  float power() const override { return damageMul * might * status.outgoing(); }
  void aim(double &ax, double &ay) const override;
  const Mob &quarry() const;
  void temper(StatusKind kind, float &duration, float &power) const override;
  void strike(float damage, double xd, double yd, float force,
              Upgrade *src = nullptr) override {
    hurt(damage, xd, yd, force, src);
  }
  Sprite body() const override { return sprite(); }
  void shield(float) override { traits.ward = std::max(traits.ward, 3); }
  float castTime = 5, wind = 0;
  const Cast *casts = nullptr, *winding = nullptr;
  int castCount = 0;
  Cast learned[2]{};
  const char *title = nullptr;
  const BossDef *def = nullptr;
  const NamedPack *pack = nullptr;
  int phase = 0;
  void makeBoss(float health, float damage);
  void learn(int count = 1);
  const char *arms(float pick) const;
  int skill(int slot) const;
  void crown(const char *name);
  void rule(const BossDef &d);
  void enrage();
  void channel();
  void think();
  void move();
  void push();
  void settle();
  float roll();
  void voice(Sound &s, float volume, float jitter = 0.06f) const;

protected:
  virtual Color tint() const { return col::white; }
  virtual void die();

private:
  void act();
  void land();
  void onHit(float damage, Upgrade *src);
  void adopt();
  Sense sense() const;
  void next(const Sense &s);
};
