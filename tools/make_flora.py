"""Adds the flora band to res/atlas.png: recoloured Tiny Town plants for more varied vegetation.

Usage: uv run --with pillow tools/make_flora.py
make_assets.py calls add_flora_band() too. Run after make_winter.py. Layout must match src/Atlas.hpp (atlas::fl).
"""
from pathlib import Path

from PIL import Image

CELL, TOWN_X, FLORA_Y = 18, 12 * 18, 390
TRUNK, CROWN, BUSH, PINE, MUSHROOMS, TUFTS, FLOWERS = 16, 4, 5, 28, 29, 1, 2

PINK = ((150, 60, 100), (230, 120, 170), (255, 200, 225))
WHITE = ((165, 165, 190), (222, 222, 234), (255, 255, 248))
LILAC = ((88, 58, 138), (150, 110, 200), (206, 176, 240))
STRAW = ((128, 98, 40), (190, 160, 70), (236, 214, 120))
DEAD = ((78, 68, 58), (120, 104, 90), (162, 146, 126))
SPIRIT = ((28, 88, 108), (60, 170, 180), (150, 240, 230))
MOSS = ((20, 68, 44), (38, 108, 60), (70, 148, 80))
WILLOW = ((38, 58, 30), (84, 112, 48), (150, 168, 82))
REEDS = ((92, 82, 36), (150, 134, 62), (204, 186, 104))
MUD = ((52, 40, 32), (84, 64, 46), (118, 94, 66))
SPROUTS = 17


def lerp(a, b, t):
    return tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(3))


def green(r, g, b):
    return g > r + 8 and g > b + 8


def shade(palette, r, g, b):
    t = max(0.0, min(1.0, (0.3 * r + 0.59 * g + 0.11 * b - 70) / 110))
    return lerp(palette[0], palette[1], t / 0.5) if t < 0.5 else lerp(palette[1], palette[2], (t - 0.5) / 0.5)


def cell(atlas, index):
    x, y = TOWN_X + (index % 12) * CELL + 1, (index // 12) * CELL + 1
    return atlas.crop((x, y, x + 16, y + 16))


def remap(tile, fn):
    out = tile.copy()
    px = out.load()
    for y in range(16):
        for x in range(16):
            r, g, b, a = px[x, y]
            if a:
                px[x, y] = (*fn(x, y, r, g, b), a)
    return out


def palette(p):
    return lambda x, y, r, g, b: shade(p, r, g, b) if green(r, g, b) else (r, g, b)


def berries(x, y, r, g, b):
    if green(r, g, b) and (x * 7 + y * 13) % 19 == 0:
        return (212, 44, 56)
    return (r, g, b)


def blue_caps(x, y, r, g, b):
    if r > 150 and g < 110 and b < 110:
        return lerp((40, 60, 170), (110, 150, 255), (r - 150) / 105)
    return (r, g, b)


def pink_flowers(x, y, r, g, b):
    if r > 200 and b < 120 and not green(r, g, b):
        return lerp((214, 96, 150), (255, 190, 220), (g - 100) / 155)
    return (r, g, b)


PETAL_SPOTS = {(3, 4): (246, 170, 204), (4, 4): (255, 214, 232), (11, 2): (246, 170, 204), (7, 10): (240, 156, 196),
               (8, 10): (255, 214, 232), (13, 12): (246, 170, 204), (2, 13): (255, 214, 232)}


def petals(x, y, r, g, b):
    return PETAL_SPOTS.get((x, y), (r, g, b))


def purple_caps(x, y, r, g, b):
    if r > 150 and g < 110 and b < 110:
        return lerp((96, 40, 140), (200, 120, 240), (r - 150) / 105)
    return (r, g, b)


def mud(x, y, r, g, b):
    if r > g > b and r - b > 40:
        return shade(MUD, r, g, b)
    return (r, g, b)


def add_flora_band(atlas):
    out = Image.new("RGBA", (atlas.width, max(atlas.height, FLORA_Y + 5 * CELL)), (0, 0, 0, 0))
    out.paste(atlas.crop((0, 0, atlas.width, min(atlas.height, FLORA_Y))), (0, 0))
    tiles = [
        remap(cell(atlas, TRUNK), palette(PINK)),
        remap(cell(atlas, CROWN), palette(PINK)),
        remap(cell(atlas, TRUNK), palette(WHITE)),
        remap(cell(atlas, CROWN), palette(WHITE)),
        remap(cell(atlas, BUSH), palette(LILAC)),
        remap(cell(atlas, BUSH), berries),
        remap(cell(atlas, BUSH), palette(STRAW)),
        remap(cell(atlas, PINE), palette(DEAD)),
        remap(cell(atlas, PINE), palette(SPIRIT)),
        remap(cell(atlas, PINE), palette(MOSS)),
        remap(cell(atlas, MUSHROOMS), blue_caps),
        remap(cell(atlas, FLOWERS), pink_flowers),
        remap(cell(atlas, TUFTS), petals),
        remap(cell(atlas, TRUNK), palette(WILLOW)),
        remap(cell(atlas, CROWN), palette(WILLOW)),
        remap(cell(atlas, SPROUTS), palette(REEDS)),
        remap(cell(atlas, MUSHROOMS), purple_caps),
    ]
    for i, t in enumerate(tiles):
        out.paste(t, (TOWN_X + (i % 12) * CELL + 1, FLORA_Y + (i // 12) * CELL + 1))
    for row in range(3):
        for col in range(3):
            out.paste(remap(cell(atlas, 12 + row * 12 + col), mud), (TOWN_X + col * CELL + 1, FLORA_Y + (2 + row) * CELL + 1))
    return out


if __name__ == "__main__":
    path = Path(__file__).resolve().parent.parent / "res" / "atlas.png"
    add_flora_band(Image.open(path).convert("RGBA")).save(path)
    print(f"wrote {path}")
