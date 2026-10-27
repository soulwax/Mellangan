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
OUTLINE_DARK = (63, 38, 49)
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

ORANGE = ((176, 84, 36), (220, 128, 52), (244, 176, 88))
SPRUCE = ((40, 84, 104), (62, 116, 134), (100, 154, 168))
REDWOOD = ((110, 48, 36), (150, 70, 48), (186, 102, 70))
APPLE = (214, 48, 52)
BLUE = ((54, 76, 168), (84, 116, 214), (150, 176, 244))
PINK = ((180, 70, 120), (220, 110, 160), (248, 170, 204))
SAGE = ((104, 128, 112), (142, 166, 146), (186, 204, 186))
PUMPKIN = ((190, 90, 30), (230, 130, 44), (250, 176, 80))
TEAL = ((30, 120, 130), (60, 180, 180), (150, 240, 230))
BONE = ((170, 160, 140), (214, 206, 186), (244, 240, 226))
TAN = ((150, 104, 70), (190, 144, 100), (224, 190, 146))
AGAVE = ((48, 112, 104), (80, 150, 132), (130, 190, 164))


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


def fir(c, snowy, pal=DARK):
    c.rect(7, 27, 9, 31, BARK)
    for top, bottom, half in ((2, 12, 4.5), (8, 19, 6), (14, 27, 7.5)):
        c.tri(8, top, bottom, half, pal)
        if snowy:
            for x in range(int(8 - half * 0.8), int(8 + half * 0.8) + 1, 2):
                c.put(x, bottom - 1, FROST[2])
            c.put(8, top, FROST[2])


def fir_pal(c, pal):
    fir(c, False, pal)


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


def apple_tree(c):
    c.rect(7, 18, 9, 31, BARK)
    c.ellipse(8, 11, 7.2, 7.5, LEAF, dither=0.15, seed=30)
    for x, y in ((4, 9), (8, 6), (11, 10), (6, 13), (12, 14), (9, 12)):
        c.put(x, y, APPLE)


def dead_oak(c):
    c.rect(7, 16, 9, 31, ((80, 66, 58), (108, 92, 80), (136, 120, 106)))
    for x0, y0, x1, y1 in ((8, 18, 2, 9), (8, 17, 14, 8), (8, 16, 6, 4), (9, 16, 11, 3), (4, 12, 2, 6), (12, 11, 15, 6)):
        c.line(x0, y0, x1, y1, (108, 92, 80))


def weeping_willow(c):
    c.rect(7, 16, 9, 31, BARK)
    c.ellipse(8, 7, 7.5, 5.5, OLIVE, seed=31)
    for x in range(2, 15, 2):
        length = 10 + (x * 7) % 9
        for y in range(9, 9 + length):
            c.put(x, y, OLIVE[(y + x) % 2])


def bamboo(c):
    for x, top in ((4, 4), (8, 1), (12, 6)):
        for y in range(top, 32):
            c.put(x, y, LIME[0] if y % 6 == 0 else LIME[1]), c.put(x + 1, y, LIME[2] if y % 6 else LIME[0])
        c.line(x + 1, top + 4, x + 4, top + 1, LIME[2])
        c.line(x, top + 8, x - 3, top + 5, LIME[1])


def redwood(c):
    for y in range(8, 32):
        half = 2 + (y - 8) * 0.06
        for x in range(int(8 - half), int(8 + half) + 1):
            c.put(x, y, REDWOOD[2] if x < 7 else REDWOOD[0] if x > 9 else REDWOOD[1])
    c.ellipse(8, 7, 4.5, 7, DARK, seed=32)
    c.ellipse(4.5, 15, 2.2, 1.6, DARK, seed=33)
    c.ellipse(11.5, 13, 2.2, 1.6, DARK, seed=34)


def tree_fern(c):
    c.line(8, 31, 8, 12, BARK[0], 2)
    for deg, r in ((185, 8), (215, 8), (245, 8), (270, 7), (295, 8), (325, 8), (355, 8)):
        dx, dy = angle(deg, r)
        for t in range(1, 11):
            k = t / 10
            c.put(8 + dx * k, 11 + dy * k + 4 * k * k, LIME[2] if k < 0.5 else LIME[1])
            if 0.2 < k < 0.9:
                c.put(8 + dx * k, 12 + dy * k + 4 * k * k, LIME[0])


def joshua(c):
    c.line(8, 31, 8, 16, TAN[1], 2)
    for x0, y0, x1, y1 in ((8, 18, 3, 10), (9, 17, 13, 9), (8, 16, 7, 6)):
        c.line(x0, y0, x1, y1, TAN[1], 2)
    for x, y in ((3, 9), (13, 8), (7, 5)):
        for deg in range(0, 360, 40):
            dx, dy = angle(deg, 2.5)
            c.line(x, y, x + dx, y + dy, AGAVE[1 if deg < 180 else 2])
        c.put(x, y, AGAVE[0])


def dotted_bush(c, berries, pal=LEAF, seed=40):
    c.ellipse(8, 11, 6.5, 4.6, pal, dither=0.12, seed=seed)
    for x, y in ((4, 9), (7, 8), (10, 9), (12, 11), (6, 12), (9, 11), (5, 11)):
        c.put(x, y, berries)


def hydrangea(c):
    c.ellipse(8, 12, 6.5, 3.6, LEAF, seed=41)
    for x, y, pal in ((5, 9, BLUE), (10, 8, PURPLE), (8, 11, BLUE), (12, 11, PURPLE)):
        c.ellipse(x, y, 2.4, 2.1, pal, seed=x + y)


def holly(c):
    c.ellipse(8, 11, 6.5, 4.6, DARK, dither=0.25, seed=42)
    for x, y in ((5, 8), (9, 9), (11, 12), (6, 12)):
        c.put(x, y, APPLE), c.put(x + 1, y, APPLE)


def pumpkins(c):
    c.curve(2, 11, 12, 1, 1, LEAF[0])
    c.ellipse(5, 12, 3.6, 2.8, PUMPKIN, seed=43)
    c.ellipse(11, 13, 3, 2.3, PUMPKIN, seed=44)
    c.put(5, 8, LEAF[0]), c.put(11, 10, LEAF[0])


def brown_mushrooms(c):
    for x, y, r in ((5, 10, 3), (10, 8, 3.2), (12, 12, 2)):
        c.rect(x - 1 if r > 2.5 else x, y + 1, x if r > 2.5 else x, 15, PALE)
        c.ellipse(x, y, r, r * 0.6, TAN, seed=x)


def stump(c):
    c.rect(4, 9, 11, 15, BARK)
    c.ellipse(7.5, 9, 4.5, 1.8, TAN, seed=45)
    c.put(7, 9, BARK[0]), c.put(9, 9, BARK[0])


def mossy_log(c):
    c.ellipse(8, 12, 7.2, 2.8, BARK, seed=46)
    c.ellipse(8, 10.5, 6, 1.4, MOSS, seed=47)
    c.ellipse(13.5, 12, 1.6, 2.2, TAN, seed=48)


def meadow_flowers(c, petals, center):
    for x, y in ((3, 10), (6, 7), (9, 9), (12, 6), (13, 11), (7, 12)):
        c.line(x, 15, x, y + 1, LEAF[0])
        c.put(x, y, petals), c.put(x - 1, y, petals), c.put(x + 1, y, petals), c.put(x, y - 1, petals)
        c.put(x, y, center)


def bluebells(c):
    for x0, h in ((4, 9), (8, 11), (12, 8)):
        c.curve(x0, 15, 2, -h, 3, LEAF[0])
        for k in range(3):
            c.put(x0 + 1 + k, 15 - h + 2 + k * 2, BLUE[1 + k % 2])


def bracken(c):
    for deg, r in ((200, 8), (230, 10), (260, 11), (290, 11), (320, 10), (345, 8)):
        dx, dy = angle(deg, r)
        for t in range(1, 11):
            k = t / 10
            c.put(8 + dx * k, 15 + dy * k + 3 * k * k, ORANGE[2] if k < 0.5 else ORANGE[1])


def clover(c):
    c.ellipse(8, 13, 6.5, 2.5, LEAF, dither=0.3, seed=49)
    for x, y in ((4, 11), (9, 10), (12, 12)):
        c.ellipse(x, y, 1.3, 1.3, PINK, seed=x)


def menhir(c):
    for y in range(1, 16):
        half = 2.6 - abs(y - 9) * 0.05
        for x in range(int(8 - half), int(8 + half) + 1):
            c.put(x, y, STONE[2] if x < 7 else STONE[0] if x > 9 else STONE[1])
    for x, y in ((6, 4), (8, 7), (7, 10)):
        c.put(x, y, MOSS[1])


def rock_pile(c):
    c.ellipse(5, 12.5, 3.6, 2.6, STONE, seed=50)
    c.ellipse(11, 12.5, 3.6, 2.6, STONE, seed=51)
    c.ellipse(8, 9, 3.2, 2.4, STONE, seed=52)


def amethyst(c):
    for x, top, half in ((5, 8, 1.6), (8, 4, 2.2), (11, 7, 1.8)):
        for y in range(top, 15):
            w = min(half, (y - top) * 0.7 + 0.5)
            for xx in range(int(x - w), int(x + w) + 1):
                c.put(xx, y, CRYSTAL[2] if xx < x else CRYSTAL[0] if xx > x else CRYSTAL[1])


def skull(c):
    c.ellipse(7, 10, 4, 3.4, BONE, seed=53)
    c.rect(5, 12, 9, 14, BONE)
    c.put(5, 10, OUTLINE_DARK), c.put(8, 10, OUTLINE_DARK), c.put(6, 13, OUTLINE_DARK)
    c.line(10, 14, 15, 11, BONE[2])


def driftwood(c):
    c.line(1, 13, 15, 11, PALE[1], 2)
    c.line(6, 12, 9, 8, PALE[1])
    c.line(11, 11, 13, 7, PALE[0])


def prickly_pear(c):
    c.ellipse(8, 12, 3, 3.2, LEAF, seed=54)
    c.ellipse(5, 7.5, 2.4, 2.6, LEAF, seed=55)
    c.ellipse(11, 7, 2.4, 2.8, LEAF, seed=56)
    for x, y in ((5, 4), (11, 4), (12, 9)):
        c.put(x, y, PINK[2])


def agave(c):
    for deg in (190, 215, 240, 265, 290, 315, 345):
        dx, dy = angle(deg, 8)
        c.line(8, 15, 8 + dx, 15 + dy, AGAVE[1 if deg < 265 else 0])
        c.line(8, 15, 8 + dx * 0.6, 15 + dy * 0.6, AGAVE[2])


def tumbleweed(c):
    for k in range(9):
        a0, a1 = k * 40, k * 40 + 150
        x0, y0 = angle(a0, 5)
        x1, y1 = angle(a1, 5)
        c.line(8 + x0, 10 + y0 * 0.9, 8 + x1, 10 + y1 * 0.9, TAN[k % 3])


def snowdrops(c):
    for x0, h in ((4, 8), (8, 10), (12, 7)):
        c.curve(x0, 15, 2, -h, 2, LEAF[0])
        c.put(x0 + 2, 15 - h + 2, FROST[2]), c.put(x0 + 2, 15 - h + 3, FROST[1])


def frozen_shrub(c):
    c.ellipse(8, 11, 6.5, 4.6, FROST, dither=0.2, seed=57)
    for x in (4, 7, 10, 12):
        c.line(x, 14, x, 16, ICE[1])


def glowshrooms(c):
    for x, y, r in ((5, 11, 2.4), (10, 9, 2.8), (12, 13, 1.8)):
        c.rect(x, y + 1, x, 15, PALE)
        c.ellipse(x, y, r, r * 0.6, TEAL, seed=x)


def ivy(c):
    c.ellipse(8, 11, 6.5, 4.6, DARK, dither=0.3, seed=58)
    for x0 in (3, 8, 13):
        c.line(x0, 9, x0 + 1, 15, LEAF[2])


def calla(c):
    for x, h in ((4, 8), (8, 10), (12, 7)):
        c.line(x, 15, x, 15 - h + 2, LEAF[0])
        c.ellipse(x, 15 - h + 1, 1.5, 1.8, FROST, seed=x)
        c.put(x, 15 - h, GOLD[2])


def strawberries(c):
    c.ellipse(8, 13, 6.5, 2.6, LEAF, dither=0.25, seed=59)
    for x, y in ((4, 12), (8, 11), (12, 13), (6, 14)):
        c.put(x, y, APPLE), c.put(x, y + 1, (160, 30, 40))


def pampas(c):
    for x, h, lean in ((4, 10, -2), (8, 13, 0), (12, 11, 2)):
        c.line(x, 15, x + lean * 0.5, 15 - h + 4, OLIVE[0])
        c.ellipse(x + lean * 0.6, 15 - h + 2, 1.4, 3, BONE, seed=x)


def stone_ring(c):
    for deg in range(0, 360, 45):
        dx, dy = angle(deg, 5.2)
        c.ellipse(8 + dx, 11 + dy * 0.55, 1.6, 1.2, STONE, seed=deg)


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


TREES2 = [
    ("apple tree", apple_tree),
    ("autumn birch", lambda c: birch(c, ORANGE)),
    ("dead oak", dead_oak),
    ("weeping willow", weeping_willow),
    ("bamboo", bamboo),
    ("redwood", redwood),
    ("tree fern", tree_fern),
    ("blue spruce", lambda c: fir_pal(c, SPRUCE)),
    ("joshua tree", joshua),
]

SINGLES2 = [
    ("roses", lambda c: dotted_bush(c, APPLE, DARK, 60)),
    ("blueberries", lambda c: dotted_bush(c, BLUE[1], LEAF, 61)),
    ("hydrangea", hydrangea),
    ("holly", holly),
    ("pumpkins", pumpkins),
    ("brown mushrooms", brown_mushrooms),
    ("stump", stump),
    ("mossy log", mossy_log),
    ("poppies", lambda c: meadow_flowers(c, APPLE, OUTLINE_DARK)),
    ("cornflowers", lambda c: meadow_flowers(c, BLUE[1], BLUE[0])),
    ("bluebells", bluebells),
    ("bracken", bracken),
    ("clover", clover),
    ("menhir", menhir),
    ("rock pile", rock_pile),
    ("amethyst", amethyst),
    ("skull", skull),
    ("driftwood", driftwood),
    ("prickly pear", prickly_pear),
    ("agave", agave),
    ("tumbleweed", tumbleweed),
    ("snowdrops", snowdrops),
    ("frozen shrub", frozen_shrub),
    ("glowshrooms", glowshrooms),
    ("ivy", ivy),
    ("sage", lambda c: dotted_bush(c, SAGE[2], SAGE, 62)),
    ("calla lilies", calla),
    ("strawberries", strawberries),
    ("pampas", pampas),
    ("stone ring", stone_ring),
]

BANDS = [(TREES, SINGLES), (TREES2, SINGLES2)]


def layout():
    slots = []
    for band, (trees, singles) in enumerate(BANDS):
        base = band * 2 * COLS
        for col, (name, fn) in enumerate(trees):
            slots.append((name, fn, True, base + col, base + COLS + col))
        free = [base + row * COLS + col for row in (0, 1) for col in range(len(trees), COLS)]
        assert len(singles) <= len(free), f"band {band} overflows"
        for k, (name, fn) in enumerate(singles):
            slots.append((name, fn, False, free[k], -1))
    return slots


BARE = {"sand tuft", "frost tuft", "ash tuft", "bog tuft", "bloom tuft"}


def paint(fn, h, outline=True):
    c = Canvas(16, h)
    fn(c)
    return c.image(outline)


def cell_pos(index):
    return (index % COLS) * CELL + 1, KIT_Y + (index // COLS) * CELL + 1


def add_sprites(atlas):
    rows = 2 * len(BANDS)
    out = Image.new("RGBA", (atlas.width, max(atlas.height, KIT_Y + rows * CELL)), (0, 0, 0, 0))
    out.paste(atlas.crop((0, 0, atlas.width, min(atlas.height, KIT_Y))), (0, 0))
    for name, fn, tall, top, foot in layout():
        if tall:
            img = paint(fn, 32)
            out.paste(img.crop((0, 0, 16, 16)), cell_pos(top))
            out.paste(img.crop((0, 16, 16, 32)), cell_pos(foot))
        else:
            out.paste(paint(fn, 16, name not in BARE), cell_pos(top))
    return out


def constants():
    def ident(n):
        return ''.join(w.capitalize() for w in n.split())
    lines = []
    for name, _, tall, top, foot in layout():
        lines.append(f"constexpr int {ident(name)}High = {top}, {ident(name)}Low = {foot};" if tall else f"constexpr int {ident(name)} = {top};")
    return "\n".join(lines)


def preview(atlas, path, zoom=4):
    grass = atlas.crop((217, 1, 233, 17))
    slots = layout()
    n = len(slots)
    cols = 12
    rows = (n + cols - 1) // cols
    w, h = cols * 20, rows * 46
    img = Image.new("RGBA", (w, h), (40, 40, 60, 255))
    draw = ImageDraw.Draw(img)
    items = [(name, paint(fn, 32), 32) if tall else (name, paint(fn, 16, name not in BARE), 16) for name, fn, tall, _, _ in slots]
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
    if len(sys.argv) > 1 and sys.argv[1] == "--constants":
        print(constants())
    elif len(sys.argv) > 2 and sys.argv[1] == "--preview":
        preview(atlas, sys.argv[2])
        print(f"wrote {sys.argv[2]}")
    else:
        add_sprites(atlas).save(root)
        print(f"wrote {root}")
