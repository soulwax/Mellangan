"""Converts the Kenney OGGs the game uses into res/sfx as mono 44.1 kHz s16 WAV (what src/audio expects),
peak-normalized to -1 dBFS so the volumes set in code are the only balance that matters.
An SFX entry may carry an extra ffmpeg filter (pitch, trim, fade). Sounds the bundle has no source for
(the wolf howl) are synthesized here with the standard library only.
The music (res/music/night.ogg) is sourced separately; see res/CREDITS.md.

Usage: python tools/make_audio.py "<path to Kenney Game Assets All-in-1>"   (needs ffmpeg on PATH)
"""
import math
import random
import re
import struct
import subprocess
import sys
import wave
from pathlib import Path

RATE = 44100

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
    "erupt": ("Foley Sounds/stonesHit1", "aresample=44100,asetrate=37485,aresample=44100"),
    "raise": ("Casino Audio/dice-shake-1",
              "aresample=44100,asetrate=35280,aresample=44100,atrim=0:0.8,afade=t=out:st=0.5:d=0.3"),
}


def howl() -> list[float]:
    """A wolf howl: a breathy rise, a held note with slow vibrato, then a falling tail."""
    rng = random.Random(7)
    n = int(RATE * 1.15)
    out, phase, breath = [], 0.0, 0.0
    for i in range(n):
        t = i / RATE
        rise = min(1.0, t / 0.3)
        fall = max(0.0, (t - 0.75) / 0.4)
        f = 250 + 190 * (1 - (1 - rise) ** 2) - 150 * fall ** 1.5
        f *= 1 + 0.012 * math.sin(math.tau * 5.5 * t) * min(1.0, t / 0.4)
        phase += math.tau * f / RATE
        tone = math.sin(phase) + 0.28 * math.sin(2 * phase) + 0.08 * math.sin(3 * phase)
        breath = 0.9 * breath + 0.1 * rng.uniform(-1, 1)
        env = min(1.0, t / 0.12) * min(1.0, (1.15 - t) / 0.4)
        out.append(env * (tone + (1.2 - 0.8 * rise) * breath))
    return out


def seamless(samples: list[float], fade: int) -> list[float]:
    """Crossfades the tail into the head so the clip loops without a click."""
    body = samples[:-fade]
    for i in range(fade):
        k = i / fade
        body[i] = body[i] * k + samples[len(samples) - fade + i] * (1 - k)
    return body


def rain() -> list[float]:
    """A rain bed: soft high hiss with scattered droplet ticks, 6 s, loopable."""
    rng = random.Random(11)
    n = int(RATE * 6.5)
    out, low, prev = [], 0.0, 0.0
    ticks = sorted(rng.randrange(n) for _ in range(2600))
    tick, env, freq, phase = 0, 0.0, 0.0, 0.0
    for i in range(n):
        white = rng.uniform(-1, 1)
        low = 0.86 * low + 0.14 * white
        hiss = (white - low) * 0.35 + low * 0.25
        while tick < len(ticks) and ticks[tick] == i:
            env, freq, phase = rng.uniform(0.3, 1), rng.uniform(1800, 5200), 0.0
            tick += 1
        phase += math.tau * freq / RATE
        drop = env * math.sin(phase)
        env *= 0.992
        prev = 0.6 * prev + 0.4 * (hiss + drop * 0.5)
        out.append(prev)
    return seamless(out, int(RATE * 0.5))


def wind() -> list[float]:
    """A wind bed: low filtered rumble that swells twice per 8 s loop."""
    rng = random.Random(13)
    n = int(RATE * 8.5)
    out, a, b = [], 0.0, 0.0
    for i in range(n):
        t = i / RATE
        a = 0.995 * a + 0.005 * rng.uniform(-1, 1)
        b = 0.9 * b + 0.1 * rng.uniform(-1, 1)
        swell = 0.55 + 0.45 * math.sin(math.tau * t / 4.0) ** 2
        out.append(swell * (a * 9 + b * 0.25 * swell))
    return seamless(out, int(RATE * 0.5))


SYNTH = {"howl": howl, "rain": rain, "wind": wind}


def write(samples: list[float], dst: Path):
    dst.parent.mkdir(parents=True, exist_ok=True)
    peak = max(abs(s) for s in samples) or 1
    gain = 10 ** (-1 / 20) / peak
    with wave.open(str(dst), "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(RATE)
        w.writeframes(b"".join(struct.pack("<h", round(s * gain * 32767)) for s in samples))
    print(f"synth -> {dst.relative_to(dst.parents[2])}")


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


def convert(src: Path, dst: Path, shape: str = ""):
    dst.parent.mkdir(parents=True, exist_ok=True)
    tmp = dst.with_suffix(".tmp.wav")
    shaped = src
    if shape:
        subprocess.run(["ffmpeg", "-v", "error", "-y", "-i", str(src), "-ac", "1", "-af", shape, str(tmp)], check=True)
        shaped = tmp
    gain = -1.0 - peak_db(shaped)
    subprocess.run(["ffmpeg", "-v", "error", "-y", "-i", str(shaped), "-ac", "1", "-ar", "44100", "-sample_fmt",
                    "s16", "-af", f"volume={gain:.2f}dB", str(dst)], check=True)
    tmp.unlink(missing_ok=True)
    print(f"{src.name} -> {dst.relative_to(dst.parents[2])} ({gain:+.1f} dB)")


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    audio = Path(sys.argv[1]) / "Audio"
    res = Path(__file__).resolve().parent.parent / "res"
    for name, entry in SFX.items():
        rel, shape = entry if isinstance(entry, tuple) else (entry, "")
        convert(find(audio, rel), res / "sfx" / f"{name}.wav", shape)
    for name, make in SYNTH.items():
        write(make(), res / "sfx" / f"{name}.wav")


if __name__ == "__main__":
    main()
