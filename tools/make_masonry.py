"""Adds the masonry band to res/atlas.png: Tiny Town building rows 3-10 recoloured in four biome materials.

Materials, in order: sandstone, frost, moss, ash. Two materials share each 8-row band row.
Usage: uv run --with pillow tools/make_masonry.py
make_assets.py calls add_masonry() too, so a full rebuild keeps the band. Layout must match atlas::masonry in src/Atlas.hpp.
"""
from pathlib import Path

from PIL import Image

from atlas_layout import extend

CELL, TOWN_X, FIRST_ROW, ROWS = 18, 12 * 18, 3, 8
MASONRY_Y = 1488
OUTLINE = {(63, 38, 49), (61, 33, 45)}
CAPS = set(range(44, 48)) | {56, 59, 80, 81, 82} | set(range(48, 56)) | {63, 67} | set(range(96, 102))

STONE_LIGHT, STONE, STONE_DARK, STONE_DEEP, STONE_LINE = (
    (192, 203, 220), (139, 155, 180), (90, 105, 136), (82, 96, 124), (38, 43, 68))
WOOD_HIGH, WOOD_LIGHT, WOOD, WOOD_DARK = (252, 188, 143), (234, 165, 108), (189, 108, 74), (118, 59, 54)
ROOF_LIGHT, ROOF = (242, 132, 98), (195, 75, 53)
GRASS, GRASS_DARK = (132, 198, 105), (101, 165, 86)

SANDSTONE = {
    STONE_LIGHT: (240, 216, 164), STONE: (214, 178, 118), STONE_DARK: (172, 130, 84),
    STONE_DEEP: (150, 110, 70), STONE_LINE: (96, 62, 46),
    WOOD_HIGH: (250, 228, 184), WOOD_LIGHT: (228, 198, 142), WOOD: (184, 140, 92), WOOD_DARK: (122, 84, 60),
    ROOF_LIGHT: (232, 146, 84), ROOF: (194, 100, 56),
    GRASS: (204, 186, 112), GRASS_DARK: (172, 152, 92),
}
FROST = {
    STONE_LIGHT: (232, 240, 250), STONE: (178, 198, 224), STONE_DARK: (112, 132, 172),
    STONE_DEEP: (98, 116, 152),
    WOOD_HIGH: (216, 174, 144), WOOD_LIGHT: (192, 140, 104), WOOD: (142, 92, 74), WOOD_DARK: (92, 54, 60),
    ROOF_LIGHT: (204, 116, 104), ROOF: (156, 74, 70),
    GRASS: (226, 236, 248), GRASS_DARK: (190, 206, 228),
}
MOSS = {
    STONE_LIGHT: (178, 194, 176), STONE: (128, 148, 132), STONE_DARK: (84, 100, 96),
    STONE_DEEP: (74, 90, 86), STONE_LINE: (36, 46, 48),
    WOOD_HIGH: (206, 166, 122), WOOD_LIGHT: (178, 132, 92), WOOD: (136, 86, 62), WOOD_DARK: (88, 50, 46),
    ROOF_LIGHT: (196, 118, 92), ROOF: (150, 70, 54),
    GRASS: (104, 162, 84), GRASS_DARK: (78, 132, 72),
}
ASH = {
    STONE_LIGHT: (116, 106, 110), STONE: (84, 76, 84), STONE_DARK: (56, 50, 58),
    STONE_DEEP: (48, 42, 50), STONE_LINE: (22, 16, 22),
    WOOD_HIGH: (140, 102, 82), WOOD_LIGHT: (112, 76, 62), WOOD: (78, 52, 48), WOOD_DARK: (50, 32, 36),
    ROOF_LIGHT: (98, 70, 66), ROOF: (68, 46, 46),
    GRASS: (122, 112, 104), GRASS_DARK: (96, 88, 84),
}
SNOW, SNOW_SHADE = (240, 245, 252), (204, 220, 240)
MOSS_TONES = ((150, 190, 104), (106, 152, 82), (68, 108, 70))
EMBER, EMBER_HOT = (214, 92, 40), (250, 168, 72)


def hashed(x, y, salt):
    h = (x * 374761393 + y * 668265263 + salt * 2246822519) & 0xFFFFFFFF
    h = ((h ^ (h >> 13)) * 1274126177) & 0xFFFFFFFF
    return (h ^ (h >> 16)) / 0xFFFFFFFF


def near(rgb, palette):
    for key in palette:
        if sum(abs(a - b) for a, b in zip(rgb, key)) <= 6:
            return key
    return None


def outline(rgb):
    return near(rgb, OUTLINE) is not None


def remap(px, w, h, palette):
    for y in range(h):
        for x in range(w):
            r, g, b, a = px[x, y]
            if not a:
                continue
            key = near((r, g, b), palette)
            if key:
                px[x, y] = (*palette[key], a)


def snow_caps(px, cols):
    for index in CAPS:
        ox, oy = (index % 12) * CELL + 1, (index // 12 - FIRST_ROW) * CELL + 1
        if not 0 <= index // 12 - FIRST_ROW < ROWS or index % 12 >= cols:
            continue
        for x in range(ox, ox + 16):
            y = oy
            while y < oy + 16 and not px[x, y][3]:
                y += 1
            while y < oy + 16 and px[x, y][3] and outline(px[x, y][:3]):
                y += 1
            for depth, tone in enumerate((SNOW, SNOW, SNOW_SHADE)):
                if y + depth >= oy + 16:
                    break
                c = px[x, y + depth]
                if not c[3] or outline(c[:3]):
                    break
                if depth == 2 and hashed(x, y, 7) < 0.5:
                    break
                px[x, y + depth] = (*tone, c[3])


def moss(px, w, h):
    surfaces = {v: i for i, v in enumerate((MOSS[STONE_LIGHT], MOSS[STONE], MOSS[STONE_DARK],
                                            MOSS[STONE_DEEP], MOSS[ROOF_LIGHT], MOSS[ROOF]))}
    for y in range(h):
        for x in range(w):
            c = px[x, y]
            if not c[3] or c[:3] not in surfaces:
                continue
            clump = hashed(x // 3, y // 2, 11) < 0.26 + 0.1 * (y % CELL) / CELL
            if clump and hashed(x, y, 13) < 0.8:
                tone = MOSS_TONES[min(2, surfaces[c[:3]] % 4)]
                px[x, y] = (*tone, c[3])


def embers(px, w, h):
    roof = (ASH[ROOF_LIGHT], ASH[ROOF])
    for y in range(h):
        for x in range(w):
            c = px[x, y]
            if c[3] and c[:3] in roof and hashed(x, y, 17) < 0.035:
                px[x, y] = (*(EMBER_HOT if hashed(x, y, 19) < 0.3 else EMBER), c[3])


def material(base, palette, extra):
    band = base.copy()
    px = band.load()
    remap(px, band.width, band.height, palette)
    extra(px, band.width, band.height)
    return band


def add_masonry(atlas):
    out = extend(atlas)
    base = atlas.crop((TOWN_X, FIRST_ROW * CELL, TOWN_X + 12 * CELL, (FIRST_ROW + ROWS) * CELL))
    looks = (
        (SANDSTONE, lambda px, w, h: None),
        (FROST, lambda px, w, h: snow_caps(px, 12)),
        (MOSS, moss),
        (ASH, embers),
    )
    for m, (palette, extra) in enumerate(looks):
        out.paste(material(base, palette, extra), ((m % 2) * 12 * CELL, MASONRY_Y + (m // 2) * ROWS * CELL))
    return out


if __name__ == "__main__":
    path = Path(__file__).resolve().parent.parent / "res" / "atlas.png"
    add_masonry(Image.open(path).convert("RGBA")).save(path)
    print(f"wrote {path}")
