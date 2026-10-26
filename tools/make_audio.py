"""Converts the Kenney OGGs the game uses into res/sfx and res/music as mono 44.1 kHz s16 WAV (what src/audio expects).

Usage: python tools/make_audio.py "<path to Kenney Game Assets All-in-1>"   (needs ffmpeg on PATH)
"""
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
}
MUSIC = {"night": "Music Loops/Retro Mystic"}


def find(audio: Path, rel: str) -> Path:
    pack, name = rel.split("/")
    hits = list((audio / pack).rglob(f"{name}.ogg"))
    if not hits:
        sys.exit(f"missing {rel}.ogg")
    return hits[0]


def convert(src: Path, dst: Path):
    dst.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run(["ffmpeg", "-v", "error", "-y", "-i", str(src), "-ac", "1", "-ar", "44100", "-sample_fmt", "s16",
                    str(dst)], check=True)
    print(f"{src.name} -> {dst.relative_to(dst.parents[2])}")


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    audio = Path(sys.argv[1]) / "Audio"
    res = Path(__file__).resolve().parent.parent / "res"
    for name, rel in SFX.items():
        convert(find(audio, rel), res / "sfx" / f"{name}.wav")
    for name, rel in MUSIC.items():
        convert(find(audio, rel), res / "music" / f"{name}.wav")


if __name__ == "__main__":
    main()
