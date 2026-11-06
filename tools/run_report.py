"""Summarise Mellangan's local run log (runs.csv in the save folder).

usage: python tools/run_report.py [runs.csv] [--mode run|campaign] [--min-runs N]

Without a path it reads %APPDATA%/soulwax/Mellangan/runs.csv (Windows) or the SDL pref path
on Linux and macOS. Weapons or classes far below the median are flagged, as P4.5 asks.
"""

import csv
import os
import statistics
import sys
from collections import Counter, defaultdict


def default_path():
    if os.name == "nt":
        return os.path.join(os.environ.get("APPDATA", ""), "soulwax", "Mellangan", "runs.csv")
    if sys.platform == "darwin":
        return os.path.expanduser("~/Library/Application Support/soulwax/Mellangan/runs.csv")
    base = os.environ.get("XDG_DATA_HOME", os.path.expanduser("~/.local/share"))
    return os.path.join(base, "soulwax", "Mellangan", "runs.csv")


def pairs(field):
    for part in filter(None, field.split("|")):
        bits = part.split(":")
        yield bits[0], bits[1:]


def clock(seconds):
    return f"{int(seconds) // 60:02}:{int(seconds) % 60:02}"


def main(argv):
    args, opts, i = [], {}, 0
    while i < len(argv):
        if argv[i].startswith("--") and i + 1 < len(argv):
            opts[argv[i]] = argv[i + 1]
            i += 2
        else:
            args.append(argv[i])
            i += 1
    path = args[0] if args else default_path()
    mode = opts.get("--mode")
    floor = int(opts.get("--min-runs", 3))
    with open(path, newline="", encoding="utf-8") as f:
        runs = [r for r in csv.DictReader(f) if not mode or r["mode"] == mode]
    if not runs:
        print(f"no runs in {path}")
        return 1

    dawns = sum(r["dawn"] == "1" for r in runs)
    print(f"{len(runs)} runs from {path}, {dawns} reached dawn ({100 * dawns / len(runs):.0f}%)")
    print(f"versions {', '.join(sorted({r['version'] for r in runs}))}")

    print("\nCLASSES                runs  dawn  median time  median level")
    by_hero = defaultdict(list)
    for r in runs:
        by_hero[r["hero"] or "?"].append(r)
    medians = {}
    for hero, rs in sorted(by_hero.items(), key=lambda kv: -len(kv[1])):
        t = statistics.median(float(r["seconds"]) for r in rs)
        medians[hero] = t
        lvl = statistics.median(int(r["level"]) for r in rs)
        rate = 100 * sum(r["dawn"] == "1" for r in rs) / len(rs)
        print(f"  {hero:<20}{len(rs):>5}{rate:>5.0f}%{clock(t):>13}{lvl:>15.0f}")
    overall = statistics.median(medians.values())
    weak = [h for h, t in medians.items() if len(by_hero[h]) >= floor and t < overall * 0.6]
    if weak:
        print(f"  far below the median time: {', '.join(weak)}")

    heats = defaultdict(list)
    for r in runs:
        heats[int(r.get("heat") or 0)].append(r)
    if len(heats) > 1 or 0 not in heats:
        print("\nHEAT     runs  dawn  median time")
        for h, rs in sorted(heats.items()):
            rate = 100 * sum(r["dawn"] == "1" for r in rs) / len(rs)
            print(f"  {h:<6}{len(rs):>5}{rate:>5.0f}%{clock(statistics.median(float(r['seconds']) for r in rs)):>13}")

    print("\nWEAPONS                picked  median share  median level")
    share = defaultdict(list)
    level = defaultdict(list)
    for r in runs:
        dealt = {name: (int(v[0]), float(v[1])) for name, v in pairs(r["weapons"]) if len(v) == 2}
        total = sum(d for _, d in dealt.values()) or 1
        for name, (lvl, d) in dealt.items():
            share[name].append(d / total)
            level[name].append(lvl)
    picked = {name: len(v) for name, v in share.items()}
    shares = {name: statistics.median(v) for name, v in share.items()}
    middle = statistics.median(shares.values()) if shares else 0
    for name in sorted(share, key=lambda n: -picked[n]):
        flag = "  <- far below the median" if picked[name] >= floor and shares[name] < middle * 0.4 else ""
        print(f"  {name:<22}{100 * picked[name] / len(runs):>5.0f}%{100 * shares[name]:>13.0f}%"
              f"{statistics.median(level[name]):>14.0f}{flag}")

    picks = Counter(name for r in runs for name in filter(None, r["picks"].split("|")))
    if picks:
        print("\nMOST PICKED UPGRADES")
        for name, n in picks.most_common(10):
            print(f"  {name:<28}{n:>5}")

    causes = Counter(r["cause"] for r in runs if r["dawn"] != "1")
    if causes:
        print("\nDEATHS")
        for cause, n in causes.most_common(10):
            print(f"  {cause or 'overwhelmed':<28}{n:>5}")
        times = [float(r["seconds"]) for r in runs if r["dawn"] != "1"]
        print(f"  median time of death {clock(statistics.median(times))}")

    curve = defaultdict(list)
    for r in runs:
        for minute, lvl in enumerate(filter(None, r["levels"].split("|")), 1):
            curve[minute].append(int(lvl))
    if curve:
        print("\nLEVEL CURVE (median level at each minute)")
        print("  " + "  ".join(f"{m}:{statistics.median(v):.0f}" for m, v in sorted(curve.items())))

    biomes = Counter()
    for r in runs:
        for name, v in pairs(r["biomes"]):
            if v:
                biomes[name] += float(v[0])
    if biomes:
        total = sum(biomes.values())
        print("\nTIME PER BIOME")
        for name, s in biomes.most_common():
            print(f"  {name:<22}{100 * s / total:>5.0f}%")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
