"""Converts the Kenney OGGs the game uses into res/sfx as mono 44.1 kHz s16 WAV (what src/audio expects),
peak-normalized to -1 dBFS so the volumes set in code are the only balance that matters.
The music (res/music/night.ogg) is sourced separately; see res/CREDITS.md.

Usage: python tools/make_audio.py "<path to Kenney Game Assets All-in-1>"   (needs ffmpeg on PATH)
"""
import re
import subprocess
import sys
from pathlib import Path

SFX = {
    "cast": "Retro Sounds 2/laser1",
    "hit": "Impact Sounds/impactPunch_medium_000",
    "kill": "Retro Sounds 2/hit2",
    "slash": "RPG Audio/knifeSlice",
    "axe": "RPG Audio/drawKnife2",
    "gem": "Retro Sounds 2/coin1",
    "levelup": "Retro Sounds 2/upgrade1",
    "hurt": "Retro Sounds 2/hurt2",
    "death": "Retro Sounds 2/gameover2",
    "chest": "Retro Sounds 2/secret2",
    "potion": "Retro Sounds 2/upgrade3",
    "select": "Interface Sounds/select_002",
    "confirm": "Interface Sounds/confirmation_002",
    "boss": "Impact Sounds/impactBell_heavy_000",
    "horde": "Impact Sounds/impactBell_heavy_002",
    "thunder": "Retro Sounds 2/explosion1",
    "explosion": "Retro Sounds 2/explosion3",
    "bow": "RPG Audio/drawKnife1",
    "scythe": "RPG Audio/knifeSlice2",
    "magnet": "Retro Sounds 2/secret1",
    "nova": "Retro Sounds 2/explosion5",
    "barrel": "Impact Sounds/impactWood_heavy_000",
    "orb": "Retro Sounds 2/laser3",
}


def find(audio: Path, rel: str) -> Path:
    pack, name = rel.split("/")
    hits = list((audio / pack).rglob(f"{name}.ogg"))
    if not hits:
        sys.exit(f"missing {rel}.ogg")
    return hits[0]


def peak_db(src: Path) -> float:
    out = subprocess.run(["ffmpeg", "-hide_banner", "-i", str(src), "-ac", "1", "-af", "volumedetect", "-f", "null", "-"],
                         capture_output=True, text=True, check=True).stderr
    return float(re.search(r"max_volume: (-?[\d.]+) dB", out).group(1))


def convert(src: Path, dst: Path):
    dst.parent.mkdir(parents=True, exist_ok=True)
    gain = -1.0 - peak_db(src)
    subprocess.run(["ffmpeg", "-v", "error", "-y", "-i", str(src), "-ac", "1", "-ar", "44100", "-sample_fmt", "s16",
                    "-af", f"volume={gain:.2f}dB", str(dst)], check=True)
    print(f"{src.name} -> {dst.relative_to(dst.parents[2])} ({gain:+.1f} dB)")


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    audio = Path(sys.argv[1]) / "Audio"
    res = Path(__file__).resolve().parent.parent / "res"
    for name, rel in SFX.items():
        convert(find(audio, rel), res / "sfx" / f"{name}.wav")


if __name__ == "__main__":
    main()
