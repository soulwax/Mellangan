#pragma once

#include <spdlog/fmt/fmt.h>

#include <string>
#include <string_view>
#include <utility>

struct Lang {
  const char *world = "world {}";

  const char *paused = "PAUSED", *pauseResume = "Enter / Esc to resume", *pauseCodex = "C codex",
             *pauseSettings = "O settings", *pauseQuit = "Q to quit to title";

  const char *settings = "SETTINGS", *settingsHint = "Up/Down choose   Left/Right change   Esc back",
             *settingsPadHint = "D-pad choose and change   B back", *masterVolume = "MASTER VOLUME",
             *music = "MUSIC", *effects = "EFFECTS", *ambience = "AMBIENCE", *screenShake = "SCREEN SHAKE",
             *flashes = "FLASHES", *particles = "PARTICLES", *damageNumbers = "DAMAGE NUMBERS",
             *colours = "COLOURS", *fullscreen = "FULLSCREEN", *scaling = "SCALING", *vsync = "VSYNC",
             *controls = "CONTROLS", *off = "OFF", *on = "ON", *full = "FULL", *reduced = "REDUCED",
             *standard = "STANDARD", *deuteranopia = "DEUTERANOPIA",
             *protanopia = "PROTANOPIA", *tritanopia = "TRITANOPIA", *monochrome = "MONOCHROME", *fill = "FILL",
             *pixelPerfect = "PIXEL PERFECT";

  const char *gamepad = "GAMEPAD", *resetDefaults = "RESET TO DEFAULTS", *pressButton = "PRESS A BUTTON",
             *pressKey = "PRESS A KEY", *newButton = "press the new button   B cancel",
             *padControlsHint = "D-pad choose   A rebind   B back   A, B, Start and the D-pad stay fixed",
             *newKey = "press the new key   Esc cancel",
             *keyControlsHint = "Up/Down choose   Enter rebind   Esc back   arrows, Enter, Esc, 1-4, Q stay fixed";

  const char *fallen = "YOU HAVE FALLEN", *felledBy = "felled by {}", *overwhelmed = "overwhelmed by the dark",
             *endlessRun = "{} into the endless night   best {}", *tip = "TIP  {}",
             *riseAgain = "R / Enter to rise again";

  const char *dawnBreaks = "DAWN BREAKS", *conquered = "The night was conquered at {}",
             *anotherNight = "R / Enter for another night",
             *keepGoing = "K  keep going into the endless night";

  const char *adventurer = "Adventurer", *heatTag = "  HEAT {}", *rank = "RANK", *fieldClass = "CLASS",
             *fieldDiff = "DIFF", *fieldRan = "RAN", *fieldDmg = "DMG", *fieldFoes = "FOES", *fieldLit = "LIT",
             *fieldTime = "TIME", *omen = "under the omen {}", *weaponDamage = "WEAPON DAMAGE",
             *achieved = "ACHIEVED  {}", *metres = "{} m", *kilometres = "{:.2f} km",
             *tipMove = "Keep moving from the first second: foes close in on a hero who stands still",
             *tipBoss = "Bosses telegraph their heaviest blows: step out of the warning ring before it fills",
             *tipShrines = "Kindle shrines: every shrine lit brings the dawn sooner",
             *tipFocus = "Raise one weapon to level 5 before spreading out: maxed weapons can evolve",
             *tipGaps = "Keep moving: walking through the gaps keeps the crowd from closing the circle",
             *tipGems = "Gather the gems: a wider magnet and more experience level you faster",
             *tipEvolve = "Pair a maxed weapon with its partner relic to evolve it";

  const char *tagline = "kindle the shrines, outlast the night", *pressEnter = "PRESS ENTER",
             *pressA = "PRESS A", *campaignSummary = "{}  level {}  act {}  day {}", *hardcore = "  HARDCORE",
             *nightmare = "  NIGHTMARE {}", *campaignLine = "{}  {} THE CAMPAIGN", *continueWord = "CONTINUE",
             *begin = "BEGIN",
             *titlePadHint = "Stick/D-pad move   Back map   Y codex   RB settings   Start pause",
             *titleKeyHint = "WASD/Arrows move   Tab map   C codex   O settings   Esc pause   M mute   F11 fullscreen",
             *worldHeading = "WORLD", *difficultyHeading = "DIFFICULTY", *pastWorlds = "PAST WORLDS",
             *typeCode = "type a code", *reroll = "R reroll", *escBack = "Esc back", *classic = "CLASSIC",
             *random = "RANDOM", *daily = "DAILY", *custom = "CUSTOM", *classicBlurb = "the same land every night",
             *randomBlurb = "a new land each run", *dailyBlurb = "one land for everyone today",
             *customBlurb = "type a code to share";

  const char *level = "LV {}", *kills = "{} kills", *streak = "x{} STREAK", *dayClock = "DAY {}  DUSK {}",
             *nightClock = "NIGHT {}  DAWN {}", *endlessClock = "ENDLESS {}", *dawnClock = "DAWN {}",
             *shrineLit = "{} shrine lit", *shrinesLit = "{} shrines lit", *skillPoint = "K  {} skill point",
             *skillPoints = "K  {} skill points", *trade = "I  TRADE", *merchant = "MERCHANT",
             *merchantAt = "MERCHANT {:.0f}", *craft = "I  CRAFT", *shrineAt = "SHRINE {:.0f}";

  const char *buildPrompt = "{}  Enter build  Esc cancel", *caveIn = "THE CAVE-IN SETTLES  {} FOES",
             *delveFloor = "DELVE  FLOOR {}/{}  {} FOES", *delveMine = "E  DELVE INTO THE MINE",
             *relight = "E  RELIGHT  {} wood", *takeDown = "E  TAKE DOWN", *gather = "E  {}",
             *bridgeHolds = "THE BRIDGE HOLDS  {} FALLEN", *questLine = "QUEST  {}", *talk = "I  TALK",
             *help = "HELP!", *helpAt = "HELP {:.0f}", *questAt = "QUEST {:.0f}";

  const char *classTag = "CLASS", *evolve = "EVOLVE", *skillReplaces = "SKILL  REPLACES {}", *none = "NONE",
             *relic = "RELIC", *classTier = "{}  TIER 1", *newTier = "TIER 1 (NEW)", *tierShort = "T{}",
             *evolvesWith = "evolves with {}", *evolvesWithHeld = "evolves with {}, which you hold",
             *evolvesYour = "evolves your {}", *roastFeast = "Roast Feast", *restoreHealth = "Restore 40 health",
             *evolves = "{} evolves: double damage, faster", *treasure = "TREASURE!", *levelUp = "LEVEL UP!",
             *levelUpHint = "1-{} / Arrows + Enter / Click";

  const char *locked = "???", *chooseClass = "CHOOSE YOUR CLASS", *dailyHint = "Enter begin   Esc back",
             *heatHint = "Left/Right choose   Up/Down heat   Enter begin   Esc back",
             *classHint = "Left/Right choose   Enter begin   Esc back",
             *dailyLine = "DAILY  {} AT HEAT {}   the same run for everyone today",
             *heatLocked = "HEAT unlocks at your first dawn",
             *heatZero = "HEAT 0   Up/Down to raise the stakes", *heatLine = "HEAT {}   {}",
             *hardcoreOn = "R  HARDCORE: one life, death deletes the hero", *hardcoreOff = "R  HARDCORE: off";

  const char *fury = "FURY", *furyGain = "+20% damage", *furyCost = "-10 MAX HP", *roots = "ROOTS",
             *rootsGain = "+30 maximum health", *rootsCost = "-6 SPEED", *vision = "VISION",
             *visionGain = "Gain a level", *visionCost = "-1 ARMOUR", *keepFlame = "KEEP THE FLAME",
             *keepFlameGain = "Leave the shrine undisturbed", *noCost = "NO COST",
             *shrineOffers = "THE SHRINE OFFERS A BARGAIN",
             *shrineHint = "1-3 choose  /  Arrows + Enter  /  Esc keep the flame";

  const char *questMark = "QUEST", *campaignHeading = "CAMPAIGN", *townsLegend = "towns and waypoints",
             *campfiresLegend = "your campfires", *minesLegend = "cleared mines  {}", *worldMap = "WORLD MAP",
             *seed = "SEED {}", *bounties = "BOUNTIES", *regions = "REGIONS", *earlierRuns = "EARLIER RUNS",
             *walked = "{} of {} walked", *charted = "{} tiles charted",
             *mapHint = "Arrows pan   1-3 zoom   Enter centre   Tab / Esc close";

  const char *inventory = "INVENTORY", *bagLine = "{} gold   bag {}/{}",
             *inventoryHint = "Arrows or mouse pick   Enter equip   Backspace drop   I close",
             *actAnnounce = "ACT {}  {}", *waypoints = "WAYPOINTS", *actRow = "ACT {}   {}",
             *travelHint = "Up/Down pick   Enter travel   I close";

  const char *chipHp = "{:+} HP", *chipArmour = "ARMOUR +{}", *chipSpeed = "SPEED", *chipDamage = "DAMAGE",
             *chipCooldown = "COOLDOWN", *chipCrit = "CRIT", *chipRegen = "REGEN {:.1f}/S",
             *chipReach = "REACH +{}", *favours = "favours", *lockedClass = "LOCKED",
             *unlockNeed = "Unlock: {}", *cardWeapon = "WEAPON", *cardTrait = "TRAIT", *cardSkill = "SKILL",
             *cardArsenal = "ARSENAL", *cardNature = "NATURE";

  const char *achievements = "ACHIEVEMENTS  {} / {}", *codexCount = "{} {}/{}", *codexRegions = "REGIONS",
             *unknown = "? ? ?", *relics = "RELICS", *codex = "CODEX",
             *codexLine = "{} runs   {} dawns   best {}   best streak {}   {} slain",
             *codexLineOne = "{} runs   {} dawn   best {}   best streak {}   {} slain",
             *bestiary = "BESTIARY", *places = "PLACES", *unknownSite = "UNKNOWN SITE",
             *codexBack = "C / Esc back", *page = "PAGE {} / {}";

  const char *nowSkill = "{} is now your skill", *evolvedNotice = "{} evolved", *rankNotice = "{} rank {}",
             *skillTree = "{}  SKILL TREE", *pointToSpend = "{} point to spend",
             *pointsToSpend = "{} points to spend", *tierRow = "TIER {}  level {}",
             *rankRow = "{}   rank {}/{}",
             *makeSkill = "Enter: make it your skill, or rank it up with a point",
             *enterEvolve = "Enter: evolve", *enterLearn = "Enter: learn",
             *treeHint = "Arrows or mouse pick   Enter learn   K close";

  const char *healingDraught = "Healing Draught", *gambleWare = "? {}", *gambled = "gambled: {}",
             *soldFor = "sold for {} gold", *merchantTitle = "MERCHANT",
             *freshWares = "fresh wares at every lit shrine and at dawn", *sold = "SOLD",
             *yourBag = "YOUR BAG", *goldLine = "{} gold", *bagCount = "bag {}/{}",
             *vendorHint = "Arrows or mouse pick   Enter buy or sell   I close",
             *gambleHint = "A gamble rolls magic, rare or unique - rares twice and uniques three times as often",
             *draughtHint = "Restores {:.0f} health at once";

  const char *questOver = "It is over. The dark is broken, and the shrines will hold.",
             *beginNightmare = "Enter begin Nightmare: every foe stronger", *escClose = "Esc close",
             *acceptReward = "Enter accept   reward {}", *claimReward = "Enter claim {}",
             *notDone = "Esc close   the task is not done yet",
             *raiseTower = "B  raise a watchtower: {} gold, {} stone, {} wood",
             *forgetPath = "R  forget your path and relearn it: {} gold";

  const char *cannotTake = "{} cannot take that item", *crafted = "{}: {}", *workbench = "WORKBENCH",
             *costs = "{}  costs  ", *nothing = "nothing", *readyRecipe = "ready - Enter on the recipe",
             *craftHint = "pick items in the bag, then Enter on the recipe   I close", *stash = "STASH",
             *yourBagCount = "YOUR BAG {}/{}", *stashCount = "STASH {}/{}",
             *stashHint = "Enter moves an item across   I close   the stash stays in town",
             *needs = "needs {} {}", *build = "BUILD",
             *buildHint = "Up/Down pick   Enter place it in front of you   B close";

  const char *againstWorn = "against what you wear:", *loseStat = "lose {}", *itemLevel = "{}  item level {}",
             *grants = "grants {}", *curseLine = "{}: {}", *classOnly = "{} only",
             *costsGold = "costs {} gold", *sellsFor = "sells for {} gold";

  const char *intro = "INTRO";

  const char *nightmareReturns = "NIGHTMARE {}  THE DARK RETURNS STRONGER",
             *reliquarySlips = "THE RELIQUARY SLIPS AWAY  IT WAITS IN THE RUINS",
             *wakeCampfire = "YOU WAKE BY YOUR CAMPFIRE", *wakeWaypoint = "YOU WAKE BY THE WAYPOINT",
             *wakeLost = "{}  -{} GOLD",
             *seekerJoins = "A SANCTUARY SEEKER JOINS THE BASTION  {} SHELTERED  THE MERCHANT IS {}% KINDER",
             *hunterHound = "A GRATEFUL HUNTER GIVES YOU HIS HOUND",
             *criesForHelp = "SOMEONE CRIES FOR HELP OUT THERE",
             *houndLimps = "YOUR HOUND LIMPS HOME  IT RETURNS AT DAWN",
             *bastionHeld = "THE BASTION HELD  {} FELL AT THE BRIDGE  +{} GOLD",
             *houndBack = "YOUR HOUND IS BACK AT YOUR SIDE", *watchtowerRises = "A WATCHTOWER RISES OVER {}",
             *mineCavesIn = "THE MINE CAVES IN BEHIND YOU", *mineCleared = "THE MINE IS CLEARED",
             *relightNeeds = "RELIGHTING NEEDS {} WOOD";

  const char *questBanner = "QUEST  {}", *elderNeeds = "{} NEEDS {} {}", *questDone = "{} DONE  +{} GOLD",
             *rewardBanner = "REWARD  {}", *actCleared = "ACT {} CLEARED", *darkBroken = "THE DARK IS BROKEN",
             *returnTo = "RETURN TO {}", *followTo = "FOLLOW {} TO {}",
             *actBastion = "ACT {}  {}  A BASTION STILL STANDING",
             *nestsPulse = "THE NESTS PULSE  BURN THEM OUT", *holdCamp = "HOLD THE CAMP",
             *sealedReliquary = "A SEALED RELIQUARY  BREAK ITS KEEPER", *sealBreaks = "THE SEAL BREAKS",
             *imraFallen = "SISTER IMRA HAS FALLEN  RETURN TO THE KEEPER",
             *imraSetsOut = "SISTER IMRA SETS OUT  STAY CLOSE",
             *seekerLost = "A SANCTUARY SEEKER IS LOST TO THE DARK",
             *seekerFollows = "A SANCTUARY SEEKER FOLLOWS YOU  BRING THEM TO A BASTION",
             *waypointKeeps = "THE WAYPOINT KEEPS YOUR PATH";

  const char *cyclopsFalls = "THE CYCLOPS FALLS", *isSlain = "{} IS SLAIN", *stirsNearby = "{} STIRS NEARBY",
             *hasFallen = "{} HAS FALLEN", *enrages = "{} ENRAGES", *thiefCaught = "THIEF CAUGHT!",
             *shadeFire = "A SHADE SMOTHERS A FIRE", *shadeTorch = "A SHADE SNUFFS A TORCH",
             *merchantCamps = "A MERCHANT CAMPS BY THE FIRE",
             *merchantRestocks = "THE MERCHANT RESTOCKS AT DAWN", *phoenixRises = "THE PHOENIX RISES",
             *newBest = "NEW BEST", *omenBanner = "{}  {}", *darkDeepens = "THE DARK DEEPENS  {} MINUTE",
             *darkDeepensMany = "THE DARK DEEPENS  {} MINUTES", *nightNoEnd = "THE NIGHT DOES NOT END",
             *hearthwickHolds = "ACT I  HEARTHWICK  THE BASTION HOLDS",
             *hearthwickStands = "HEARTHWICK STILL STANDS",
             *pathForgotten = "YOUR PATH IS FORGOTTEN  {} SKILL POINTS",
             *levelPoint = "LEVEL {}  +{} SKILL POINT  K TO SPEND",
             *levelPoints = "LEVEL {}  +{} SKILL POINTS  K TO SPEND";

  const char *treeRenews = "THE TREE RENEWS YOU: +15 MAX HEALTH",
             *powerRings = "YOUR POWER RINGS: SKILL READY, +5% MIGHT",
             *sporesMend = "THE SPORES MEND YOU: +0.4 REGEN/S", *gateSilent = "THE GATE IS SILENT",
             *gateCarries = "THE GATE CARRIES YOU TO A SLEEPING SHRINE",
             *forgeNothing = "THE FORGE HAS NOTHING TO TEMPER", *forgeTempers = "THE FORGE TEMPERS {}",
             *iceClaims = "THE ICE CLAIMS YOU: +0.5 ARMOUR, A FREEZING TRAIL", *encounterTask = "{}: {}",
             *rewardColon = "REWARD: {}", *encounterLost = "{} LOST", *bountyBanner = "BOUNTY: {}",
             *nightFallsBridge = "NIGHT FALLS  HOLD THE BRIDGE", *nightFalls = "NIGHT FALLS",
             *dawnBastion = "DAWN  THE BASTION STILL STANDS", *renegades = "RENEGADES ON YOUR TRAIL",
             *championStalks = "A CHAMPION STALKS: {}";

  const char *towerHas = "this town already has its watchtower",
             *towerNeeds = "a watchtower needs {} gold, {} stone and {} wood", *needsLower = "needs {} {}",
             *buildWater = "cannot build on water or stairs", *buildRuin = "too close to a ruin or house",
             *buildTaken = "something is already built here", *buildClear = "clear this spot first (E)",
             *questStirs = "{} STIRS", *carryHome = "CARRY IT HOME TO {}",
             *questComplete = "{} COMPLETE  RETURN TO {}", *statusBroken = "The dark is broken",
             *statusFollow = "Follow {} to {}", *statusOffered = "{} in {} has work for you",
             *statusReady = "{}: return to {}", *statusEscort = "{}  {}%", *statusHolding = "{}  {}/{} s",
             *statusReachCamp = "{}: reach the camp", *statusCount = "{}  {}/{}",
             *seekerLostLine = "Please! Is the bastion still standing?",
             *seekerFollowLine = "Take me to the walls. I'll help hold them.", *bountySlay = "Slay {} {}",
             *bountyKindle = "Kindle a shrine in {}", *bountyFind = "Find a {}",
             *bountyWalk = "Walk {} regions", *bountySurvive = "Survive until {}",
             *bountyStreak = "Reach a {} kill streak", *bountyAlpha = "Slay the alpha",
             *bountyThief = "Catch a thief";

  int parse(std::string_view text);
};

extern Lang lang;

struct LangKey {
  const char *key;
  const char *Lang::*text;
};

extern const LangKey LANG_KEYS[];
extern const int LANG_COUNT;

template <class... A> std::string say(const char *f, A &&...a) {
  return fmt::format(fmt::runtime(f), std::forward<A>(a)...);
}
