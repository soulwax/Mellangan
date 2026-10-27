"""Builds res/atlas.png from Kenney's Tiny Dungeon and Tiny Town sheets plus procedurally drawn FX sprites.

Usage: uv run --with pillow tools/make_assets.py "<path to Kenney Game Assets All-in-1>"
The layout here must stay in sync with src/Atlas.hpp.
"""
import math
import sys
from pathlib import Path

from PIL import Image

TILE = 16
CELL = 18  # 1px transparent gutter on each side of every tile, prevents bleeding
SHEET_COLS, SHEET_ROWS = 12, 11
FX_Y = SHEET_ROWS * CELL + 2  # 200
SHEETS = ["Tiny Dungeon", "Tiny Town"]  # side by side, each SHEET_COLS * CELL wide
ATLAS_W, ATLAS_H = len(SHEETS) * SHEET_COLS * CELL, 264


def clamp01(v):
    return max(0.0, min(1.0, v))


def smoothstep(e0, e1, x):
    t = clamp01((x - e0) / (e1 - e0))
    return t * t * (3 - 2 * t)


def field(w, h, fn):
    im = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    px = im.load()
    if px is None:
        raise RuntimeError("Unable to access image pixels")
    for y in range(h):
        for x in range(w):
            r, g, b, a = fn(x + 0.5, y + 0.5)
            px[x, y] = (int(r * 255), int(g * 255), int(b * 255), int(clamp01(a) * 255))
    return im


def glow(size):
    c = size / 2

    def fn(x, y):
        d = math.hypot(x - c, y - c) / c
        a = (1 - smoothstep(0.0, 1.0, d)) ** 1.6
        return 1, 1, 1, a

    return field(size, size, fn)


def ring(size):
    c = size / 2

    def fn(x, y):
        d = math.hypot(x - c, y - c) / c
        band = math.exp(-((d - 0.86) / 0.07) ** 2)
        inner = 0.22 * smoothstep(0.95, 0.2, d) * (0.6 + 0.4 * d)
        return 1, 1, 1, clamp01(band + inner) * (d < 1.0)

    return field(size, size, fn)


def slash(w, h):
    # ")"-shaped crescent bulging to the right: inside one circle, outside an offset one.
    # The game mirrors it with a negative x scale for left-facing swings.
    cy = h / 2
    c1x, r1 = w - 2 - h * 0.95, h * 0.95
    c2x, r2 = c1x - 7, h * 0.98

    def fn(x, y):
        d1 = math.hypot(x - c1x, y - cy)
        d2 = math.hypot(x - c2x, y - cy)
        inside = smoothstep(r1 + 0.8, r1 - 0.8, d1) * smoothstep(r2 - 0.8, r2 + 0.8, d2)
        tip = clamp01(1 - abs(y - cy) / (h / 2))
        edge = smoothstep(r1 - 5, r1, d1)
        v = 0.7 + 0.3 * edge
        return v, v, v, inside * (tip ** 0.6)

    return field(w, h, fn)


def soft_dot(size, hardness=2.5):
    c = size / 2

    def fn(x, y):
        d = math.hypot(x - c, y - c) / c
        return 1, 1, 1, clamp01(1 - d) ** (1 / hardness)

    return field(size, size, fn)


def shadow(w, h):
    def fn(x, y):
        dx = (x - w / 2) / (w / 2)
        dy = (y - h / 2) / (h / 2)
        d = math.hypot(dx, dy)
        return 0, 0, 0, 0.45 * smoothstep(1.0, 0.55, d)

    return field(w, h, fn)


def bolt(size):
    c = size / 2

    def fn(x, y):
        d = math.hypot(x - c, y - c) / c
        core = smoothstep(0.45, 0.1, d)
        halo = (1 - smoothstep(0.0, 1.0, d)) ** 2
        v = 0.55 + 0.45 * core
        return v, v, v, clamp01(core + halo * 0.8)

    return field(size, size, fn)


GEM_SHAPE = [
    "..ooo..",
    ".ohhlo.",
    "ohhllmo",
    "olllmdo",
    ".olmdo.",
    "..odo..",
    "...o...",
]

GEM_PALETTES = {
    "blue": {"o": (20, 30, 70), "h": (235, 250, 255), "l": (120, 200, 255), "m": (50, 120, 230), "d": (30, 60, 150)},
    "green": {"o": (15, 50, 30), "h": (240, 255, 235), "l": (130, 240, 140), "m": (40, 180, 80), "d": (20, 100, 50)},
    "red": {"o": (60, 10, 30), "h": (255, 240, 240), "l": (255, 120, 140), "m": (220, 40, 80), "d": (130, 20, 50)},
}


def gem(palette):
    im = Image.new("RGBA", (10, 10), (0, 0, 0, 0))
    px = im.load()
    if px is None:
        raise RuntimeError("Unable to access image pixels")
    for y, row in enumerate(GEM_SHAPE):
        for x, ch in enumerate(row):
            if ch != ".":
                px[x + 1, y + 1] = (*palette[ch], 255)
    return im


DIGITS = [
    "111101101101111", "010110010010111", "111001111100111", "111001111001111", "101101111001001",
    "111100111001111", "111100111101111", "111001010010010", "111101111101111", "111101111001111",
]


def digits():
    # 3x5 glyphs, white with a baked 1px dark outline, packed in 5x7 cells.
    im = Image.new("RGBA", (50, 7), (0, 0, 0, 0))
    px = im.load()
    if px is None:
        raise RuntimeError("Unable to access image pixels")
    for i, bits in enumerate(DIGITS):
        on = {(x, y) for y in range(5) for x in range(3) if bits[y * 3 + x] == "1"}
        for y in range(-1, 6):
            for x in range(-1, 4):
                p = (i * 5 + x + 1, y + 1)
                if (x, y) in on:
                    px[p] = (255, 255, 255, 255)
                elif any((x + dx, y + dy) in on for dx in (-1, 0, 1) for dy in (-1, 0, 1)):
                    px[p] = (20, 12, 28, 255)
    return im


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    atlas = Image.new("RGBA", (ATLAS_W, ATLAS_H), (0, 0, 0, 0))
    for i, name in enumerate(SHEETS):
        sheet = Image.open(Path(sys.argv[1]) / "2D assets" / name / "Tilemap" / "tilemap_packed.png").convert("RGBA")
        ox = i * SHEET_COLS * CELL
        for r in range(SHEET_ROWS):
            for c in range(SHEET_COLS):
                tile = sheet.crop((c * TILE, r * TILE, c * TILE + TILE, r * TILE + TILE))
                atlas.paste(tile, (ox + c * CELL + 1, r * CELL + 1))

    fx = [
        (glow(64), (0, FX_Y)),
        (ring(64), (64, FX_Y)),
        (slash(24, 48), (128, FX_Y)),
        (shadow(16, 8), (176, FX_Y)),
        (gem(GEM_PALETTES["blue"]), (196, FX_Y)),
        (gem(GEM_PALETTES["green"]), (208, FX_Y)),
        (gem(GEM_PALETTES["red"]), (220, FX_Y)),
        (bolt(10), (232, FX_Y)),
        (soft_dot(8), (244, FX_Y)),
        (Image.new("RGBA", (4, 4), (255, 255, 255, 255)), (196, FX_Y + 12)),
        (digits(), (176, FX_Y + 20)),
    ]
    for im, pos in fx:
        atlas.paste(im, pos)

    out = Path(__file__).resolve().parent.parent / "res" / "atlas.png"
    atlas.save(out)
    print(f"wrote {out}")


if __name__ == "__main__":
    main()
