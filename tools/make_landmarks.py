"""Paints the four landmark sprites into res/atlas.png.

Each landmark is a 48 x 64 painting cut into 3 columns by 4 rows of 16 x 16 cells, so the game can stand it up as
three stacked props (Prop parts at yo 0, -16, -32, -48), the same way cottages are built. Band at y=1236, 24 columns:
landmark L owns cells L*12 .. L*12+11, row by row from the top (cell = L*12 + row*3 + col). Mirrors atlas::landmark.

    0  Ancient Tree   a vast oak with lantern fruit
    1  Fallen Star    a faceted star half sunk in a scorched crater
    2  Colossus       the skull and ribs of something enormous
    3  Monolith       a black obelisk carved with glowing runes
    4  Crystal Spire  a towering cluster of amethyst
    5  Great Mushroom a red-capped giant with a pale stem
    6  Shipwreck      a hull stranded on dry land, mast snapped
    7  Ancient Gate   a stone arch around a swirling portal
    8  Ember Forge    an anvil on glowing coals
    9  Frozen Giant   a figure sealed in a block of ice

Usage: python tools/make_landmarks.py [--preview out.png]   (make_assets.py calls add_landmarks too)
"""
import math
import sys
from pathlib import Path

from PIL import Image

from make_grounds import CELL
from sprite_kit import Canvas, _hash

BAND_Y = 1236
W, H = 48, 64

LEAF = ((40, 92, 50), (70, 140, 66), (122, 190, 88))
BARK = ((78, 52, 40), (114, 78, 54), (156, 110, 76))
GOLD = ((214, 150, 40), (250, 206, 92), (255, 246, 196))
ROCK = ((74, 66, 66), (110, 98, 94), (150, 138, 128))
STAR = ((214, 196, 120), (250, 238, 170), (255, 255, 236))
BONE = ((168, 154, 128), (214, 204, 178), (244, 238, 220))
SOCKET = (52, 40, 46)
SLATE = ((34, 38, 52), (56, 62, 82), (88, 96, 122))
RUNE = (110, 226, 255)


def tree():
    c = Canvas(W, H)
    for i, dx in enumerate((-9, -4, 4, 9)):
        c.line(24 + dx * 0.4, 56, 24 + dx * 1.4, 63, BARK[i % 2], 2)
    c.rect(19, 30, 28, 62, BARK)
    c.material([(x, y) for x in range(20, 28) for y in range(32, 61)], BARK[0], BARK[2], 11, 0.25)
    for cx, cy, rx, ry in ((24, 20, 22, 15), (11, 26, 11, 9), (37, 26, 11, 9), (24, 9, 14, 8), (16, 13, 9, 7),
                           (33, 13, 9, 7)):
        c.ellipse(cx, cy, rx, ry, LEAF, dither=0.18, seed=cx * 7 + cy)
    for x, y in ((10, 27), (19, 31), (30, 30), (38, 24), (22, 19), (14, 15), (33, 11), (27, 6)):
        c.put(x, y, GOLD[2])
        c.put(x, y + 1, GOLD[1])
    return c.image()


def star():
    c = Canvas(W, H)
    c.ellipse(24, 57, 23, 7, ROCK, dither=0.3, seed=5)
    c.ellipse(24, 58, 17, 4, ((44, 36, 34), (64, 54, 50), (84, 72, 66)))
    points = []
    for k in range(10):
        r = 17 if k % 2 == 0 else 7
        a = math.radians(-90 + k * 36)
        points.append((24 + math.cos(a) * r, 40 + math.sin(a) * r * 1.05))
    for y in range(18, 60):
        for x in range(4, 44):
            inside = False
            for i in range(10):
                (x0, y0), (x1, y1) = points[i], points[(i + 1) % 10]
                if (y0 > y + 0.5) != (y1 > y + 0.5) and x + 0.5 < (x1 - x0) * (y + 0.5 - y0) / (y1 - y0) + x0:
                    inside = not inside
            if inside and y < 58:
                lit = (x - 24) * -0.6 + (y - 40) * -0.8
                c.put(x, y, STAR[2] if lit > 4 else STAR[0] if lit < -6 else STAR[1])
    for x, y in ((24, 27), (18, 37), (30, 37), (24, 46)):
        c.put(x, y, (255, 255, 255))
    for x, y, s in ((6, 50, 1), (41, 48, 2), (12, 44, 1), (36, 52, 1)):
        c.line(x, y, x + s, y - 3, STAR[1])
    return c.image()


def colossus():
    c = Canvas(W, H)
    c.ellipse(24, 60, 23, 4, ROCK, dither=0.3, seed=9)
    for i, x in enumerate((28, 33, 38, 43)):
        c.curve(x, 58, -2 - i, -20 + i * 3, 3, BONE[1 + (i % 2)], 2, 16)
    c.line(26, 56, 46, 56, BONE[0], 2)
    c.ellipse(16, 38, 14, 13, BONE, dither=0.1, seed=3)
    c.rect(7, 46, 24, 54, BONE)
    for x in range(8, 24, 3):
        c.put(x, 52, SOCKET)
        c.put(x, 53, SOCKET)
    c.ellipse(10, 38, 4, 4, ((30, 22, 28), SOCKET, SOCKET))
    c.ellipse(21, 38, 4, 4, ((30, 22, 28), SOCKET, SOCKET))
    c.put(16, 44, SOCKET)
    c.put(15, 45, SOCKET)
    c.put(17, 45, SOCKET)
    c.material([(x, y) for x in range(4, 28) for y in range(26, 50)], BONE[0], BONE[2], 21, 0.12)
    return c.image()


def monolith():
    c = Canvas(W, H)
    c.ellipse(24, 60, 16, 4, ROCK, dither=0.25, seed=13)
    c.tri(24, 2, 9, 7, SLATE)
    c.rect(17, 10, 30, 59, SLATE)
    c.material([(x, y) for x in range(18, 30) for y in range(10, 58)], SLATE[0], SLATE[2], 17, 0.2)
    for y in range(14, 56, 5):
        for x in (21, 24, 27):
            if _hash(x, y, 31) > 0.45:
                c.put(x, y, RUNE)
                c.put(x, y + 1, RUNE if _hash(y, x, 31) > 0.5 else SLATE[1])
    return c.image()


AMETHYST = ((108, 70, 160), (164, 120, 220), (226, 200, 255))
CAP = ((150, 34, 40), (206, 58, 54), (240, 120, 104))
STEM = ((190, 176, 150), (226, 214, 190), (248, 242, 226))
PLANK = ((92, 60, 40), (130, 88, 58), (170, 120, 80))
SAND = ((176, 150, 104), (210, 184, 132), (236, 214, 166))
SAIL = ((176, 168, 150), (214, 208, 190), (240, 236, 222))
STONE = ((92, 92, 104), (130, 130, 144), (172, 172, 186))
PORTAL = ((70, 40, 130), (130, 80, 220), (210, 170, 255))
IRON = ((40, 40, 48), (68, 68, 80), (104, 104, 120))
EMBER = ((170, 52, 20), (240, 120, 40), (255, 220, 120))
ICE = ((140, 186, 222), (186, 222, 244), (232, 246, 255))
SNOW = ((196, 210, 230), (226, 236, 248), (250, 252, 255))


def spire():
    c = Canvas(W, H)
    c.ellipse(24, 60, 20, 4, ROCK, dither=0.3, seed=41)
    for cx, top, half in ((24, 4, 6), (14, 22, 5), (34, 18, 5), (8, 38, 3), (40, 36, 4)):
        c.tri(cx, top, top + 8, half, AMETHYST)
        c.rect(cx - half + 1, top + 8, cx + half - 1, 60, AMETHYST)
        c.line(cx - half + 2, top + 9, cx - half + 2, 58, AMETHYST[2])
    for x, y in ((24, 14), (14, 30), (34, 26), (24, 40)):
        c.put(x, y, (255, 255, 255))
    return c.image()


def mushroom():
    c = Canvas(W, H)
    c.ellipse(24, 61, 14, 3, ((60, 80, 50), (80, 110, 64), (110, 140, 84)))
    c.ellipse(24, 22, 23, 17, CAP)
    for x in range(3, 46):
        for y in range(22, 40):
            c.px.pop((x, y), None)
    c.ellipse(24, 23, 21, 4, ((170, 150, 130), (200, 184, 160), (226, 214, 196)))
    c.rect(18, 26, 30, 60, STEM)
    c.material([(x, y) for x in range(19, 30) for y in range(28, 60)], STEM[0], STEM[2], 43, 0.2)
    for x, y, r in ((14, 12, 3), (28, 8, 2), (36, 15, 3), (22, 17, 2), (8, 19, 2)):
        c.ellipse(x, y, r, r - 0.4, ((226, 220, 210), (246, 242, 236), (255, 255, 255)))
    return c.image()


def wreck():
    c = Canvas(W, H)
    c.ellipse(24, 58, 23, 6, SAND, dither=0.25, seed=47)
    hull = [(x, y) for x in range(4, 46) for y in range(28, 60)
            if 30 + (x - 4) * 0.22 <= y <= 57 - ((x - 25) / 21) ** 2 * 13]
    for x, y in hull:
        deck = 30 + (x - 4) * 0.22
        c.put(x, y, PLANK[2] if y - deck < 1 else PLANK[1] if int(y - deck) % 4 else PLANK[0])
    c.material(hull, PLANK[0], PLANK[2], 49, 0.15)
    c.line(20, 33, 26, 6, PLANK[0], 2)
    c.line(26, 6, 33, 14, PLANK[1])
    for y in range(10, 28):
        for x in range(27, 27 + max(0, (y - 10) // 2 + 4)):
            if _hash(x, y, 53) > 0.15:
                c.put(x, y, SAIL[1] if x % 3 else SAIL[0])
    c.put(12, 45, (240, 200, 80))
    c.put(13, 45, (255, 230, 120))
    return c.image()


def gate():
    c = Canvas(W, H)
    c.ellipse(24, 61, 22, 3, ROCK, dither=0.3, seed=59)
    c.rect(4, 18, 12, 60, STONE)
    c.rect(36, 18, 44, 60, STONE)
    for x in range(2, 47):
        top = 10 + int(((x - 24) / 22) ** 2 * 8)
        for y in range(top, top + 7):
            c.put(x, y, STONE[2] if y == top else STONE[1] if (x + y) % 7 else STONE[0])
    c.ellipse(24, 40, 11, 20, PORTAL, dither=0.4, seed=61)
    c.ellipse(24, 40, 6, 12, (PORTAL[1], PORTAL[2], (240, 220, 255)))
    c.material([(x, y) for x in range(4, 13) for y in range(18, 60)], STONE[0], STONE[2], 63, 0.2)
    c.material([(x, y) for x in range(36, 45) for y in range(18, 60)], STONE[0], STONE[2], 67, 0.2)
    return c.image()


def forge():
    c = Canvas(W, H)
    c.ellipse(24, 58, 21, 5, ROCK, dither=0.3, seed=71)
    c.ellipse(24, 56, 15, 3, EMBER, dither=0.5, seed=73)
    c.rect(14, 44, 34, 53, STONE)
    c.rect(19, 36, 29, 44, IRON)
    c.rect(8, 30, 40, 36, IRON)
    c.tri(6, 30, 33, 3, IRON)
    c.rect(16, 27, 30, 29, EMBER)
    for x, y in ((12, 24), (22, 18), (32, 21), (26, 12), (16, 14)):
        c.put(x, y, EMBER[2] if _hash(x, y, 77) > 0.5 else EMBER[1])
    return c.image()


def frozen():
    c = Canvas(W, H)
    c.ellipse(24, 59, 22, 5, SNOW, dither=0.2, seed=79)
    c.rect(8, 10, 40, 58, ICE)
    for y in range(8, 11):
        for x in range(10 + (10 - y), 39 - (10 - y)):
            c.put(x, y, ICE[2])
    figure = ((24, 18, 4, 4), (24, 31, 6, 8), (17, 30, 2, 6), (31, 30, 2, 6), (21, 46, 2, 7), (27, 46, 2, 7))
    for cx, cy, rx, ry in figure:
        c.ellipse(cx, cy, rx, ry, ((70, 96, 130), (96, 124, 160), (120, 150, 186)))
    for x, y in ((11, 14), (12, 15), (36, 40), (35, 41), (14, 50)):
        c.put(x, y, (255, 255, 255))
    return c.image()


PAINTERS = (tree, star, colossus, monolith, spire, mushroom, wreck, gate, forge, frozen)


def slot(landmark, row, col):
    i = landmark * 12 + row * 3 + col
    return (i % 24) * CELL + 1, BAND_Y + (i // 24) * CELL + 1


def add_landmarks(atlas):
    rows = (len(PAINTERS) * 12 + 23) // 24
    out = Image.new("RGBA", (atlas.width, max(atlas.height, BAND_Y + rows * CELL)), (0, 0, 0, 0))
    out.paste(atlas.crop((0, 0, atlas.width, min(atlas.height, BAND_Y))), (0, 0))
    for landmark, paint in enumerate(PAINTERS):
        art = paint()
        for row in range(4):
            for col in range(3):
                out.paste(art.crop((col * 16, row * 16, col * 16 + 16, row * 16 + 16)), slot(landmark, row, col))
    return out


def preview(atlas, path, zoom=5):
    img = Image.new("RGBA", (len(PAINTERS) * 56, 72), (60, 110, 60, 255))
    for landmark in range(len(PAINTERS)):
        for row in range(4):
            for col in range(3):
                x, y = slot(landmark, row, col)
                img.alpha_composite(atlas.crop((x, y, x + 16, y + 16)), (landmark * 56 + 4 + col * 16, 4 + row * 16))
    img.resize((img.width * zoom, img.height * zoom), Image.Resampling.NEAREST).save(path)


if __name__ == "__main__":
    root = Path(__file__).resolve().parent.parent / "res" / "atlas.png"
    atlas = add_landmarks(Image.open(root).convert("RGBA"))
    if len(sys.argv) > 2 and sys.argv[1] == "--preview":
        preview(atlas, sys.argv[2])
        print(f"wrote {sys.argv[2]}")
    else:
        atlas.save(root)
        print(f"wrote {root}")
