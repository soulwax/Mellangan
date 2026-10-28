"""Paints the frontier band into res/atlas.png: badlands and taiga flora, standing stones and the mine.

Usage: uv run --with pillow tools/make_frontier.py [--preview out.png | --constants]
make_assets.py calls add_frontier() last. Layout must match src/Atlas.hpp (atlas::frontier / atlas::fr).
The band fills the free left half below the FX row: 12 columns from y=270. Tall sprites take two cells of one column,
top half in the upper row. Ground tiles are opaque and laid over the Tiny Town dirt fill.
"""
import sys
from pathlib import Path

from PIL import Image, ImageDraw

sys.path.insert(0, str(Path(__file__).resolve().parent))

from sprite_kit import Canvas, _hash
from make_sprites import (APPLE, BARK, BONE, DARK, FROST, GOLD, MOSS, OLIVE, OUTLINE_DARK, PALE, STONE, TAN,
                          boulder, dotted_bush, stump, tuft)

CELL, FRONTIER_Y, COLS, ROWS = 18, 270, 12, 11
TOWN_X = 12 * CELL

CLAY = ((140, 58, 42), (186, 92, 58), (220, 136, 88))
OCHRE = ((176, 112, 58), (214, 152, 86), (238, 196, 128))
JUNIPER = ((52, 92, 86), (78, 124, 106), (118, 162, 134))
TAIGA = ((24, 62, 54), (38, 88, 70), (62, 118, 88))
LARCH = ((168, 116, 36), (210, 160, 58), (240, 206, 106))
LICHEN = ((140, 150, 62), (180, 190, 86), (214, 220, 130))
RUNE = (128, 244, 255)
IRON = ((64, 64, 76), (104, 104, 118), (152, 152, 166))
SHELF = ((176, 96, 40), (218, 140, 62), (244, 190, 112))
GLOOM = ((14, 10, 16), (26, 20, 26), (40, 32, 38))
WOLF = ((84, 88, 100), (124, 128, 140), (170, 174, 186))
DIRE = ((46, 46, 58), (72, 72, 88), (108, 108, 126))
WORM = ((150, 88, 78), (196, 128, 108), (232, 176, 148))
NECRO = ((38, 24, 54), (66, 42, 90), (100, 68, 128))
CLOAK = ((36, 84, 58), (56, 118, 78), (92, 156, 102))
SKIN = (232, 184, 144)


def mesa(c):
    for y in range(6, 32):
        half = 5.2 + (y - 6) * 0.07 + (0.6 if y > 26 else 0)
        band = CLAY if (y // 3) % 3 else OCHRE
        for x in range(int(8 - half), int(8 + half) + 1):
            rel = (x - 8) / half
            c.put(x, y, band[2] if rel < -0.55 else band[0] if rel > 0.45 else band[1])
    for x in range(3, 14):
        c.put(x, 5, OCHRE[2])
        c.put(x, 6, OCHRE[1] if x > 4 else OCHRE[2])
    for x, y in ((5, 12), (10, 17), (6, 22), (11, 27)):
        c.put(x, y, CLAY[0]), c.put(x + 1, y, CLAY[0])


def hoodoo(c):
    for y in range(12, 32):
        half = 1.8 + abs((y - 21) / 5.0) ** 2 * 0.25 + (1.4 if y > 27 else 0)
        band = CLAY if (y // 2) % 3 else OCHRE
        for x in range(int(8 - half), int(8 + half) + 1):
            c.put(x, y, band[2] if x < 8 - half + 1.2 else band[0] if x > 8 + half - 1.2 else band[1])
    c.ellipse(8, 10, 4.6, 3, TAN, seed=70)


def juniper(c):
    c.line(8, 31, 7, 24, BARK[1], 2)
    c.line(7, 24, 10, 17, BARK[1], 2)
    c.line(9, 19, 4, 14, BARK[0])
    c.line(10, 18, 13, 12, BARK[0])
    for x, y, rx, ry, s in ((4, 12, 3.4, 2.4, 71), (12, 10, 3.6, 2.6, 72), (9, 6, 4.2, 3, 73), (7, 16, 2.6, 1.8, 74)):
        c.ellipse(x, y, rx, ry, JUNIPER, dither=0.2, seed=s)
    for x, y in ((5, 11), (11, 9), (9, 5), (13, 11)):
        c.put(x, y, (110, 130, 200))


def spruce(c, pal):
    c.rect(7, 28, 8, 31, BARK)
    tiers = ((1, 6, 2.2), (4, 10, 3.2), (8, 14, 4.2), (12, 19, 5.2), (16, 24, 6.2), (21, 29, 7.2))
    for top, bottom, half in tiers:
        c.tri(7.5, top, bottom, half, pal)
        for x in range(int(7.5 - half), int(7.5 + half) + 1, 3):
            c.put(x, bottom + 1, pal[0])


def rune_stone(c):
    for y in range(4, 32):
        half = 3.4 - max(0, 7 - y) * 0.45 + (0.6 if y > 28 else 0)
        for x in range(int(8 - half), int(8 + half) + 1):
            c.put(x, y, STONE[2] if x < 7 else STONE[0] if x > 9 else STONE[1])
    for x, y in ((7, 9), (8, 10), (9, 9), (8, 12), (8, 13), (7, 16), (9, 16), (8, 17), (7, 20), (8, 21), (9, 22)):
        c.put(x, y, RUNE)
    for x, y in ((6, 26), (10, 27), (6, 28)):
        c.put(x, y, MOSS[1])


def mine(c):
    for x in range(48):
        top = 3 + int(abs(x - 23.5) ** 1.6 * 0.09)
        for y in range(top, 32):
            k = _hash(x, y, 75)
            c.put(x, y, STONE[2] if y < top + 2 or k < 0.12 else STONE[0] if k > 0.86 else STONE[1])
    for x in range(14, 34):
        for y in range(12, 32):
            c.put(x, y, GLOOM[0] if y > 15 or 16 < x < 31 else GLOOM[1])
    for x0 in (12, 32):
        c.rect(x0, 11, x0 + 3, 31, BARK)
    for x in range(10, 38):
        for y in (8, 9, 10):
            c.put(x, y, BARK[2] if y == 8 else BARK[0] if y == 10 else BARK[1])
    for x in (12, 35):
        c.put(x, 9, IRON[2])
    for x in range(19, 29, 2):
        c.put(x, 30, BARK[1]), c.put(x, 31, BARK[0])
    for y in range(22, 32):
        c.put(20 - (y - 22) * 0.2, y, IRON[1]), c.put(27 + (y - 22) * 0.2, y, IRON[1])
    c.put(11, 12, IRON[1]), c.rect(10, 13, 11, 15, ((200, 120, 40), (250, 190, 80), (255, 230, 150)))


def ocotillo(c):
    for x1, top, lean in ((3, 3, -3), (5, 1, -2), (8, 0, 0), (11, 2, 2), (13, 4, 3)):
        c.line(8, 15, x1, top, OLIVE[0] if x1 % 2 else OLIVE[1])
        c.put(x1, top, (228, 64, 48)), c.put(x1, top + 1, (196, 52, 44))
    for x, y in ((6, 8), (10, 7), (7, 11), (9, 4)):
        c.put(x, y, LEAF_DOT)


LEAF_DOT = (104, 178, 88)


def red_rocks(c):
    c.ellipse(5, 12.5, 4, 2.8, CLAY, dither=0.1, seed=76)
    c.ellipse(11.5, 13, 3.4, 2.4, CLAY, seed=77)
    c.ellipse(8.5, 9, 3, 2.6, OCHRE, seed=78)


def ribs(c):
    c.line(2, 13, 14, 13, BONE[1])
    for x in (4, 7, 10, 13):
        c.curve(x, 13, -1.5, -6, 2.5, BONE[2 if x < 9 else 1])
    c.ellipse(2.5, 12, 1.6, 1.4, BONE, seed=79)


def lichen_rock(c):
    boulder(c, STONE)
    for x, y in ((4, 9), (5, 9), (8, 8), (9, 8), (11, 10), (6, 12), (12, 12)):
        c.put(x, y, LICHEN[2] if x % 2 else LICHEN[1])


def cotton_grass(c):
    for x, h, lean in ((3, 9, -1), (6, 12, 0), (9, 11, 1), (12, 8, 1)):
        c.line(x, 15, x + lean, 15 - h, OLIVE[0])
        c.ellipse(x + lean, 14 - h, 1.6, 1.5, ((210, 210, 200), (236, 236, 228), (252, 252, 248)), seed=x)


def shelf_stump(c):
    stump(c)
    for x, y in ((3, 11), (12, 12), (3, 13)):
        c.ellipse(x, y, 2, 0.9, SHELF, seed=x + y)


def toppled_stone(c):
    for x in range(1, 15):
        for y in range(9, 15):
            if (x, y) in ((1, 9), (14, 9), (1, 14), (14, 14)):
                continue
            c.put(x, y, STONE[2] if y == 9 else STONE[0] if y > 12 else STONE[1])
    for x, y in ((4, 11), (5, 12), (8, 11), (11, 12), (12, 11)):
        c.put(x, y, RUNE)


def altar(c):
    c.rect(3, 11, 5, 15, STONE)
    c.rect(10, 11, 12, 15, STONE)
    for x in range(1, 15):
        c.put(x, 8, STONE[2]), c.put(x, 9, STONE[1]), c.put(x, 10, STONE[0])
    c.put(7, 7, RUNE), c.put(8, 7, RUNE), c.put(8, 6, (220, 252, 255))


def mine_cart(c):
    c.ellipse(4.5, 14, 1.6, 1.6, IRON, seed=80)
    c.ellipse(11.5, 14, 1.6, 1.6, IRON, seed=81)
    for y in range(8, 13):
        for x in range(2 + (y - 8) // 3, 15 - (y - 8) // 3):
            c.put(x, y, IRON[2] if y == 8 else IRON[0] if x > 12 else IRON[1])
    c.ellipse(6, 7, 2.4, 1.8, STONE, seed=82)
    c.ellipse(10, 6.5, 2.6, 2, STONE, seed=83)
    for x, y in ((6, 6), (10, 5), (11, 7), (5, 8)):
        c.put(x, y, GOLD[2])


def ore_pile(c):
    c.ellipse(8, 12, 6.4, 3.6, STONE, dither=0.2, seed=84)
    c.ellipse(8, 9, 3.4, 2.4, STONE, seed=85)
    for x, y in ((5, 11), (9, 8), (11, 12), (7, 13), (12, 10)):
        c.put(x, y, GOLD[2]), c.put(x + 1, y, GOLD[1])


def lantern_post(c):
    c.rect(3, 2, 4, 15, BARK)
    c.line(4, 2, 10, 2, BARK[1])
    c.line(9, 3, 9, 4, IRON[0])
    c.rect(8, 5, 10, 8, ((200, 120, 40), (250, 190, 80), (255, 230, 150)))
    c.line(8, 9, 10, 9, IRON[0])


def timber(c):
    c.rect(2, 3, 4, 15, BARK)
    c.rect(11, 3, 13, 15, BARK)
    for x in range(1, 15):
        c.put(x, 2, BARK[2])
        c.put(x, 3, BARK[1])
    c.line(5, 4, 10, 9, BARK[0])


def wolf(c, pal, eye, mane=None):
    for x in (4, 6, 10, 12):
        c.line(x, 11, x, 15, pal[0])
    c.curve(3, 9, -3, -4, 0, pal[1])
    c.ellipse(7.5, 10, 5.4, 2.9, pal, seed=90)
    for x in range(4, 11):
        c.put(x, 12, pal[2])
    if mane:
        c.ellipse(10.5, 8, 3.2, 3.4, mane, seed=91)
    c.ellipse(12.5, 7, 2.8, 2.3, pal, seed=92)
    c.rect(14, 7, 15, 8, pal)
    c.put(15, 7, OUTLINE_DARK)
    c.put(11, 4, pal[1])
    c.put(12, 4, pal[2])
    c.put(11, 3, pal[2])
    c.put(13, 6, eye)


def burrower(c):
    c.ellipse(8, 14, 6.5, 2, OCHRE, seed=93)
    for cy, rx in ((12, 4.2), (9, 3.8), (6.5, 3.4)):
        c.ellipse(8, cy, rx, 2.2, WORM, seed=int(cy * 3))
    c.ellipse(8, 4, 3.6, 2.6, WORM, seed=94)
    c.ellipse(8, 3.6, 2, 1.4, GLOOM, seed=95)
    for x, y in ((6, 3), (10, 3), (7, 5), (9, 5)):
        c.put(x, y, BONE[2])
    c.line(4, 2, 5, 4, WORM[0])
    c.line(12, 2, 11, 4, WORM[0])


def mound(c):
    c.ellipse(8, 13, 6, 2.6, OCHRE, dither=0.25, seed=96)
    c.line(5, 13, 7, 12, CLAY[0]), c.line(9, 12, 11, 14, CLAY[0])
    for x, y in ((3, 14), (13, 13), (8, 11)):
        c.put(x, y, STONE[1])


def necromancer(c):
    c.line(13, 2, 13, 15, BARK[1])
    c.ellipse(13, 2, 1.6, 1.4, BONE, seed=97)
    c.tri(7.5, 5, 15, 5.5, NECRO)
    c.ellipse(7.5, 5, 3.6, 3.4, NECRO, seed=98)
    c.ellipse(7.5, 6, 2.2, 1.8, GLOOM, seed=99)
    c.put(6, 6, (120, 255, 140)), c.put(9, 6, (120, 255, 140))
    c.put(12, 9, BONE[2]), c.put(4, 10, BONE[1])
    for x in range(3, 13, 3):
        c.put(x, 15, NECRO[0])


def skeleton(c):
    c.line(6, 11, 5, 15, BONE[1]) 
    c.line(10, 11, 11, 15, BONE[1])
    c.line(8, 6, 8, 11, BONE[1])
    for y in (8, 10):
        c.line(5, y, 11, y, BONE[2 if y == 8 else 1])
    c.line(5, 7, 3, 11, BONE[1])
    c.line(11, 7, 13, 11, BONE[1])
    c.rect(6, 11, 10, 11, BONE)
    c.ellipse(8, 4, 3.2, 2.9, BONE, seed=100)
    c.put(7, 4, OUTLINE_DARK)
    c.put(9, 4, OUTLINE_DARK)
    c.put(8, 6, OUTLINE_DARK)


def thief(c):
    c.line(6, 12, 6, 15, GLOOM[2]), c.line(9, 12, 9, 15, GLOOM[2])
    c.tri(7.5, 5, 13, 5, CLOAK)
    c.ellipse(7.5, 5, 3.6, 3.4, CLOAK, seed=101)
    c.ellipse(7.5, 6.5, 2.2, 1.6, (SKIN, SKIN, SKIN), seed=102)
    c.line(5, 6, 10, 6, GLOOM[1])
    c.put(6, 6, (250, 250, 240)), c.put(9, 6, (250, 250, 240))
    c.ellipse(12, 10.5, 3, 3.2, TAN, seed=103)
    c.line(11, 7, 13, 7, BARK[0])
    c.put(12, 9, GOLD[2]), c.put(11, 11, GOLD[1])


def chakram(c):
    c.ellipse(7.5, 7.5, 6.0, 6.0, IRON, seed=110)
    c.ellipse(7.5, 7.5, 3.8, 3.8, GLOOM, seed=111)
    c.ellipse(7.5, 7.5, 2.0, 2.0, GOLD, seed=112)
    c.put(7, 7, RUNE), c.put(8, 8, RUNE)
    for x, y in ((2, 7), (13, 7), (7, 2), (7, 13)):
        c.put(x, y, FROST[2])


def glaive(c):
    c.line(2, 14, 10, 6, BARK[1])
    c.line(1, 15, 3, 13, BARK[0])
    c.line(9, 5, 14, 2, IRON[2])
    c.line(8, 6, 13, 1, IRON[1])
    c.line(10, 6, 15, 3, IRON[0])
    c.put(14, 1, (240, 240, 255)), c.put(15, 2, (240, 240, 255))
    c.put(8, 7, (210, 45, 45)), c.put(9, 7, (220, 50, 50)), c.put(8, 8, (160, 25, 25))


def censer(c):
    c.line(7, 1, 7, 5, GOLD[1])
    c.line(5, 5, 10, 5, GOLD[2])
    c.ellipse(7.5, 9.5, 4.2, 3.8, GOLD, seed=113)
    c.rect(5, 7, 10, 8, GOLD)
    for x in (5, 7, 9):
        c.put(x, 9, (255, 150, 30))
    c.put(6, 6, (200, 200, 220)), c.put(8, 5, (220, 220, 240)), c.put(7, 3, (240, 240, 250))
    c.put(7, 13, GOLD[0]), c.put(7, 14, GOLD[1])


def needle_tuft(c):
    tuft(c, ((70, 64, 40), (100, 92, 56), (130, 122, 76)))


def clay_tuft(c):
    tuft(c, OLIVE, ((5, 10), (11, 11)))


def rails(vertical):
    def fn(img):
        d = img.load()

        def put(along, across, c):
            d[(across, along) if vertical else (along, across)] = (*c, 255)

        for along in range(1, 16, 4):
            for across in range(2, 14):
                put(along, across, BARK[1])
                put(along + 1, across, BARK[0])
        for along in range(16):
            for r in (4, 11):
                put(along, r, IRON[2])
                put(along, r + 1, IRON[0])
    return fn


TALL = [
    ("mesa", mesa),
    ("hoodoo", hoodoo),
    ("juniper", juniper),
    ("taiga spruce", lambda c: spruce(c, TAIGA)),
    ("larch", lambda c: spruce(c, LARCH)),
    ("rune stone", rune_stone),
]

WIDE = [("mine", mine, 3)]

SINGLES = [
    ("ocotillo", ocotillo),
    ("red rocks", red_rocks),
    ("ribs", ribs),
    ("clay tuft", clay_tuft),
    ("lingonberry", lambda c: dotted_bush(c, APPLE, DARK, 86)),
    ("lichen rock", lichen_rock),
    ("cotton grass", cotton_grass),
    ("needle tuft", needle_tuft),
    ("shelf stump", shelf_stump),
    ("toppled stone", toppled_stone),
    ("altar", altar),
    ("mine cart", mine_cart),
    ("ore pile", ore_pile),
    ("lantern post", lantern_post),
    ("timber", timber),
]

GROUNDS = [("rail across", rails(False)), ("rail down", rails(True))]

MOBS = [
    ("wolf", lambda c: wolf(c, WOLF, (250, 210, 80))),
    ("alpha", lambda c: wolf(c, DIRE, (255, 70, 60), BONE)),
    ("burrower", burrower),
    ("mound", mound),
    ("necromancer", necromancer),
    ("skeleton", skeleton),
    ("thief", thief),
]

WEAPONS = [
    ("chakram", chakram),
    ("glaive", glaive),
    ("censer", censer),
]

BARE = {"clay tuft", "needle tuft"}


def layout():
    slots, col = [], 0
    for name, fn in TALL:
        slots.append((name, fn, "tall", col, COLS + col))
        col += 1
    for name, fn, w in WIDE:
        for k in range(w):
            slots.append((f"{name} {('left', 'mouth', 'right')[k]}", (fn, k, w), "wide", col + k, COLS + col + k))
        col += w
    free = [row * COLS + c for row in (0, 1) for c in range(col, COLS)] + list(range(2 * COLS, ROWS * COLS))
    items = ([(n, f, "single") for n, f in SINGLES] + [(n, f, "ground") for n, f in GROUNDS] +
             [(n, f, "single") for n, f in MOBS] + [(n, f, "single") for n, f in WEAPONS])
    assert len(items) <= len(free)
    for k, (name, fn, kind) in enumerate(items):
        slots.append((name, fn, kind, free[k], -1))
    return slots


def cell_pos(index):
    return (index % COLS) * CELL + 1, FRONTIER_Y + (index // COLS) * CELL + 1


def town(atlas, index):
    x, y = TOWN_X + (index % 12) * CELL + 1, (index // 12) * CELL + 1
    return atlas.crop((x, y, x + 16, y + 16))


def render(atlas, name, fn, kind):
    if kind == "tall":
        c = Canvas(16, 32)
        fn(c)
        return c.image()
    if kind == "wide":
        paint, k, w = fn
        c = Canvas(16 * w, 32)
        paint(c)
        return c.image().crop((16 * k, 0, 16 * k + 16, 32))
    if kind == "ground":
        img = town(atlas, 25).copy()
        fn(img)
        return img
    c = Canvas(16, 16)
    fn(c)
    return c.image(name not in BARE)


def add_frontier(atlas):
    out = atlas.copy()
    out.paste(Image.new("RGBA", (COLS * CELL, ROWS * CELL), (0, 0, 0, 0)), (0, FRONTIER_Y))
    for name, fn, kind, top, foot in layout():
        img = render(atlas, name, fn, kind)
        if foot >= 0:
            out.paste(img.crop((0, 0, 16, 16)), cell_pos(top))
            out.paste(img.crop((0, 16, 16, 32)), cell_pos(foot))
        else:
            out.paste(img, cell_pos(top))
    return out


def constants():
    def ident(n):
        return ''.join(w.capitalize() for w in n.split())
    lines = []
    for name, _, kind, top, foot in layout():
        lines.append(f"constexpr int {ident(name)}High = {top}, {ident(name)}Low = {foot};" if foot >= 0
                     else f"constexpr int {ident(name)} = {top};")
    return "\n".join(lines)


def preview(atlas, path, zoom=4):
    grass = town(atlas, 0)
    slots = layout()
    cols = 9
    rows = (len(slots) + cols - 1) // cols
    w, h = cols * 20, rows * 46
    img = Image.new("RGBA", (w, h), (40, 40, 60, 255))
    for i, (name, fn, kind, _, _) in enumerate(slots):
        x, y = (i % cols) * 20 + 2, (i // cols) * 46 + 10
        for gy in range(0, 32, 16):
            img.paste(grass, (x, y + gy))
        spr = render(atlas, name, fn, kind)
        img.alpha_composite(spr, (x, y + 32 - spr.height))
    big = img.resize((w * zoom, h * zoom), Image.NEAREST)
    d = ImageDraw.Draw(big)
    for i, (name, *_rest) in enumerate(slots):
        d.text(((i % cols) * 20 * zoom + 6, (i // cols) * 46 * zoom + 4), name, fill=(255, 230, 120, 255))
    big.save(path)


if __name__ == "__main__":
    root = Path(__file__).resolve().parent.parent / "res" / "atlas.png"
    atlas = Image.open(root).convert("RGBA")
    if len(sys.argv) > 1 and sys.argv[1] == "--constants":
        print(constants())
    elif len(sys.argv) > 2 and sys.argv[1] == "--preview":
        preview(atlas, sys.argv[2])
        print(f"wrote {sys.argv[2]}")
    else:
        add_frontier(atlas).save(root)
        print(f"wrote {root}")
