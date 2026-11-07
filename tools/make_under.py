"""Adds the underground band to res/atlas.png: one row of hand-coded pixel art for the Caves and the Sundark.

Cells, in order: chasm void, cracked wall, hanging roots, vent chimney, sinkhole light, ore vein.
Usage: uv run --with pillow tools/make_under.py
make_assets.py calls add_under() too, so a full rebuild keeps the band. Layout must match atlas::under in src/Atlas.hpp.
"""
import math
from pathlib import Path

from PIL import Image

from atlas_layout import BANDS, extend, place_cell

UNDER = next(b for b in BANDS if b.name == "under")

OUTLINE, DARK, MID, LIGHT = (26, 20, 30), (48, 40, 54), (76, 68, 84), (116, 106, 122)
PALE = (168, 158, 166)
VOID_LOW, VOID_HIGH, GLINT = (6, 5, 12), (20, 13, 34), (120, 82, 196)
ROOT_DARK, ROOT, ROOT_LIGHT = (58, 38, 28), (98, 66, 42), (146, 104, 64)
BASALT, BASALT_RIM, EMBER, EMBER_HOT, SMOKE = (40, 34, 42), (88, 76, 84), (214, 92, 40), (252, 176, 80), (150, 142, 150)
DAY, ROPE, ROPE_DARK = (255, 242, 204), (162, 120, 74), (104, 72, 46)
GOLD, COPPER = (244, 196, 86), (206, 122, 54)


def hashed(x, y, salt):
    h = (x * 374761393 + y * 668265263 + salt * 2246822519) & 0xFFFFFFFF
    h = ((h ^ (h >> 13)) * 1274126177) & 0xFFFFFFFF
    return (h ^ (h >> 16)) / 0xFFFFFFFF


def blank():
    return Image.new("RGBA", (16, 16), (0, 0, 0, 0))


def mix(a, b, t):
    return tuple(int(round(x + (y - x) * t)) for x, y in zip(a, b))


def void():
    im = blank()
    px = im.load()
    for y in range(16):
        for x in range(16):
            t = 0.25 + 0.5 * hashed(x // 2, y // 2, 3) + 0.25 * hashed(x, y, 5)
            px[x, y] = (*mix(VOID_LOW, VOID_HIGH, t * t), 255)
            if hashed(x, y, 7) < 0.025:
                px[x, y] = (*GLINT, 255)
            elif hashed(x, y, 9) < 0.03:
                px[x, y] = (*mix(VOID_HIGH, GLINT, 0.4), 255)
    return im


def crack():
    im = blank()
    px = im.load()
    x = 8
    for y in range(0, 12):
        x = max(5, min(10, x + (1 if hashed(0, y, 11) < 0.35 else -1 if hashed(0, y, 13) < 0.35 else 0)))
        px[x, y] = (*OUTLINE, 255)
        px[x - 1, y] = (*PALE, 255)
        px[x + 1, y] = (*LIGHT, 255)
        if y % 3 == 0:
            px[x - 2, y] = (*LIGHT, 200)
        if y in (4, 8):
            for k in range(1, 3):
                px[min(15, x + 1 + k), y + k - 1] = (*OUTLINE, 255)
    for y in range(12, 16):
        for x in range(2, 14):
            r = hashed(x, y, 17)
            if r < 0.22 + 0.12 * (y - 12):
                px[x, y] = (*(PALE if r < 0.08 else LIGHT if r < 0.18 else MID), 255)
    return im


def roots():
    im = blank()
    px = im.load()
    for k, (x0, length, phase) in enumerate(((3, 13, 0.0), (6, 15, 1.7), (9, 12, 3.1), (12, 14, 4.4))):
        for y in range(length):
            x = int(round(x0 + 1.2 * math.sin(phase + y * 0.55)))
            thick = 2 if y < length - 5 else 1
            for t in range(thick):
                px[min(15, x + t), y] = (*(ROOT_LIGHT if t == 0 and y % 3 else ROOT), 255)
            if thick == 2:
                px[max(0, x - 1), y] = (*ROOT_DARK, 255)
            if y > 3 and hashed(k, y, 19) < 0.12:
                px[min(15, x + 2), y + 1 if y < 15 else y] = (*ROOT_DARK, 255)
    return im


def chimney():
    im = blank()
    px = im.load()
    for y in range(8, 16):
        for x in range(16):
            dx, dy = (x - 7.5) / 7.5, (y - 12) / 4
            d = dx * dx + dy * dy
            if d > 1:
                continue
            inner = (x - 7.5) ** 2 / 16 + (y - 11.5) ** 2 / 3
            if inner < 1:
                px[x, y] = (*mix(EMBER_HOT, EMBER, min(1, inner * 1.2)), 255)
            elif d > 0.75 or y < 10:
                px[x, y] = (*BASALT_RIM, 255)
            else:
                px[x, y] = (*BASALT, 255)
    for y in range(0, 9):
        for x in range(4, 12):
            wave = 7.5 + 2 * math.sin(y * 0.8)
            if abs(x - wave) < 1.6 - y * 0.08 and hashed(x, y, 23) < 0.75:
                px[x, y] = (*SMOKE, 80 + y * 12)
    return im


def sinkhole():
    im = blank()
    px = im.load()
    for y in range(16):
        for x in range(3, 13):
            edge = 1 - abs(x - 7.5) / 5
            px[x, y] = (*DAY, int(30 + 70 * edge * (0.4 + 0.6 * y / 15)))
    for y in range(0, 14):
        x = 8 + (1 if y % 6 > 3 else 0)
        px[x, y] = (*(ROPE_DARK if y % 4 == 3 else ROPE), 255)
    for x in range(4, 12):
        px[x, 15] = (*DAY, 190)
        if 5 <= x <= 10:
            px[x, 14] = (*DAY, 150)
    return im


def ore():
    im = blank()
    px = im.load()
    for y in range(5, 16):
        for x in range(1, 15):
            dx, dy = (x - 7.5) / 6.8, (y - 11) / 5.4
            d = dx * dx + dy * dy
            if d > 1:
                continue
            tone = OUTLINE if d > 0.8 else LIGHT if dy < -0.35 and dx < 0.2 else MID if dy < 0.3 else DARK
            px[x, y] = (*tone, 255)
            seam = abs((x - 3) - (y - 6) * 1.1) < 1.6
            if d < 0.8 and (seam and hashed(x, y, 29) < 0.7 or hashed(x, y, 31) < 0.06):
                px[x, y] = (*(GOLD if hashed(x, y, 37) < 0.55 else COPPER), 255)
    return im


SPRITES = (void, crack, roots, chimney, sinkhole, ore)


def add_under(atlas):
    out = extend(atlas)
    for index, paint in enumerate(SPRITES):
        place_cell(out, UNDER, index, paint())
    return out


if __name__ == "__main__":
    path = Path(__file__).resolve().parent.parent / "res" / "atlas.png"
    add_under(Image.open(path).convert("RGBA")).save(path)
    print(f"wrote {path}")
