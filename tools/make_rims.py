"""Adds dirt-to-ground edge sets to res/atlas.png, one per carpet and expansion ground.

Tiny Town's dirt edges fade into grass, which frames every dirt yard, mine and ruin with a green ring in biomes
that have no grass. Each set here is that 3x3 edge set with its grass pixels replaced by the biome's own ground,
so dirt meets ash, moss, clay or salt instead. The dirt itself leans 30% toward the ground's average colour.

Layout: band at y=786, 24 columns. Set s (0-5 carpets straw, ash, crystal moss, mycelium, red clay, needles;
6-15 expansion grounds 0-9) owns cells s*9 .. s*9+8, the edge set row by row. Mirrors atlas::dirtEdge.
Lake edges follow from cell 144: the frontier water edges with grass swapped for the same 16 grounds, then a 17th
set for frozen lakes, ice ringed by snow. Set s owns cells 144 + s*9 .. +8. Mirrors atlas::waterEdge.

Usage: python tools/make_rims.py [--preview out.png]   (make_assets.py and make_expansion.py call add_rims too)
"""
import sys
from pathlib import Path

from PIL import Image

from make_grounds import CELL, green, slot, town

RIM_Y = 786
WATER_CELL = 144
FROST_SET = 16
CARPETS = (0, 1, 2, 3, 5, 6)
EXPANSION_Y = 696
SETS = len(CARPETS) + 10


def expansion_slot(index, cell):
    return ((index % 2) * 12 + cell) * CELL + 1, EXPANSION_Y + (index // 2) * CELL + 1


def rim_slot(set_index, cell):
    i = set_index * 9 + cell
    return (i % 24) * CELL + 1, RIM_Y + (i // 24) * CELL + 1


def crop(atlas, corner):
    x, y = corner
    return atlas.crop((x, y, x + 16, y + 16))


def average(tile):
    data = tile.load()
    px = [data[x, y] for y in range(16) for x in range(16) if data[x, y][3]]
    return tuple(sum(p[i] for p in px) // len(px) for i in range(3))


def edge(tile, ground):
    out = tile.copy()
    px, gp = out.load(), ground.load()
    tone = average(ground)
    for y in range(16):
        for x in range(16):
            r, g, b, a = px[x, y]
            if not a:
                continue
            if green(r, g, b):
                px[x, y] = gp[x, y]
            else:
                px[x, y] = (int(r * 0.7 + tone[0] * 0.3), int(g * 0.7 + tone[1] * 0.3),
                            int(b * 0.7 + tone[2] * 0.3), a)
    return out


def water_slot(index):
    return 216 + (6 + index % 12) * CELL + 1, 426 + (index // 12) * CELL + 1


def ice(tile):
    out = tile.copy()
    px = out.load()
    for y in range(16):
        for x in range(16):
            r, g, b, a = px[x, y]
            if a and b > r + 30:
                px[x, y] = (int(r * 0.35 + 196 * 0.65), int(g * 0.35 + 222 * 0.65), int(b * 0.35 + 242 * 0.65), a)
    return out


def lake(tile, ground):
    out = tile.copy()
    px, gp = out.load(), ground.load()
    for y in range(16):
        for x in range(16):
            r, g, b, a = px[x, y]
            if a and green(r, g, b):
                px[x, y] = gp[x, y]
    return out


def add_rims(atlas):
    rows = (WATER_CELL + (SETS + 1) * 9 + 23) // 24
    out = Image.new("RGBA", (atlas.width, max(atlas.height, RIM_Y + rows * CELL)), (0, 0, 0, 0))
    out.paste(atlas.crop((0, 0, atlas.width, min(atlas.height, RIM_Y))), (0, 0))
    grounds = [crop(atlas, slot(c, 0)) for c in CARPETS] + [crop(atlas, expansion_slot(i, 0)) for i in range(10)]
    for s, ground in enumerate(grounds):
        for row in range(3):
            for col in range(3):
                out.paste(edge(town(atlas, 12 + row * 12 + col), ground), rim_slot(s, row * 3 + col))
    snow = crop(atlas, (216 + 1, 264 + 1))
    for s, ground in enumerate(grounds + [snow]):
        for row in range(3):
            for col in range(3):
                tile = lake(crop(atlas, water_slot(row * 12 + col)), ground)
                out.paste(ice(tile) if s == FROST_SET else tile,
                          rim_slot(0, WATER_CELL + s * 9 + row * 3 + col))
    return out


def preview(atlas, path, zoom=4):
    img = Image.new("RGBA", (18 * 18, (SETS + 1) * 18), (40, 40, 60, 255))
    for s in range(SETS + 1):
        for c in range(9):
            if s < SETS:
                img.paste(crop(atlas, rim_slot(s, c)), (c * 18 + 1, s * 18 + 1))
            img.paste(crop(atlas, rim_slot(0, WATER_CELL + s * 9 + c)), ((9 + c) * 18 + 1, s * 18 + 1))
    img.resize((img.width * zoom, img.height * zoom), Image.Resampling.NEAREST).save(path)


if __name__ == "__main__":
    root = Path(__file__).resolve().parent.parent / "res" / "atlas.png"
    atlas = add_rims(Image.open(root).convert("RGBA"))
    if len(sys.argv) > 2 and sys.argv[1] == "--preview":
        preview(atlas, sys.argv[2])
        print(f"wrote {sys.argv[2]}")
    else:
        atlas.save(root)
        print(f"wrote {root}")
