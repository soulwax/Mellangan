"""Adds carpet ground sets to res/atlas.png: straw, ash, crystal moss, mycelium, and lava pools.

Usage: uv run --with pillow tools/make_grounds.py [--preview out.png]
make_assets.py calls add_grounds() too, after make_sprites. Layout must match src/Atlas.hpp (atlas::carpet).
Each set is 12 cells in one row half: 3 fills (plain, tufted, flowered) then the 3x3 edge set, row by row.
"""
import sys
from pathlib import Path

from PIL import Image

CELL, TOWN_X, GROUND_Y = 18, 12 * 18, 552

STRAW = ((164, 140, 64), (196, 176, 92), (222, 206, 128))
ASH = ((70, 66, 70), (98, 94, 98), (128, 122, 124))
MOSS = ((126, 116, 170), (160, 150, 206), (196, 188, 236))
MYCELIUM = ((110, 84, 110), (146, 114, 140), (180, 150, 170))
LAVA = ((150, 40, 20), (220, 90, 30), (255, 186, 70))

SETS = [
    ("straw", STRAW, None),
    ("ash", ASH, (236, 110, 40)),
    ("crystal moss", MOSS, (200, 250, 255)),
    ("mycelium", MYCELIUM, (240, 190, 220)),
]


def lerp(a, b, t):
    t = max(0.0, min(1.0, t))
    return tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(3))


def shade(pal, r, g, b, bias=0):
    t = max(0.0, min(1.0, (0.3 * r + 0.59 * g + 0.11 * b - 70 + bias) / 110))
    return lerp(pal[0], pal[1], t / 0.5) if t < 0.5 else lerp(pal[1], pal[2], (t - 0.5) / 0.5)


def green(r, g, b):
    return g > r + 8 and g > b + 8


def dirt(r, g, b):
    return r > g > b and r - b > 40


def town(atlas, index):
    x, y = TOWN_X + (index % 12) * CELL + 1, (index // 12) * CELL + 1
    return atlas.crop((x, y, x + 16, y + 16))


def remap(tile, fn):
    out = tile.copy()
    px = out.load()
    for y in range(16):
        for x in range(16):
            r, g, b, a = px[x, y]
            if a:
                px[x, y] = (*fn(r, g, b), a)
    return out


def fill(pal, accent):
    def fn(r, g, b):
        if green(r, g, b):
            return shade(pal, r, g, b)
        if r > 200 and not green(r, g, b):
            return accent if accent and r + g + b > 690 else pal[2] if accent else (r, g, b)
        return (r, g, b)
    return fn


def rim(pal, fringe=None):
    def fn(r, g, b):
        if dirt(r, g, b):
            return shade(pal, r, g, b, -25 if pal is LAVA else 20)
        if fringe and green(r, g, b):
            return shade(fringe, r, g, b)
        return (r, g, b)
    return fn


def slot(set_index, cell):
    return ((set_index % 2) * 12 + cell) * CELL + 1, GROUND_Y + (set_index // 2) * CELL + 1


def add_grounds(atlas):
    out = Image.new("RGBA", (atlas.width, max(atlas.height, GROUND_Y + 3 * CELL)), (0, 0, 0, 0))
    out.paste(atlas.crop((0, 0, atlas.width, min(atlas.height, GROUND_Y))), (0, 0))
    sets = SETS + [("lava", LAVA, None)]
    for i, (_, pal, accent) in enumerate(sets):
        lava = i == len(SETS)
        if not lava:
            for k, src in enumerate((0, 1, 2)):
                out.paste(remap(town(atlas, src), fill(pal, accent)), slot(i, k))
        for row in range(3):
            for col in range(3):
                edge = town(atlas, 12 + row * 12 + col)
                out.paste(remap(edge, rim(pal, ASH if lava else None)), slot(i, 3 + row * 3 + col))
    return out


def preview(atlas, path, zoom=4):
    names = [s[0] for s in SETS] + ["lava"]
    img = Image.new("RGBA", (12 * 18, len(names) * 18), (40, 40, 60, 255))
    for i in range(len(names)):
        for c in range(12):
            x, y = slot(i, c)
            img.paste(atlas.crop((x, y, x + 16, y + 16)), (c * 18 + 1, i * 18 + 1))
    img.resize((img.width * zoom, img.height * zoom), Image.NEAREST).save(path)


if __name__ == "__main__":
    root = Path(__file__).resolve().parent.parent / "res" / "atlas.png"
    atlas = add_grounds(Image.open(root).convert("RGBA"))
    if len(sys.argv) > 2 and sys.argv[1] == "--preview":
        preview(atlas, sys.argv[2])
        print(f"wrote {sys.argv[2]}")
    else:
        atlas.save(root)
        print(f"wrote {root}")
