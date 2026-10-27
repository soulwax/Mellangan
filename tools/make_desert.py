"""Adds desert tiles to the flora band of res/atlas.png: sand, sand and water edge sets, cactus and palm.

Usage: uv run --with pillow tools/make_desert.py
make_assets.py calls add_desert() too. Run after make_flora.py. Layout must match src/Atlas.hpp (atlas::fl).
"""
from pathlib import Path

from PIL import Image

CELL, TOWN_X, FLORA_Y = 18, 12 * 18, 390
SAND = ((192, 152, 98), (224, 192, 130), (242, 220, 164))
WATER = ((36, 86, 148), (58, 124, 188), (112, 176, 228))
OUTLINE = (39, 25, 37)
CACTUS = ((52, 110, 60), (84, 150, 74), (130, 190, 96))
TRUNK = ((112, 76, 44), (150, 106, 62), (184, 140, 86))
FROND = ((40, 104, 48), (70, 150, 64), (120, 196, 90))


def lerp(a, b, t):
    t = max(0.0, min(1.0, t))
    return tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(3))


def shade(palette, r, g, b):
    t = max(0.0, min(1.0, (0.3 * r + 0.59 * g + 0.11 * b - 70) / 110))
    return lerp(palette[0], palette[1], t / 0.5) if t < 0.5 else lerp(palette[1], palette[2], (t - 0.5) / 0.5)


def cell(atlas, index):
    x, y = TOWN_X + (index % 12) * CELL + 1, (index // 12) * CELL + 1
    return atlas.crop((x, y, x + 16, y + 16))


def remap(tile, fn):
    out = tile.copy()
    px = out.load()
    for y in range(out.height):
        for x in range(out.width):
            r, g, b, a = px[x, y]
            if a:
                px[x, y] = (*fn(x, y, r, g, b), a)
    return out


def is_green(r, g, b):
    return g > r + 8 and g > b + 8


def is_dirt(r, g, b):
    return r > g > b and r - b > 40


def sand_ground(x, y, r, g, b):
    return shade(SAND, r, g, b) if is_green(r, g, b) else (r, g, b)


def sand_rim(x, y, r, g, b):
    return shade(SAND, r + 20, g + 20, b + 20) if is_dirt(r, g, b) else (r, g, b)


def water_rim(x, y, r, g, b):
    if not is_dirt(r, g, b):
        return (r, g, b)
    if (x * 5 + y * 3) % 23 == 0:
        return WATER[2]
    return shade(WATER, r - 10, g - 10, b - 10)


def pebbles(tile):
    out = remap(tile, sand_ground)
    px = out.load()
    for x, y in ((4, 5), (11, 3), (9, 11), (3, 12), (13, 9)):
        px[x, y] = (*SAND[0], 255)
        px[x + 1, y] = (*lerp(SAND[0], OUTLINE, 0.3), 255)
    return out


def sprite(w, h, paint):
    mask = {}
    paint(mask)
    img = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    px = img.load()
    for (x, y), c in mask.items():
        if 0 <= x < w and 0 <= y < h:
            px[x, y] = (*c, 255)
    for (x, y) in list(mask):
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            nx, ny = x + dx, y + dy
            if 0 <= nx < w and 0 <= ny < h and (nx, ny) not in mask:
                px[nx, ny] = (*OUTLINE, 255)
    return img


def fill(mask, x0, y0, x1, y1, palette):
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            mask[(x, y)] = palette[2] if x == x0 else palette[0] if x == x1 else palette[1]


def cactus():
    def paint(m):
        fill(m, 7, 2, 9, 14, CACTUS)
        fill(m, 3, 6, 4, 9, CACTUS)
        fill(m, 5, 9, 6, 10, CACTUS)
        fill(m, 12, 4, 13, 8, CACTUS)
        fill(m, 10, 8, 11, 9, CACTUS)
        for y in (4, 7, 10, 13):
            m[(8, y)] = CACTUS[2]
    return sprite(16, 16, paint)


def palm():
    def paint(m):
        for y in range(12, 31):
            x = 6 if y > 24 else 7 if y > 17 else 8
            for k in range(3):
                m[(x + k, y)] = TRUNK[2] if k == 0 else TRUNK[0] if k == 2 else TRUNK[1]
            if y % 3 == 0:
                m[(x + 1, y)] = TRUNK[0]
        fronds = [(-7, 0), (-6, -4), (-3, -7), (1, -8), (5, -6), (7, -2), (7, 3), (-7, 4), (-4, 6), (4, 6)]
        for dx, dy in fronds:
            steps = max(abs(dx), abs(dy)) * 2
            for i in range(steps + 1):
                t = i / steps
                x, y = 9 + dx * t, 9 + dy * t + t * t * 2.5
                c = FROND[2] if t < 0.35 else FROND[1] if t < 0.7 else FROND[0]
                m[(int(round(x)), int(round(y)))] = c
                if t < 0.85:
                    m[(int(round(x)), int(round(y)) + 1)] = FROND[0]
        for x in range(7, 12):
            for y in range(7, 11):
                m[(x, y)] = FROND[1] if (x + y) % 2 else FROND[2]
        for x, y in ((8, 11), (10, 11), (9, 12)):
            m[(x, y)] = (126, 82, 44)
    return sprite(16, 32, paint)


def barrel():
    def paint(m):
        for y in range(6, 15):
            for x in range(2, 15):
                if ((x - 8) / 5.5) ** 2 + ((y - 10.5) / 4.5) ** 2 <= 1:
                    m[(x, y)] = CACTUS[0] if x in (5, 8, 11) else CACTUS[2] if x < 6 else CACTUS[1]
        for x, y in ((7, 5), (8, 4), (9, 5), (8, 5)):
            m[(x, y)] = (236, 120, 170) if (x, y) != (8, 5) else (252, 210, 90)
    return sprite(16, 16, paint)


def put(out, tile, col, row):
    out.paste(tile, (TOWN_X + col * CELL + 1, FLORA_Y + row * CELL + 1))


def add_desert(atlas):
    out = atlas.copy()
    put(out, remap(cell(atlas, 0), sand_ground), 5, 1)
    put(out, remap(cell(atlas, 1), sand_ground), 6, 1)
    put(out, cactus(), 7, 1)
    tall = palm()
    put(out, tall.crop((0, 16, 16, 32)), 8, 1)
    put(out, tall.crop((0, 0, 16, 16)), 9, 1)
    put(out, pebbles(cell(atlas, 0)), 10, 1)
    put(out, barrel(), 11, 1)
    for row in range(3):
        for col in range(3):
            rim = cell(atlas, 12 + row * 12 + col)
            put(out, remap(rim, sand_rim), 3 + col, 2 + row)
            put(out, remap(rim, water_rim), 6 + col, 2 + row)
    return out


if __name__ == "__main__":
    path = Path(__file__).resolve().parent.parent / "res" / "atlas.png"
    add_desert(Image.open(path).convert("RGBA")).save(path)
    print(f"wrote {path}")
