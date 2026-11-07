"""Adds the Lantern Synod band to res/atlas.png: six hooded figures in grey-white habits, drawn in code.

Cells, in order: Lamplighter, Penitent, Censer, Limer, Confessor, Prioress.
Usage: uv run --with pillow tools/make_synod.py
make_assets.py calls add_synod() too. Layout must match atlas::synod in src/Atlas.hpp.
"""
from pathlib import Path

from PIL import Image

from atlas_layout import BANDS, extend, place_cell

SYNOD = next(b for b in BANDS if b.name == "synod")

LINE, HABIT_DARK, HABIT, HABIT_LIGHT = (38, 34, 44), (150, 146, 150), (198, 194, 192), (232, 228, 222)
SHADOW, SKIN = (54, 46, 58), (214, 170, 138)
BRASS, FLAME, FLAME_HOT, ROPE = (196, 150, 72), (246, 152, 52), (255, 226, 132), (120, 88, 60)
BLOOD, LIME, SMOKE, INK, GOLD = (168, 40, 44), (244, 244, 232), (210, 210, 214), (70, 52, 46), (236, 196, 92)


def blank():
    return Image.new("RGBA", (16, 16), (0, 0, 0, 0))


def body(px, tall=0, crown=None):
    top = 2 - tall
    for y in range(top, 16):
        if y < top + 4:
            half = 2 if y == top else 3
        else:
            half = min(5, 3 + (y - top - 4) // 3)
        for x in range(8 - half, 8 + half):
            edge = x == 8 - half or x == 7 + half or y == 15
            tone = LINE if edge else HABIT_LIGHT if x < 7 else HABIT if x < 9 else HABIT_DARK
            px[x, y] = (*tone, 255)
    for y in range(top + 2, top + 4):
        for x in range(6, 10):
            px[x, y] = (*SHADOW, 255)
    px[7, top + 3] = (*SKIN, 255)
    px[8, top + 3] = (*SKIN, 255)
    for x in range(5, 11):
        px[x, top + 8] = (*ROPE, 255)
    if crown:
        for x in range(6, 10):
            px[x, top - 1] = (*crown, 255)
        px[7, top - 2] = (*crown, 255)


def lamplighter():
    im = blank()
    px = im.load()
    body(px)
    for y in range(1, 16):
        px[13, y] = (*INK, 255)
    for y in range(0, 4):
        for x in range(12, 15):
            px[x, y] = (*BRASS, 255)
    px[13, 1] = (*FLAME_HOT, 255)
    px[13, 2] = (*FLAME, 255)
    return im


def penitent():
    im = blank()
    px = im.load()
    body(px)
    for y in range(7, 14):
        if y % 2:
            px[6, y] = (*BLOOD, 255)
            px[9, y + 1 if y < 15 else y] = (*BLOOD, 255)
    for k, y in enumerate(range(5, 11)):
        px[2 + k % 2, y] = (*ROPE, 255)
    px[2, 11] = (*BLOOD, 255)
    px[3, 12] = (*BLOOD, 255)
    return im


def censer():
    im = blank()
    px = im.load()
    body(px)
    for y in range(6, 11):
        px[12, y] = (*INK, 200)
    for y in range(11, 14):
        for x in range(11, 14):
            px[x, y] = (*BRASS, 255)
    px[12, 12] = (*FLAME, 255)
    for k, (x, y) in enumerate(((13, 9), (14, 7), (13, 5), (14, 3), (12, 4), (15, 5))):
        px[x, y] = (*SMOKE, 150 - k * 15)
    return im


def limer():
    im = blank()
    px = im.load()
    body(px)
    for y in range(10, 15):
        for x in range(11, 15):
            px[x, y] = (*(LINE if x in (11, 14) or y == 14 else LIME), 255)
    for x in range(11, 15):
        px[x, 9] = (*INK, 255)
    for x, y in ((6, 14), (9, 13), (4, 15), (12, 15), (10, 15)):
        px[x, y] = (*LIME, 255)
    return im


def confessor():
    im = blank()
    px = im.load()
    body(px, tall=1, crown=HABIT_LIGHT)
    for y in range(8, 12):
        for x in range(2, 6):
            px[x, y] = (*(INK if x in (2, 5) or y in (8, 11) else (232, 220, 190)), 255)
    px[3, 9] = (*BLOOD, 255)
    return im


def prioress():
    im = blank()
    px = im.load()
    body(px, tall=1, crown=GOLD)
    for y in range(0, 16):
        px[13, y] = (*GOLD, 255)
    for y in range(0, 3):
        for x in range(12, 15):
            px[x, y] = (*BRASS, 255)
    px[13, 1] = (*FLAME_HOT, 255)
    for x in range(4, 12):
        px[x, 6] = (*GOLD, 255)
    return im


SPRITES = (lamplighter, penitent, censer, limer, confessor, prioress)


def add_synod(atlas):
    out = extend(atlas)
    for index, paint in enumerate(SPRITES):
        place_cell(out, SYNOD, index, paint())
    return out


if __name__ == "__main__":
    path = Path(__file__).resolve().parent.parent / "res" / "atlas.png"
    add_synod(Image.open(path).convert("RGBA")).save(path)
    print(f"wrote {path}")
