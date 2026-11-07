#include "gui/Lang.hpp"

#include <algorithm>
#include <cstring>
#include <deque>

Lang lang;
Glossary glossary;

#define K(m) {#m, &Lang::m}
const LangKey LANG_KEYS[] = {
    K(world),

    K(paused), K(pauseResume), K(pauseCodex), K(pauseSettings), K(pauseQuit),

    K(settings), K(settingsHint), K(settingsPadHint), K(masterVolume), K(music), K(effects), K(ambience),
    K(screenShake), K(flashes), K(particles), K(damageNumbers), K(colours), K(fullscreen), K(scaling),
    K(vsync), K(controls), K(off), K(on), K(full), K(reduced), K(standard), K(deuteranopia), K(protanopia),
    K(tritanopia), K(monochrome), K(fill), K(pixelPerfect),

    K(gamepad), K(resetDefaults), K(pressButton), K(pressKey), K(newButton), K(padControlsHint), K(newKey),
    K(keyControlsHint),

    K(fallen), K(felledBy), K(overwhelmed), K(endlessRun), K(tip), K(riseAgain),

    K(dawnBreaks), K(conquered), K(anotherNight), K(keepGoing),

    K(adventurer), K(heatTag), K(rank), K(fieldClass), K(fieldDiff), K(fieldRan), K(fieldDmg), K(fieldFoes),
    K(fieldLit), K(fieldTime), K(omen), K(weaponDamage), K(achieved), K(metres), K(kilometres), K(tipMove),
    K(tipBoss), K(tipShrines), K(tipFocus), K(tipGaps), K(tipGems), K(tipEvolve),

    K(tagline), K(pressEnter), K(pressA), K(campaignSummary), K(hardcore), K(nightmare), K(campaignLine),
    K(continueWord), K(begin), K(titlePadHint), K(titleKeyHint), K(worldHeading), K(difficultyHeading),
    K(pastWorlds), K(typeCode), K(reroll), K(escBack), K(classic), K(random), K(daily), K(custom),
    K(classicBlurb), K(randomBlurb), K(dailyBlurb), K(customBlurb),

    K(level), K(kills), K(streak), K(dayClock), K(nightClock), K(endlessClock), K(dawnClock), K(shrineLit),
    K(shrinesLit), K(skillPoint), K(skillPoints), K(trade), K(merchant), K(merchantAt), K(craft), K(shrineAt),

    K(buildPrompt), K(caveIn), K(delveFloor), K(delveMine), K(caveDown), K(caveUp), K(caveDepths), K(caveSurface), K(relight), K(takeDown), K(gather), K(bridgeHolds),
    K(questLine), K(talk), K(help), K(helpAt), K(questAt),

    K(classTag), K(evolve), K(skillReplaces), K(none), K(relic), K(classTier), K(newTier), K(tierShort),
    K(evolvesWith), K(evolvesWithHeld), K(evolvesYour), K(roastFeast), K(restoreHealth), K(evolves), K(treasure),
    K(levelUp), K(levelUpHint),

    K(locked), K(chooseClass), K(dailyHint), K(heatHint), K(classHint), K(dailyLine), K(heatLocked),
    K(heatZero), K(heatLine), K(hardcoreOn), K(hardcoreOff),

    K(fury), K(furyGain), K(furyCost), K(roots), K(rootsGain), K(rootsCost), K(vision), K(visionGain),
    K(visionCost), K(keepFlame), K(keepFlameGain), K(noCost), K(shrineOffers), K(shrineHint),

    K(questMark), K(campaignHeading), K(townsLegend), K(campfiresLegend), K(minesLegend), K(worldMap),
    K(seed), K(bounties), K(regions), K(earlierRuns), K(walked), K(charted), K(mapHint),

    K(inventory), K(bagLine), K(inventoryHint), K(actAnnounce), K(waypoints), K(actRow), K(travelHint),

    K(chipHp), K(chipArmour), K(chipSpeed), K(chipDamage), K(chipCooldown), K(chipCrit), K(chipRegen),
    K(chipReach), K(favours), K(lockedClass), K(unlockNeed), K(cardWeapon), K(cardTrait), K(cardSkill),
    K(cardArsenal), K(cardNature),

    K(achievements), K(codexCount), K(codexRegions), K(unknown), K(relics), K(codex), K(codexLine),
    K(codexLineOne), K(bestiary), K(places), K(unknownSite), K(codexBack), K(page),

    K(nowSkill), K(evolvedNotice), K(rankNotice), K(skillTree), K(pointToSpend), K(pointsToSpend), K(tierRow),
    K(rankRow), K(makeSkill), K(enterEvolve), K(enterLearn), K(treeHint),

    K(healingDraught), K(gambleWare), K(gambled), K(soldFor), K(merchantTitle), K(freshWares), K(sold),
    K(yourBag), K(goldLine), K(bagCount), K(vendorHint), K(gambleHint), K(draughtHint),

    K(questOver), K(beginNightmare), K(escClose), K(acceptReward), K(claimReward), K(notDone), K(raiseTower),
    K(forgetPath),

    K(cannotTake), K(crafted), K(workbench), K(costs), K(nothing), K(readyRecipe), K(craftHint), K(stash),
    K(yourBagCount), K(stashCount), K(stashHint), K(needs), K(build), K(buildHint),

    K(againstWorn), K(loseStat), K(itemLevel), K(grants), K(curseLine), K(classOnly), K(costsGold),
    K(sellsFor),

    K(intro),

    K(nightmareReturns), K(reliquarySlips), K(wakeCampfire), K(wakeWaypoint), K(wakeLost), K(seekerJoins),
    K(hunterHound), K(criesForHelp), K(houndLimps), K(bastionHeld), K(houndBack), K(watchtowerRises),
    K(mineCavesIn), K(mineCleared), K(relightNeeds),

    K(questBanner), K(elderNeeds), K(questDone), K(rewardBanner), K(actCleared), K(darkBroken), K(returnTo),
    K(followTo), K(actBastion), K(nestsPulse), K(holdCamp), K(sealedReliquary), K(sealBreaks), K(imraFallen),
    K(imraSetsOut), K(seekerLost), K(seekerFollows), K(waypointKeeps),

    K(cyclopsFalls), K(isSlain), K(stirsNearby), K(hasFallen), K(enrages), K(thiefCaught), K(shadeFire),
    K(shadeTorch), K(merchantCamps), K(merchantRestocks), K(phoenixRises), K(newBest), K(omenBanner),
    K(darkDeepens), K(darkDeepensMany), K(nightNoEnd), K(hearthwickHolds), K(hearthwickStands),
    K(pathForgotten), K(levelPoint), K(levelPoints),

    K(treeRenews), K(powerRings), K(sporesMend), K(gateSilent), K(gateCarries), K(forgeNothing),
    K(forgeTempers), K(iceClaims), K(encounterTask), K(rewardColon), K(encounterLost), K(bountyBanner),
    K(nightFallsBridge), K(nightFalls), K(dawnBastion), K(renegades), K(championStalks),

    K(towerHas), K(towerNeeds), K(needsLower), K(buildWater), K(buildRuin), K(buildTaken), K(buildClear),
    K(questStirs), K(carryHome), K(questComplete), K(statusBroken), K(statusFollow), K(statusOffered),
    K(statusReady), K(statusEscort), K(statusHolding), K(statusReachCamp), K(statusCount), K(seekerLostLine),
    K(seekerFollowLine), K(bountySlay), K(bountyKindle), K(bountyFind), K(bountyWalk), K(bountySurvive),
    K(bountyStreak), K(bountyAlpha), K(bountyThief),

    K(itemBase), K(itemRare), K(itemPrefix), K(itemSuffix), K(itemSuperior), K(itemNight),

    K(pickOne), K(pickMore), K(noFit), K(pickDifferent), K(shareSlot), K(tooMany), K(recipeLine),

    K(languageHeader)};
#undef K
const int LANG_COUNT = int(sizeof LANG_KEYS / sizeof *LANG_KEYS);
static_assert(sizeof LANG_KEYS / sizeof *LANG_KEYS * sizeof(const char *) == sizeof(Lang));

namespace {
bool formats(std::string_view f) {
  try {
    (void)fmt::format(fmt::runtime(f), 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    return true;
  } catch (const fmt::format_error &) {
    return false;
  }
}
} // namespace

int Lang::parse(std::string_view text) {
  static std::deque<std::string> held;
  static const Lang english;
  int taken = 0;
  for (size_t at = 0; at < text.size();) {
    size_t end = std::min(text.find('\n', at), text.size());
    std::string_view line = text.substr(at, end - at);
    at = end + 1;
    if (!line.empty() && line.back() == '\r')
      line.remove_suffix(1);
    size_t gap = line.find(' ');
    if (gap == std::string_view::npos || line[0] == '#')
      continue;
    std::string_view key = line.substr(0, gap), value = line.substr(gap + 1);
    for (int i = 0; i < LANG_COUNT; i++) {
      const LangKey &k = LANG_KEYS[i];
      std::string_view was = english.*k.text;
      if (key != k.key || std::count(was.begin(), was.end(), '{') != std::count(value.begin(), value.end(), '{') ||
          !formats(value))
        continue;
      this->*k.text = held.emplace_back(value).c_str();
      taken++;
    }
  }
  return taken;
}

int Glossary::parse(std::string_view text) {
  int taken = 0;
  for (size_t at = 0; at < text.size();) {
    size_t end = std::min(text.find('\n', at), text.size());
    std::string_view line = text.substr(at, end - at);
    at = end + 1;
    if (!line.empty() && line.back() == '\r')
      line.remove_suffix(1);
    size_t gap = line.find('\t');
    if (line.size() < 2 || line[0] != '~' || gap == std::string_view::npos)
      continue;
    std::string_view english = line.substr(1, gap - 1), value = line.substr(gap + 1);
    if (english.empty() || value.empty() ||
        std::count(english.begin(), english.end(), '{') != std::count(value.begin(), value.end(), '{') ||
        (value.find('{') != std::string_view::npos && !formats(value)))
      continue;
    words[std::string(english)] = std::string(value);
    taken++;
  }
  return taken;
}

std::string langTemplate() {
  static const Lang english;
  std::string out = std::string(english.languageHeader) + "\n";
  for (int i = 0; i < LANG_COUNT; i++)
    out += std::string(LANG_KEYS[i].key) + " " + english.*LANG_KEYS[i].text + "\n";
  return out;
}
