"""Builds res/atlas.png from Kenney's Tiny Dungeon and Tiny Town sheets plus procedurally drawn FX sprites.

Usage: uv run --with pillow tools/make_assets.py "<path to Kenney Game Assets All-in-1>"
Verify: uv run --with pillow==13.0.0 tools/make_assets.py --verify "<path>"
The layout here must stay in sync with src/Atlas.hpp.
"""
import io
import hashlib
import math
import sys
from pathlib import Path

from PIL import Image, __version__ as PILLOW_VERSION

from atlas_layout import validate, validate_cpp_layout
from make_desert import add_desert
from make_expansion import add_expansion
from make_flora import add_flora_band
from make_frontier import add_frontier
from make_grounds import add_grounds
from make_rims import add_rims
from make_sprites import add_sprites
from make_winter import add_winter_band

TILE = 16
CELL = 18  # 1px transparent gutter on each side of every tile, prevents bleeding
SHEET_COLS, SHEET_ROWS = 12, 11
FX_Y = SHEET_ROWS * CELL + 2  # 200
SHEETS = ["Tiny Dungeon", "Tiny Town"]  # side by side, each SHEET_COLS * CELL wide
ATLAS_W, ATLAS_H = len(SHEETS) * SHEET_COLS * CELL, 264
PIPELINE = (
    ("winter", (432, 390), add_winter_band),
    ("flora", (432, 480), add_flora_band),
    ("desert", (432, 480), add_desert),
    ("sprite kit", (432, 552), add_sprites),
    ("grounds", (432, 624), add_grounds),
    ("frontier", (432, 624), add_frontier),
    ("expansion", (432, 1020), add_expansion),
    ("dirt rims", (432, 1020), add_rims),
)


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


BAYER = [[0, 8, 2, 10], [12, 4, 14, 6], [3, 11, 1, 9], [15, 7, 13, 5]]
GLOW_BANDS = [0.0, 0.16, 0.34, 0.58, 0.9]


def dither(v, x, y, levels):
    t = (BAYER[int(y) % 4][int(x) % 4] + 0.5) / 16
    for lo, hi in zip(levels, levels[1:]):
        if v <= hi:
            return hi if (v - lo) / (hi - lo) > t else lo
    return levels[-1]


def glow(size, banded=True):
    c = size / 2

    def fn(x, y):
        d = math.hypot(x - c, y - c) / c
        a = (1 - smoothstep(0.0, 1.0, d)) ** 1.6
        return 1, 1, 1, dither(a, x, y, GLOW_BANDS) if banded else a

    return field(size, size, fn)


def ring(size):
    c = size / 2

    def fn(x, y):
        d = math.hypot(x - c, y - c)
        band = abs(d - c * 0.86)
        a = 1.0 if band < 0.8 else 0.45 if d < c * 0.86 and band < 1.8 else 0.0
        return 1, 1, 1, a

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
        if d1 > r1 or d2 < r2:
            return 0, 0, 0, 0
        tip = clamp01(1 - abs(y - cy) / (h / 2))
        if tip < 0.12:
            return 0, 0, 0, 0
        edge = r1 - d1 < 1.5
        v = 1.0 if edge else 0.72
        return v, v, v, 1.0 if edge or tip > 0.45 else 0.55

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
        return 0, 0, 0, 0.42 if d < 0.62 else 0.24 if d < 0.95 else 0.0

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


def build(source):
    atlas = Image.new("RGBA", (ATLAS_W, ATLAS_H), (0, 0, 0, 0))
    for i, name in enumerate(SHEETS):
        sheet = Image.open(source / "2D assets" / name / "Tilemap" /
                           "tilemap_packed.png").convert("RGBA")
        if sheet.width < SHEET_COLS * TILE or sheet.height < SHEET_ROWS * TILE:
            raise ValueError(f"{name} source is {sheet.width}x{sheet.height}, expected at least 192x176")
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
        (glow(64, banded=False), (256, FX_Y)),
        (Image.new("RGBA", (4, 4), (255, 255, 255, 255)), (196, FX_Y + 12)),
        (digits(), (176, FX_Y + 20)),
    ]
    for im, pos in fx:
        atlas.paste(im, pos)

    for name, size, step in PIPELINE:
        atlas = step(atlas)
        if atlas.size != size:
            raise RuntimeError(f"{name} produced {atlas.width}x{atlas.height}, expected {size[0]}x{size[1]}")
    errors = validate(atlas)
    if errors:
        raise RuntimeError("invalid atlas layout:\n" + "\n".join(errors))
    return atlas


def encode(atlas):
    out = io.BytesIO()
    atlas.save(out, format="PNG", compress_level=9)
    return out.getvalue()


def main():
    verify = "--verify" in sys.argv[1:]
    args = [arg for arg in sys.argv[1:] if arg != "--verify"]
    if len(args) != 1:
        sys.exit(__doc__)
    source = Path(args[0])
    root = Path(__file__).resolve().parent.parent
    if verify and PILLOW_VERSION != "13.0.0":
        raise RuntimeError(f"verification requires Pillow 13.0.0, found {PILLOW_VERSION}")
    inputs = []
    for name in SHEETS:
        path = source / "2D assets" / name / "Tilemap" / "tilemap_packed.png"
        inputs.append((name, hashlib.sha256(path.read_bytes()).hexdigest()))
    cpp_errors = validate_cpp_layout(root / "src" / "Atlas.hpp")
    if cpp_errors:
        raise RuntimeError("atlas lookup mismatch:\n" + "\n".join(cpp_errors))
    first_image = build(source)
    first = encode(first_image)
    if verify:
        second_image = build(source)
        if first_image.mode != second_image.mode or first_image.size != second_image.size or first_image.tobytes() != second_image.tobytes():
            raise RuntimeError("asset pipeline produced different RGBA pixels on its second run")
        first = encode(first_image)
        second = encode(second_image)
        if first != second:
            raise RuntimeError("asset pipeline produced different PNG bytes on its second run")
    out = Path(__file__).resolve().parent.parent / "res" / "atlas.png"
    out.write_bytes(first)
    message = f"wrote {out}"
    if verify:
        rgba_hash = hashlib.sha256(first_image.tobytes()).hexdigest()
        png_hash = hashlib.sha256(first).hexdigest()
        source_hashes = ", ".join(f"{name} {digest}" for name, digest in inputs)
        message += (f"; reproducible with Pillow {PILLOW_VERSION}; inputs {source_hashes}; "
                    f"rgba {rgba_hash}; png {png_hash}")
    print(message)


if __name__ == "__main__":
    main()
