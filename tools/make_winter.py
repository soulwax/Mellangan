"""Adds the winter band to res/atlas.png: Tiny Town rows 0-3 (ground, dirt rims, trees) with the greens frosted.

Usage: uv run --with pillow tools/make_winter.py
make_assets.py calls add_winter_band() too, so a full rebuild keeps the band. Layout must match src/Atlas.hpp.
"""
from pathlib import Path

from PIL import Image

CELL, TOWN_X, BASE_H, ROWS = 18, 12 * 18, 264, 4
WINTER_Y = BASE_H
RIM_Y = WINTER_Y + ROWS * CELL
SNOW, ICE, SHADE, FROZEN = (238, 243, 250), (190, 206, 228), (104, 120, 158), (128, 122, 132)


def lerp(a, b, t):
    return tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(3))


def frost(rgba):
    r, g, b, a = rgba
    if a == 0:
        return rgba
    if r > g > b and r - b > 40:
        return (*lerp((r, g, b), FROZEN, 0.45), a)
    if g <= r + 8 or g <= b + 8:
        return rgba
    t = max(0.0, min(1.0, (0.3 * r + 0.59 * g + 0.11 * b - 70) / 110))
    c = lerp(SHADE, ICE, t / 0.6) if t < 0.6 else lerp(ICE, SNOW, (t - 0.6) / 0.4)
    return (*c, a)


def snow_over_dirt(rgba):
    r, g, b, a = rgba
    if a == 0 or not (r > g > b and r - b > 40):
        return rgba
    t = max(0.0, min(1.0, (0.3 * r + 0.59 * g + 0.11 * b - 90) / 90))
    return (*(lerp(ICE, SNOW, t) if t > 0.5 else lerp(SHADE, ICE, t * 2)), a)


def add_winter_band(atlas):
    base = atlas.crop((0, 0, atlas.width, BASE_H))
    out = Image.new("RGBA", (atlas.width, RIM_Y + 3 * CELL), (0, 0, 0, 0))
    out.paste(base, (0, 0))
    band = base.crop((TOWN_X, 0, TOWN_X + 12 * CELL, ROWS * CELL))
    px = band.load()
    for y in range(band.height):
        for x in range(band.width):
            px[x, y] = frost(px[x, y])
    out.paste(band, (TOWN_X, WINTER_Y))
    rim = base.crop((TOWN_X, CELL, TOWN_X + 3 * CELL, 4 * CELL))
    px = rim.load()
    for y in range(rim.height):
        for x in range(rim.width):
            px[x, y] = snow_over_dirt(px[x, y])
    out.paste(rim, (TOWN_X, RIM_Y))
    return out


if __name__ == "__main__":
    path = Path(__file__).resolve().parent.parent / "res" / "atlas.png"
    add_winter_band(Image.open(path).convert("RGBA")).save(path)
    print(f"wrote {path}")
