"""Paints the sprite kit band into res/atlas.png: trees, shrubs, flowers, rocks and biome-transition plants.

Usage: uv run --with pillow tools/make_sprites.py [--preview out.png]
make_assets.py calls add_sprites() too. Layout must match src/Atlas.hpp (atlas::kit / atlas::kt).
Trees are two cells tall: their top half sits in row 0 and their foot in row 1 of the same column.
"""
import sys
from pathlib import Path

from PIL import Image, ImageDraw

sys.path.insert(0, str(Path(__file__).resolve().parent))

from sprite_kit import Canvas, angle

CELL, KIT_Y, COLS = 18, 480, 24

LEAF = ((71, 159, 74), (104, 178, 88), (132, 198, 105))
LIME = ((86, 150, 52), (136, 192, 66), (186, 224, 104))
OLIVE = ((118, 128, 52), (160, 164, 70), (204, 198, 104))
DARK = ((32, 88, 64), (48, 118, 78), (76, 150, 92))
FROST = ((150, 176, 206), (198, 218, 236), (240, 246, 252))
BARK = ((92, 60, 44), (128, 86, 56), (160, 112, 72))
PALE = ((170, 160, 150), (214, 206, 196), (244, 240, 232))
CHAR = ((34, 30, 34), (58, 52, 56), (86, 78, 80))
RED = ((150, 36, 44), (200, 56, 60), (236, 104, 96))
STONE = ((104, 106, 120), (140, 144, 158), (182, 186, 198))
SANDSTONE = ((168, 102, 60), (204, 138, 82), (232, 178, 118))
ICE = ((92, 150, 196), (150, 206, 234), (220, 244, 252))
OBSIDIAN = ((34, 24, 46), (66, 44, 84), (120, 92, 150))
CRYSTAL = ((90, 60, 170), (140, 110, 230), (206, 186, 255))
GOLD = ((176, 128, 40), (220, 178, 70), (248, 222, 120))
PURPLE = ((104, 60, 140), (150, 96, 196), (200, 160, 236))
MOSS = ((60, 110, 50), (90, 146, 60), (130, 180, 80))


def birch(c, leaves):
    for y in range(13, 32):
        c.put(7, y, PALE[2]), c.put(8, y, PALE[1]), c.put(9, y, PALE[0])
        if y % 4 == 1:
            c.put(7 + y % 3, y, CHAR[0])
    c.ellipse(8, 9, 6.5, 7, leaves, dither=0.18, seed=1)
    c.ellipse(4.5, 14, 3, 3, leaves, seed=2)
    c.ellipse(11.5, 13, 3, 3.5, leaves, seed=3)


def cypress(c):
    c.rect(7, 26, 9, 31, BARK)
    c.ellipse(8, 14, 4.5, 13.5, DARK, dither=0.2, seed=4)


def acacia(c):
    c.line(8, 31, 8, 20, BARK[1], 2)
    c.line(8, 21, 3, 12, BARK[1], 2)
    c.line(9, 20, 13, 13, BARK[1], 2)
    c.ellipse(8, 10, 7.6, 3.4, OLIVE, seed=5)
    c.ellipse(4, 12, 3.5, 2, OLIVE, seed=6)
    c.ellipse(12, 11.5, 3.5, 2, OLIVE, seed=7)


def baobab(c):
    for y in range(12, 32):
        half = 2 + (y - 12) * 0.18
        for x in range(int(8 - half), int(8 + half) + 1):
            c.put(x, y, BARK[2] if x < 8 - half + 1.5 else BARK[0] if x > 8 + half - 1.5 else BARK[1])
    for dx in (-5, -2, 2, 5):
        c.line(8, 13, 8 + dx, 7, BARK[1])
        c.ellipse(8 + dx, 6, 2.2, 1.8, LEAF, seed=dx + 20)


def mushroom_tree(c):
    c.rect(6, 15, 10, 31, PALE)
    c.ellipse(8, 10, 7.6, 6.5, RED, seed=8)
    for x, y in ((5, 7), (9, 6), (11, 10), (4, 11), (8, 12)):
        c.put(x, y, PALE[2]), c.put(x + 1, y, PALE[2])


def jungle(c):
    c.line(8, 31, 9, 16, BARK[1], 2)
    for deg, r, droop in ((200, 7, 3), (240, 7, 2), (290, 7, 2), (330, 7, 3), (160, 6, 4), (20, 6, 4)):
        dx, dy = angle(deg, r)
        for k in range(4):
            c.curve(9 + k * 0.1, 14 + k * 0.4, dx, dy, droop, LIME[1 + (k == 0)], 1)
    c.ellipse(9, 14, 3.5, 3, LIME, seed=9)


def charred(c):
    c.line(8, 31, 8, 9, CHAR[1], 2)
    for x0, y0, x1, y1 in ((8, 16, 3, 10), (9, 13, 13, 7), (8, 20, 13, 15), (8, 11, 5, 5)):
        c.line(x0, y0, x1, y1, CHAR[1])
    for x, y in ((3, 10), (13, 7), (13, 15), (8, 25)):
        c.put(x, y, (240, 120, 40))


def crystal_spire(c):
    for cx, top, half, pal in ((8, 4, 3.5, CRYSTAL), (4, 16, 2.5, ICE), (12, 13, 2.5, CRYSTAL)):
        for y in range(top, 31):
            w = min(half, (y - top) * 0.7 + 0.5)
            for x in range(int(cx - w), int(cx + w) + 1):
                c.put(x, y, pal[2] if x < cx else pal[0] if x > cx else pal[1])


def fir(c, snowy):
    c.rect(7, 27, 9, 31, BARK)
    for top, bottom, half in ((2, 12, 4.5), (8, 19, 6), (14, 27, 7.5)):
        c.tri(8, top, bottom, half, DARK)
        if snowy:
            for x in range(int(8 - half * 0.8), int(8 + half * 0.8) + 1, 2):
                c.put(x, bottom - 1, FROST[2])
            c.put(8, top, FROST[2])


def fern(c, tips=None):
    for deg, r in ((200, 8), (225, 10), (250, 12), (272, 12), (295, 12), (320, 10), (345, 8)):
        dx, dy = angle(deg, r)
        for t in range(1, 11):
            k = t / 10
            x, y = 8 + dx * k, 15 + dy * k + 3 * k * k
            c.put(x, y, LEAF[2] if k < 0.5 else LEAF[1])
            if 0.3 < k < 0.9:
                c.put(x + (1 if dx > 0 else -1), y + 1, LEAF[0])
        if tips:
            c.put(8 + dx, 15 + dy + 3, tips)



def tall_grass(c, pal=LEAF, seed=0):
    for i, (x, h, lean) in enumerate(((3, 9, -2), (5, 12, -1), (7, 13, 0), (9, 11, 1), (11, 12, 2), (13, 8, 3))):
        c.line(x, 15, x + lean, 15 - h, pal[i % 3])


def heather(c):
    c.ellipse(8, 11, 6.5, 4.5, DARK, seed=10)
    for x, y in ((4, 9), (7, 8), (10, 9), (12, 11), (6, 11), (9, 12), (3, 12)):
        c.put(x, y, PURPLE[2]), c.put(x, y + 1, PURPLE[1])


def thornbush(c):
    for x0, y0, x1, y1 in ((8, 15, 2, 6), (8, 15, 14, 7), (8, 15, 6, 4), (8, 15, 11, 3), (4, 10, 9, 8), (12, 10, 7, 7)):
        c.line(x0, y0, x1, y1, BARK[0])
    for x, y in ((5, 7), (11, 5), (9, 9), (3, 10)):
        c.put(x, y, RED[2])


def sunflower(c):
    c.line(8, 15, 8, 8, LEAF[0])
    c.ellipse(5.5, 12, 2.2, 1.2, LEAF, seed=11)
    c.ellipse(10.5, 10.5, 2.2, 1.2, LEAF, seed=15)
    c.ellipse(8, 5, 3.6, 3.6, GOLD, seed=16)
    c.ellipse(8, 5, 1.6, 1.6, BARK, seed=12)



def tulips(c):
    for x, h, col in ((4, 7, RED), (8, 10, GOLD), (12, 8, RED)):
        c.line(x, 15, x, 15 - h, LEAF[0])
        c.rect(x - 1, 14 - h, x + 1, 16 - h, col)


def lavender(c):
    for x, h in ((3, 8), (6, 11), (9, 12), (12, 9)):
        c.line(x, 15, x, 15 - h, LEAF[0])
        for y in range(15 - h, 15 - h + 5):
            c.put(x, y, PURPLE[(y + x) % 3])


def wheat(c):
    for x, h in ((3, 10), (6, 12), (9, 13), (12, 11)):
        c.line(x, 15, x, 15 - h, GOLD[0])
        for y in range(15 - h, 15 - h + 4):
            c.put(x - 1, y, GOLD[2]), c.put(x + 1, y, GOLD[1])


def cattails(c):
    for x, h in ((4, 11), (8, 13), (11, 10)):
        c.line(x, 15, x, 15 - h, OLIVE[0])
        c.rect(x - 1, 16 - h, x, 19 - h, BARK)
    c.line(6, 15, 3, 9, OLIVE[1]), c.line(10, 15, 14, 8, OLIVE[1])


def boulder(c, pal, top=None):
    c.ellipse(8, 11, 6.5, 4.8, pal, dither=0.15, seed=13)
    if top:
        c.ellipse(7, 8, 5, 2, top, seed=14)


def sandstone(c):
    for y, x0, x1, pal in ((14, 2, 13, SANDSTONE), (11, 3, 12, SANDSTONE), (8, 5, 10, SANDSTONE)):
        for yy in (y, y + 1, y + 2):
            for x in range(x0, x1 + 1):
                c.put(x, yy, pal[2] if yy == y else pal[0] if yy == y + 2 else pal[1])


def ice_chunk(c):
    for x, top, half in ((6, 4, 3), (10, 7, 3)):
        for y in range(top, 15):
            w = min(half, (y - top) * 0.8 + 0.5)
            for xx in range(int(x - w), int(x + w) + 1):
                c.put(xx, y, ICE[2] if xx < x else ICE[0] if xx > x else ICE[1])


def obsidian(c):
    for x, top, half in ((5, 6, 2), (9, 3, 2.5), (12, 8, 1.5)):
        for y in range(top, 15):
            w = min(half, (y - top) * 0.6 + 0.5)
            for xx in range(int(x - w), int(x + w) + 1):
                c.put(xx, y, OBSIDIAN[2] if xx < x else OBSIDIAN[0] if xx > x else OBSIDIAN[1])


def pebbles(c):
    for x, y, r in ((4, 12, 2), (9, 13, 2.2), (12, 10, 1.6), (7, 9, 1.3)):
        c.ellipse(x, y, r, r * 0.8, STONE, seed=x)


def tuft(c, blades, dots=None):
    for x, h, lean in ((4, 4, -1), (6, 6, -1), (8, 7, 0), (10, 5, 1), (12, 4, 1)):
        for k in range(h):
            c.put(x + lean * k / h, 15 - k, blades[0] if k < h * 0.35 else blades[1] if k < h * 0.75 else blades[2])
    for x, y in dots or ():
        c.put(x, y, (250, 250, 244))



def daisies(c):
    for x, y in ((4, 10), (9, 8), (12, 12), (6, 13)):
        c.line(x, 15, x, y + 1, LEAF[0])
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            c.put(x + dx, y + dy, (250, 250, 244))
        c.put(x, y, GOLD[2])


TREES = [
    ("birch", lambda c: birch(c, LEAF)),
    ("frost birch", lambda c: birch(c, FROST)),
    ("cypress", cypress),
    ("acacia", acacia),
    ("baobab", baobab),
    ("mushroom tree", mushroom_tree),
    ("jungle tree", jungle),
    ("charred tree", charred),
    ("crystal spire", crystal_spire),
    ("fir", lambda c: fir(c, False)),
    ("snowy fir", lambda c: fir(c, True)),
]

SINGLES = [
    ("fern", fern),
    ("frosted fern", lambda c: fern(c, FROST[2])),
    ("tall grass", tall_grass),
    ("dry grass", lambda c: tall_grass(c, OLIVE)),
    ("heather", heather),
    ("thornbush", thornbush),
    ("sunflower", sunflower),
    ("tulips", tulips),
    ("lavender", lavender),
    ("wheat", wheat),
    ("cattails", cattails),
    ("daisies", daisies),
    ("boulder", lambda c: boulder(c, STONE)),
    ("mossy boulder", lambda c: boulder(c, STONE, MOSS)),
    ("snowy boulder", lambda c: boulder(c, STONE, FROST)),
    ("sandstone", sandstone),
    ("ice chunk", ice_chunk),
    ("obsidian", obsidian),
    ("pebbles", pebbles),
    ("sand tuft", lambda c: tuft(c, OLIVE)),
    ("frost tuft", lambda c: tuft(c, FROST)),
    ("ash tuft", lambda c: tuft(c, CHAR)),
    ("bog tuft", lambda c: tuft(c, MOSS)),
    ("bloom tuft", lambda c: tuft(c, LEAF, ((6, 9), (10, 10)))),
]


def single_slot(i):
    free = [(col, row) for row in (0, 1) for col in range(len(TREES), COLS)]
    return free[i]


BARE = {"sand tuft", "frost tuft", "ash tuft", "bog tuft", "bloom tuft"}


def paint(fn, h, outline=True):
    c = Canvas(16, h)
    fn(c)
    return c.image(outline)


def add_sprites(atlas):
    out = Image.new("RGBA", (atlas.width, max(atlas.height, KIT_Y + 2 * CELL)), (0, 0, 0, 0))
    out.paste(atlas.crop((0, 0, atlas.width, min(atlas.height, KIT_Y))), (0, 0))
    for col, (_, fn) in enumerate(TREES):
        tall = paint(fn, 32)
        out.paste(tall.crop((0, 0, 16, 16)), (col * CELL + 1, KIT_Y + 1))
        out.paste(tall.crop((0, 16, 16, 32)), (col * CELL + 1, KIT_Y + CELL + 1))
    for i, (name, fn) in enumerate(SINGLES):
        col, row = single_slot(i)
        out.paste(paint(fn, 16, name not in BARE), (col * CELL + 1, KIT_Y + row * CELL + 1))
    return out


def preview(atlas, path, zoom=4):
    grass = atlas.crop((217, 1, 233, 17))
    n = len(TREES) + len(SINGLES)
    cols = 12
    rows = (n + cols - 1) // cols
    w, h = cols * 20, rows * 46
    img = Image.new("RGBA", (w, h), (40, 40, 60, 255))
    draw = ImageDraw.Draw(img)
    items = [(name, paint(fn, 32), 32) for name, fn in TREES] + [(name, paint(fn, 16, name not in BARE), 16) for name, fn in SINGLES]
    for i, (name, spr, sh) in enumerate(items):
        x, y = (i % cols) * 20 + 2, (i // cols) * 46 + 10
        for gy in range(0, 32, 16):
            img.paste(grass, (x, y + gy))
        img.alpha_composite(spr, (x, y + 32 - sh))
    big = img.resize((w * zoom, h * zoom), Image.NEAREST)
    d = ImageDraw.Draw(big)
    for i, (name, _, _) in enumerate(items):
        d.text(((i % cols) * 20 * zoom + 6, (i // cols) * 46 * zoom + 4), name, fill=(255, 230, 120, 255))
    big.save(path)


if __name__ == "__main__":
    root = Path(__file__).resolve().parent.parent / "res" / "atlas.png"
    atlas = Image.open(root).convert("RGBA")
    if len(sys.argv) > 2 and sys.argv[1] == "--preview":
        preview(atlas, sys.argv[2])
        print(f"wrote {sys.argv[2]}")
    else:
        add_sprites(atlas).save(root)
        print(f"wrote {root}")
