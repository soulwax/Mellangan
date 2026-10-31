"""Paints procedural cliff sets into res/atlas.png, one 3x3 edge set per material.

Each set is drawn on a 48 x 48 canvas as one plateau seen from above at a slight tilt, then cut into nine 16 x 16
tiles. The plateau top is left transparent, so the walls overlay any ground:
    north edge  a 1 px outline over a lit lip
    south edge  a tall face (14 px) with strata, cracks and speckles, darkening downward
    west side   a narrow face catching the light from the top left
    east side   a narrow face in shade
    corners     rounded, with the south face wrapping into the side faces
Textures repeat every 16 px along each edge, so middle tiles join seamlessly when the set is laid out.

Material 0 is the owner's DuskMoon earth (make_rims.py, rims band cells 297-305). This script paints materials 1-5
(stone, sandstone, ice, basalt, mossy stone) into the cliffs band at y=1326: material m owns cells (m-1)*9 .. +8, row by
row. Mirrors atlas::cliff(material, offset).
After them, cells 45 + m*3 .. +2 hold each material's stairs (left, middle, right), m 0-5 including DuskMoon earth:
steps descending south, each a lit tread over a shaded riser, flanked by side walls at the ends. Mirrors
atlas::stairs(material, part).

Usage: python tools/make_cliffs.py [--preview out.png]   (make_assets.py calls add_cliffs too)
"""
import sys
from pathlib import Path

from PIL import Image

from make_grounds import CELL

RIM_Y = 786
DUSKMOON_CELL = 297
BAND_Y = 1326
FACE = 14
SIDE = 4

MATERIALS = [
    ("stone", [(196, 196, 204), (156, 156, 168), (122, 122, 136), (92, 92, 106), (52, 50, 62)], None),
    ("sandstone", [(240, 212, 156), (214, 176, 118), (180, 138, 92), (146, 106, 72), (92, 66, 52)], None),
    ("ice", [(238, 248, 255), (198, 226, 248), (156, 196, 230), (116, 156, 204), (64, 84, 126)], None),
    ("basalt", [(112, 106, 114), (82, 76, 86), (62, 58, 68), (44, 40, 50), (22, 20, 26)], None),
    ("mossy stone", [(176, 180, 168), (138, 142, 132), (106, 110, 102), (80, 84, 78), (46, 48, 46)],
     [(126, 186, 92), (88, 146, 66)]),
]


def noise(x, y, seed):
    n = (x * 374761393 + y * 668265263 + seed * 2246822519) & 0xFFFFFFFF
    n = ((n ^ (n >> 13)) * 1274126177) & 0xFFFFFFFF
    return ((n ^ (n >> 16)) & 0xFFFF) / 0xFFFF


def inside(x, y, r=4):
    for cx, cy in ((r, r), (47 - r, r), (r, 47 - r), (47 - r, 47 - r)):
        if (x < r and cx == r or x > 47 - r and cx == 47 - r) and (y < r and cy == r or y > 47 - r and cy == 47 - r):
            return (x - cx) ** 2 + (y - cy) ** 2 <= r * r + r
    return 0 <= x < 48 and 0 <= y < 48


def face(x, y, pal, seed):
    """South face body, rows 0..FACE-1 counted from the lip, periodic in x every 16 px."""
    px, depth = x % 16, y / (FACE - 1)
    light, mid, dark, deep, _ = pal
    band = (y + (1 if noise(px // 4, 0, seed) > 0.5 else 0)) % 5 == 0
    crack = px in (4, 11) and 2 < y < FACE - 2 and noise(px, y // 3, seed + 1) > 0.35
    tone = mid if depth < 0.3 else dark if depth < 0.75 else deep
    if band:
        tone = dark if tone is mid else deep
    if crack:
        tone = deep
    if noise(px, y, seed + 2) > 0.86 and not crack:
        tone = light if depth < 0.5 else mid
    return tone


def side(x, y, pal, seed, west):
    """West or east face, periodic in y every 16 px. x counts from the outer edge inward."""
    light, mid, dark, deep, _ = pal
    py = y % 16
    if west:
        tone = (mid, light, light)[min(x - 1, 2)] if x >= 1 else dark
    else:
        tone = (dark, dark, mid)[min(x - 1, 2)] if x >= 1 else deep
    if noise(x, py, seed + 3) > 0.8:
        tone = mid if tone is not mid else dark
    return tone


def paint(pal, moss, seed):
    canvas = Image.new("RGBA", (48, 48), (0, 0, 0, 0))
    px = canvas.load()
    light, mid, dark, deep, outline = pal
    top = 47 - FACE
    for y in range(48):
        for x in range(48):
            if not inside(x, y):
                continue
            edge = not all(inside(x + dx, y + dy) for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)))
            if edge:
                px[x, y] = (*outline, 255)
                continue
            if y > top:
                c = face(x, y - top - 1, pal, seed)
                if x <= SIDE:
                    c = dark if c is mid else c
                if x >= 47 - SIDE:
                    c = deep if c in (dark, mid) else c
            elif y == top:
                c = moss[0] if moss and noise(x % 16, 0, seed + 5) > 0.3 else light
            elif x <= SIDE:
                c = side(x, y, pal, seed, True)
            elif x >= 47 - SIDE:
                c = side(47 - x, y, pal, seed, False)
            elif y == 1:
                c = moss[1] if moss and noise(x % 16, 1, seed + 6) > 0.5 else light
            else:
                continue
            px[x, y] = (*c, 255)
    return canvas


EARTH = [(176, 124, 100), (140, 96, 74), (110, 74, 57), (82, 58, 48), (62, 47, 52)]
STAIRS_CELL = 45


def stairs(pal, part):
    light, mid, dark, deep, outline = pal
    tile = Image.new("RGBA", (16, 16), (0, 0, 0, 0))
    px = tile.load()
    for y in range(16):
        step = y % 4
        tone = light if step == 0 else mid if step < 3 else dark
        if y >= 12:
            tone = {light: mid, mid: dark, dark: deep}[tone]
        for x in range(16):
            c = tone
            if part == 0 and x < 3:
                c = outline if x == 0 else dark if x == 1 else mid
            elif part == 2 and x > 12:
                c = outline if x == 15 else deep if x == 14 else dark
            elif noise(x, y, 7 + part) > 0.9 and step in (1, 2):
                c = dark
            px[x, y] = (*c, 255)
    return tile


def stairs_slot(material, part):
    i = STAIRS_CELL + material * 3 + part
    return (i % 24) * CELL + 1, BAND_Y + (i // 24) * CELL + 1


def cliff_slot(material, cell):
    if material == 0:
        i = DUSKMOON_CELL + cell
        return (i % 24) * CELL + 1, RIM_Y + (i // 24) * CELL + 1
    i = (material - 1) * 9 + cell
    return (i % 24) * CELL + 1, BAND_Y + (i // 24) * CELL + 1


def rows():
    return (STAIRS_CELL + (len(MATERIALS) + 1) * 3 + 23) // 24


def add_cliffs(atlas):
    out = Image.new("RGBA", (atlas.width, max(atlas.height, BAND_Y + rows() * CELL)), (0, 0, 0, 0))
    out.paste(atlas, (0, 0))
    for m, (name, pal, moss) in enumerate(MATERIALS, start=1):
        canvas = paint(pal, moss, m * 97)
        for row in range(3):
            for col in range(3):
                tile = canvas.crop((col * 16, row * 16, col * 16 + 16, row * 16 + 16))
                out.paste(tile, cliff_slot(m, row * 3 + col))
    for m, pal in enumerate([EARTH] + [p for _, p, _ in MATERIALS]):
        for part in range(3):
            out.paste(stairs(pal, part), stairs_slot(m, part))
    return out


def preview(atlas, path, zoom=6):
    img = Image.new("RGBA", ((len(MATERIALS) + 1) * 52, 72), (90, 150, 80, 255))
    for m in range(len(MATERIALS) + 1):
        for cell in range(9):
            x, y = cliff_slot(m, cell)
            img.alpha_composite(atlas.crop((x, y, x + 16, y + 16)), (m * 52 + cell % 3 * 16 + 2, cell // 3 * 16 + 2))
        for part in range(3):
            x, y = stairs_slot(m, part)
            img.alpha_composite(atlas.crop((x, y, x + 16, y + 16)), (m * 52 + part * 16 + 2, 54))
    img.resize((img.width * zoom, img.height * zoom), Image.Resampling.NEAREST).save(path)


if __name__ == "__main__":
    root = Path(__file__).resolve().parent.parent / "res" / "atlas.png"
    atlas = add_cliffs(Image.open(root).convert("RGBA"))
    if len(sys.argv) > 2 and sys.argv[1] == "--preview":
        preview(atlas, sys.argv[2])
        print(f"wrote {sys.argv[2]}")
    else:
        atlas.save(root)
        print(f"wrote {root}")
