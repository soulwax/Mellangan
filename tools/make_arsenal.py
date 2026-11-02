"""Paints the arsenal band: icons for recipe weapons and drafted skills.

Run after make_expansion.py (both make_expansion.py and make_assets.py chain it).
`--preview out.png` writes the painted icons at 8x for review.
"""

import sys
from pathlib import Path

from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))

from atlas_layout import ARSENAL, BANDS, extend, place_cell, validate
from sprite_kit import Canvas

BAND = next(b for b in BANDS if b.name == "arsenal")

EMBER = ((122, 40, 24), (222, 96, 40), (255, 196, 90))
ICE = ((52, 96, 150), (120, 182, 230), (214, 240, 255))
ARC = ((66, 40, 120), (140, 92, 214), (222, 186, 255))
STAR = ((150, 104, 40), (240, 200, 90), (255, 246, 200))
VOID = ((34, 20, 60), (88, 48, 150), (170, 120, 240))
RUNE = ((30, 80, 110), (80, 180, 220), (200, 250, 255))
GOLD = ((128, 82, 36), (204, 150, 52), (250, 220, 122))
CORE = ((40, 14, 12),) * 3
LEAF = ((40, 96, 44), (88, 170, 70), (180, 230, 120))
STEEL = ((84, 92, 112), (156, 166, 186), (226, 232, 244))
TOXIC = ((60, 110, 30), (130, 200, 60), (210, 250, 140))
DUSK = ((70, 24, 56), (150, 56, 110), (226, 140, 190))
WOOD = ((92, 52, 28), (156, 96, 52), (214, 158, 96))
IRON = ((40, 40, 52), (78, 78, 96), (140, 140, 160))
RAGE = ((120, 30, 24), (214, 70, 50), (255, 160, 120))
LINEN = ((150, 140, 120), (220, 212, 190), (252, 248, 236))
BONE = ((150, 120, 70), (214, 186, 130), (246, 232, 190))
MAG = ((60, 100, 150), (110, 180, 230), (210, 240, 255))
SEA = ((30, 90, 100), (70, 170, 180), (170, 236, 240))
HOLY = ((170, 140, 70), (250, 226, 150), (255, 252, 230))
BLOOD = ((100, 20, 36), (190, 50, 70), (250, 130, 140))


def inverted(c, cx, top, bottom, half, pal):
    for y in range(top, bottom + 1):
        w = half * (bottom - y + 1) / (bottom - top + 1)
        for x in range(int(cx - w), int(cx + w) + 1):
            rel = (x - cx) / max(w, 0.5)
            c.put(x, y, pal[2] if rel < -0.35 else pal[0] if rel > 0.45 else pal[1])


def ember_lance():
    c = Canvas()
    c.line(3, 12, 11, 4, EMBER[1], 2)
    c.line(4, 12, 11, 5, EMBER[2])
    c.tri(12, 2, 5, 2, EMBER)
    for x, y in ((2, 13), (3, 14), (2, 11)):
        c.put(x, y, EMBER[0])
    return c.image()


def shard(c, x, y, pal):
    c.put(x, y - 1, pal[2])
    c.put(x - 1, y, pal[1])
    c.put(x, y, pal[2])
    c.put(x + 1, y, pal[0])
    c.put(x, y + 1, pal[1])


def hoarfrost_fan():
    c = Canvas()
    c.ellipse(4, 12, 2, 2, ICE)
    for x, y in ((12, 4), (13, 9), (8, 3)):
        c.line(4, 12, x, y, ICE[0])
        shard(c, x, y, ICE)
    return c.image()


def arcane_orbit():
    c = Canvas()
    c.line(3, 4, 13, 12, ARC[0])
    c.ellipse(8, 8, 4, 4, ARC)
    c.ellipse(3, 4, 2, 2, ARC)
    c.ellipse(13, 12, 2, 2, ARC)
    c.put(7, 7, (250, 236, 255))
    return c.image()


def starfall():
    c = Canvas()
    c.line(2, 14, 8, 7, STAR[0], 2)
    c.line(3, 13, 8, 8, STAR[1])
    c.line(10, 2, 10, 10, STAR[1], 2)
    c.line(6, 6, 14, 6, STAR[1], 2)
    c.ellipse(10.5, 6.5, 2, 2, STAR)
    return c.image()


def void_rift():
    c = Canvas()
    c.ellipse(8, 8, 6, 4, VOID)
    c.ellipse(8, 8, 3, 2, CORE)
    c.line(8, 5, 8, 11, VOID[2])
    c.put(7, 8, VOID[2])
    c.put(9, 8, VOID[2])
    return c.image()


def seeker_runes():
    c = Canvas()
    pts = ((8, 2), (14, 8), (8, 14), (2, 8))
    for (x0, y0), (x1, y1) in zip(pts, pts[1:] + pts[:1]):
        c.line(x0, y0, x1, y1, RUNE[1], 2)
    c.line(8, 5, 8, 11, RUNE[2])
    c.line(6, 8, 10, 8, RUNE[0])
    return c.image()


def flame_ring():
    c = Canvas()
    c.ellipse(8, 10, 6, 4, EMBER)
    c.ellipse(8, 10, 3, 2, CORE)
    c.tri(8, 2, 6, 2, EMBER)
    c.tri(3, 4, 7, 1, EMBER)
    c.tri(13, 4, 7, 1, EMBER)
    return c.image()


def time_warp():
    c = Canvas()
    c.line(4, 2, 12, 2, GOLD[1])
    c.line(4, 14, 12, 14, GOLD[1])
    inverted(c, 8, 3, 7, 4, ICE)
    c.tri(8, 9, 13, 4, ICE)
    c.line(8, 7, 8, 9, ICE[2])
    c.line(4, 3, 4, 13, GOLD[0])
    c.line(12, 3, 12, 13, GOLD[0])
    return c.image()


def mana_surge():
    c = Canvas()
    c.line(10, 1, 5, 8, ARC[1], 2)
    c.line(5, 8, 11, 8, ARC[1], 2)
    c.line(11, 8, 6, 14, ARC[1], 2)
    c.line(9, 2, 6, 7, ARC[2])
    return c.image()


def arcane_missiles():
    c = Canvas()
    for x0, y0, x1, y1 in ((2, 9, 9, 3), (3, 13, 12, 6), (7, 14, 13, 10)):
        c.line(x0, y0, x1, y1, ARC[0])
        shard(c, x1, y1, ARC)
    c.line(2, 9, 7, 14, ARC[0])
    return c.image()


def singularity():
    c = Canvas()
    c.ellipse(8, 8, 6, 6, VOID)
    c.ellipse(8, 8, 4, 4, ((60, 30, 100), (120, 70, 200), (190, 150, 250)))
    c.ellipse(8, 8, 2, 2, CORE)
    c.put(11, 4, VOID[2])
    c.put(4, 11, VOID[2])
    return c.image()


def arrow_storm():
    c = Canvas()
    for x, y in ((8, 1), (14, 4), (15, 9), (12, 14), (4, 14), (1, 9), (2, 4)):
        c.line(8, 8, x, y, WOOD[1])
        c.put(x, y, STEEL[2])
    c.ellipse(8, 8, 2, 2, LEAF)
    return c.image()


def shield_charge():
    c = Canvas()
    for y in (5, 8, 11):
        c.line(1, y, 7, y, STEEL[1])
    c.ellipse(10, 8, 4, 6, STEEL)
    c.line(10, 4, 10, 12, GOLD[1])
    c.line(7, 8, 13, 8, GOLD[1])
    return c.image()


def miasma():
    c = Canvas()
    c.ellipse(6, 10, 4, 3, TOXIC)
    c.ellipse(10, 7, 4, 4, TOXIC)
    c.ellipse(5, 5, 2, 2, TOXIC)
    c.put(9, 6, CORE[0])
    c.put(11, 6, CORE[0])
    return c.image()


def chain_storm():
    c = Canvas()
    nodes = ((2, 12), (7, 4), (10, 11), (14, 3))
    for (x0, y0), (x1, y1) in zip(nodes, nodes[1:]):
        c.line(x0, y0, x1, y1, RUNE[1], 2)
    for x, y in nodes:
        c.ellipse(x, y, 1, 1, RUNE)
    return c.image()


def death_grip():
    c = Canvas()
    for x, y in ((2, 2), (14, 2), (2, 14), (14, 14), (8, 1)):
        c.line(x, y, 8, 8, DUSK[1])
    c.ellipse(8, 8, 3, 3, DUSK)
    c.put(8, 8, CORE[0])
    return c.image()


def sacred_light():
    c = Canvas()
    c.ellipse(8, 13, 6, 2, GOLD)
    c.rect(6, 1, 10, 12, STAR)
    c.line(8, 1, 8, 12, (255, 252, 230))
    return c.image()


def lunge():
    c = Canvas()
    c.line(2, 14, 11, 5, WOOD[1], 2)
    c.line(10, 6, 14, 2, STEEL[2], 2)
    c.line(9, 4, 12, 7, STEEL[0])
    for x, y in ((1, 11), (4, 15)):
        c.line(x, y, 3, 13, STEEL[1])
    return c.image()


def brambles():
    c = Canvas()
    c.curve(1, 13, 14, -9, 4, LEAF[1], 2)
    c.curve(2, 4, 12, 9, -3, LEAF[0], 2)
    for x, y in ((4, 9), (10, 6), (7, 12), (12, 11)):
        c.put(x, y, LEAF[2])
    return c.image()


def minefield():
    c = Canvas()
    c.ellipse(7, 10, 5, 4, IRON)
    c.line(10, 6, 13, 3, WOOD[1])
    c.ellipse(13.5, 2.5, 1, 1, EMBER)
    c.put(5, 8, IRON[2])
    return c.image()


def comet():
    c = Canvas()
    c.line(2, 2, 10, 10, VOID[1], 3)
    c.line(2, 6, 9, 11, ARC[1])
    c.ellipse(11, 11, 3, 3, ARC)
    c.put(10, 10, (250, 236, 255))
    return c.image()


def powder_keg():
    c = Canvas()
    c.ellipse(8, 9, 5, 5, WOOD)
    c.line(3, 7, 13, 7, IRON[1])
    c.line(3, 11, 13, 11, IRON[1])
    c.line(8, 4, 11, 1, WOOD[0])
    c.put(12, 1, EMBER[2])
    return c.image()


def war_cry():
    c = Canvas()
    c.curve(3, 12, 8, -8, 3, BONE[1], 3)
    c.ellipse(12, 5, 2, 3, BONE)
    for x, y in ((14, 9), (15, 12), (11, 1)):
        c.line(12, 5, x, y, RAGE[1])
    return c.image()


def bandage():
    c = Canvas()
    c.ellipse(8, 8, 6, 5, LINEN)
    c.rect(7, 4, 9, 12, RAGE)
    c.rect(4, 7, 12, 9, RAGE)
    return c.image()


def tumble():
    c = Canvas()
    c.curve(2, 12, 11, -8, -4, MAG[1], 2)
    c.tri(13, 2, 7, 3, MAG)
    c.ellipse(4, 12, 2, 2, LINEN)
    return c.image()


def whirlwind():
    c = Canvas()
    c.ellipse(8, 8, 6, 6, STEEL)
    c.ellipse(8, 8, 4, 4, ((60, 66, 84), (110, 118, 140), (190, 198, 216)))
    c.ellipse(9, 7, 2, 2, STEEL)
    c.line(2, 4, 6, 2, STEEL[2])
    c.line(14, 12, 10, 14, STEEL[2])
    return c.image()


def ground_slam():
    c = Canvas()
    c.rect(2, 11, 14, 14, WOOD)
    c.line(8, 11, 6, 7, CORE[0])
    c.line(8, 11, 11, 6, CORE[0])
    c.line(6, 7, 4, 4, WOOD[1])
    c.line(11, 6, 13, 3, WOOD[1])
    return c.image()


def fire_ring():
    c = Canvas()
    c.ellipse(8, 9, 6, 5, EMBER)
    c.ellipse(8, 9, 3, 2, IRON)
    for x in (3, 8, 13):
        c.tri(x, 1 if x == 8 else 3, 6, 1, EMBER)
    return c.image()


def lodestar():
    c = Canvas()
    c.rect(3, 3, 5, 12, RAGE)
    c.rect(11, 3, 13, 12, RAGE)
    c.rect(3, 11, 13, 14, RAGE)
    c.rect(3, 2, 5, 4, STEEL)
    c.rect(11, 2, 13, 4, STEEL)
    return c.image()


def snare_net():
    c = Canvas()
    for i in (2, 6, 10, 14):
        c.line(i, 2, i, 14, BONE[1])
        c.line(2, i, 14, i, BONE[1])
    c.ellipse(14, 14, 1, 1, IRON)
    return c.image()


def hollow(c, cx, cy, rx, ry):
    for x, y in list(c.px):
        if ((x - cx) / rx) ** 2 + ((y - cy) / ry) ** 2 < 1:
            del c.px[(x, y)]


def lionheart_cleave():
    c = Canvas()
    c.line(4, 12, 13, 3, STEEL[1], 2)
    c.line(5, 12, 13, 4, STEEL[2])
    c.line(3, 9, 7, 13, GOLD[1], 2)
    c.line(2, 14, 5, 11, WOOD[1])
    c.ellipse(2, 14, 1, 1, GOLD)
    return c.image()


def bastion_shields():
    c = Canvas()
    c.ellipse(8, 8, 6, 5, IRON)
    hollow(c, 8, 8, 4, 3)
    for x, y in ((3, 4), (13, 12)):
        c.ellipse(x, y, 2, 3, STEEL)
        c.put(x, y, GOLD[1])
    return c.image()


def hawk_talon():
    c = Canvas()
    c.line(2, 4, 8, 11, WOOD[1], 2)
    c.line(8, 11, 14, 4, WOOD[1], 2)
    c.line(3, 4, 8, 10, LEAF[2])
    c.line(8, 10, 13, 4, LEAF[1])
    return c.image()


def bear_trap():
    c = Canvas()
    c.ellipse(8, 12, 6, 2, IRON)
    c.curve(2, 11, 12, 0, 7, STEEL[1], 2)
    for x in (4, 6, 8, 10, 12):
        c.line(x, 5, x, 8, STEEL[2])
    c.line(3, 8, 13, 8, STEEL[1])
    return c.image()


def flask(c, pal):
    c.rect(7, 2, 9, 5, LINEN)
    c.ellipse(8, 10, 5, 5, pal)
    c.put(6, 8, pal[2])


def caustic_flask():
    c = Canvas()
    flask(c, TOXIC)
    c.put(10, 12, TOXIC[2])
    c.put(9, 13, TOXIC[0])
    return c.image()


def witchfire_phial():
    c = Canvas()
    flask(c, DUSK)
    c.tri(8, 6, 12, 2, EMBER)
    c.put(8, 9, EMBER[2])
    return c.image()


def ball_lightning():
    c = Canvas()
    c.ellipse(8, 8, 5, 5, MAG)
    for x0, y0, x1, y1 in ((8, 1, 6, 5), (15, 8, 11, 7), (3, 13, 6, 11)):
        c.line(x0, y0, x1, y1, RUNE[2])
    c.put(7, 7, (240, 252, 255))
    return c.image()


def arc_lance():
    c = Canvas()
    c.line(1, 14, 14, 1, MAG[0], 3)
    c.line(1, 14, 14, 1, RUNE[2])
    for x, y in ((5, 6), (11, 10), (9, 3)):
        c.line(x, y, 8, 7, RUNE[1])
    return c.image()


def soul_harvest():
    c = Canvas()
    c.curve(2, 4, 12, 9, 5, DUSK[1], 2)
    c.curve(3, 4, 10, 8, 4, DUSK[2])
    c.line(13, 2, 13, 14, WOOD[1])
    c.ellipse(5, 12, 2, 2, BLOOD)
    return c.image()


def bone_wheel():
    c = Canvas()
    c.ellipse(8, 8, 6, 6, BONE)
    c.ellipse(8, 8, 2, 2, CORE)
    for x, y in ((8, 2), (14, 8), (8, 14), (2, 8)):
        c.line(8, 8, x, y, BONE[0])
    return c.image()


def lance_of_dawn():
    c = Canvas()
    c.ellipse(3, 13, 3, 2, HOLY)
    c.line(4, 12, 14, 2, HOLY[1], 3)
    c.line(4, 12, 14, 2, (255, 255, 240))
    c.put(14, 2, HOLY[2])
    return c.image()


def seraph_wings():
    c = Canvas()
    c.curve(8, 9, -7, -6, 3, HOLY[1], 2)
    c.curve(8, 9, 7, -6, 3, HOLY[1], 2)
    c.curve(8, 11, -5, -2, 2, LINEN[2])
    c.curve(8, 11, 5, -2, 2, LINEN[2])
    c.ellipse(8, 9, 1, 2, HOLY)
    return c.image()


def crescent_cut():
    c = Canvas()
    c.curve(2, 3, 11, 9, 4, (250, 200, 225), 2)
    c.curve(5, 2, 10, 11, 4, STEEL[2])
    c.put(13, 13, RAGE[1])
    return c.image()


def flicker_darts():
    c = Canvas()
    for x, y in ((12, 2), (14, 7), (8, 1)):
        c.line(2, 14, x, y, STEEL[1])
        c.put(x, y, STEEL[2])
    c.ellipse(3, 13, 2, 2, ((150, 80, 110), (230, 150, 190), (250, 210, 230)))
    return c.image()


def thornwhip():
    c = Canvas()
    c.curve(2, 14, 12, -11, -6, LEAF[1], 2)
    for x, y in ((5, 9), (9, 5), (12, 3), (4, 12)):
        c.put(x, y, LEAF[2])
        c.put(x + 1, y - 1, BONE[1])
    return c.image()


def root_snare():
    c = Canvas()
    c.ellipse(8, 12, 5, 2, WOOD)
    for x, d in ((4, -2), (8, 0), (12, 2)):
        c.curve(x, 12, d, -9, 2, LEAF[1])
    c.put(8, 3, LEAF[2])
    return c.image()


def clockwork_mine():
    c = Canvas()
    c.ellipse(8, 9, 6, 5, GOLD)
    c.ellipse(8, 9, 2, 2, IRON)
    for x, y in ((8, 3), (14, 9), (8, 15), (2, 9)):
        c.put(x, y, GOLD[0])
    c.put(8, 9, EMBER[2])
    return c.image()


def flash_grenade():
    c = Canvas()
    c.ellipse(7, 10, 4, 4, IRON)
    c.line(9, 6, 11, 3, IRON[2])
    for x, y in ((13, 1), (15, 4), (12, 5)):
        c.line(11, 3, x, y, STAR[2])
    return c.image()


def prophecy_ray():
    c = Canvas()
    c.ellipse(4, 8, 3, 2, ARC)
    c.put(4, 8, CORE[0])
    c.line(7, 7, 15, 3, ARC[2])
    c.line(7, 9, 15, 13, ARC[2])
    c.line(7, 8, 15, 8, (250, 236, 255))
    return c.image()


def fate_wheel():
    c = Canvas()
    c.ellipse(8, 8, 6, 6, VOID)
    c.ellipse(8, 8, 4, 4, ARC)
    for x, y in ((8, 2), (13, 11), (3, 11)):
        c.put(x, y, RUNE[2])
    c.line(8, 5, 8, 11, VOID[2])
    return c.image()


def chain_shot():
    c = Canvas()
    c.ellipse(4, 11, 3, 3, IRON)
    c.ellipse(12, 5, 3, 3, IRON)
    c.line(6, 9, 10, 7, STEEL[1])
    c.put(3, 10, STEEL[2])
    c.put(11, 4, STEEL[2])
    return c.image()


def riptide():
    c = Canvas()
    c.ellipse(8, 9, 6, 5, SEA)
    c.curve(3, 9, 9, -4, 3, SEA[2])
    c.curve(6, 10, 4, -2, 1, SEA[0])
    c.put(8, 9, SEA[0])
    return c.image()


def war_banner():
    c = Canvas()
    c.line(4, 1, 4, 15, WOOD[1], 2)
    c.rect(5, 2, 13, 8, RAGE)
    c.tri(9, 8, 11, 4, RAGE)
    c.ellipse(9, 5, 1, 1, GOLD)
    return c.image()


def hunters_mark():
    c = Canvas()
    c.ellipse(8, 8, 6, 6, LEAF)
    hollow(c, 8, 8, 4, 4)
    c.line(8, 1, 8, 15, RAGE[1])
    c.line(1, 8, 15, 8, RAGE[1])
    c.put(8, 8, RAGE[2])
    return c.image()


def flask_barrage():
    c = Canvas()
    for x, y in ((5, 10), (10, 11), (8, 6)):
        c.ellipse(x, y, 3, 3, EMBER)
    c.rect(7, 1, 9, 3, LINEN)
    c.put(4, 9, EMBER[2])
    return c.image()


def tempest():
    c = Canvas()
    c.ellipse(6, 4, 4, 3, IRON)
    c.ellipse(11, 4, 4, 3, IRON)
    c.line(9, 7, 6, 11, RUNE[2], 2)
    c.line(6, 11, 10, 11, RUNE[2])
    c.line(10, 11, 7, 15, RUNE[2])
    return c.image()


def blood_pact():
    c = Canvas()
    c.ellipse(8, 9, 5, 5, BLOOD)
    c.tri(8, 1, 6, 3, BLOOD)
    c.put(6, 8, BLOOD[2])
    c.line(3, 15, 13, 15, DUSK[1])
    return c.image()


def hallowed_ground():
    c = Canvas()
    c.ellipse(8, 12, 7, 3, HOLY)
    c.line(8, 1, 8, 10, HOLY[2], 2)
    c.line(5, 4, 11, 4, HOLY[2], 2)
    return c.image()


def flash_step():
    c = Canvas()
    for x in (3, 7, 11):
        c.line(x, 13, x + 3, 3, (240, 190, 220) if x < 11 else STEEL[2])
    c.ellipse(13, 3, 1, 1, STEEL)
    return c.image()


def overgrowth():
    c = Canvas()
    c.ellipse(5, 10, 4, 4, LEAF)
    c.ellipse(11, 9, 4, 5, LEAF)
    c.ellipse(8, 5, 3, 3, TOXIC)
    c.put(11, 7, TOXIC[2])
    return c.image()


def scrap_barrage():
    c = Canvas()
    c.ellipse(6, 10, 4, 4, IRON)
    c.rect(10, 3, 13, 6, STEEL)
    c.line(9, 7, 12, 10, GOLD[1])
    c.put(6, 6, EMBER[2])
    return c.image()


def eclipse():
    c = Canvas()
    c.ellipse(8, 8, 6, 6, GOLD)
    c.ellipse(10, 7, 5, 5, VOID)
    c.put(4, 4, ARC[2])
    return c.image()


def whirlpool():
    c = Canvas()
    c.ellipse(8, 8, 7, 6, SEA)
    c.ellipse(8, 8, 4, 3, ((20, 60, 70), (40, 120, 130), (120, 210, 220)))
    c.ellipse(8, 8, 1, 1, CORE)
    c.curve(2, 6, 7, -3, 2, SEA[2])
    return c.image()


def bloodletter_fan():
    c = Canvas()
    c.ellipse(4, 12, 2, 2, WOOD)
    for x, y in ((13, 3), (14, 8), (9, 2), (11, 12)):
        c.line(4, 12, x, y, BLOOD[0])
        c.put(x, y, BLOOD[2])
        c.put(x - 1, y + 1, BLOOD[1])
    c.put(7, 13, BLOOD[1])
    return c.image()


def hex_whip():
    c = Canvas()
    c.line(2, 14, 5, 11, WOOD[1], 2)
    c.curve(5, 11, 8, -8, 4, DUSK[1])
    c.curve(6, 10, 7, -7, 3, ARC[2])
    c.ellipse(13, 3, 1, 1, ARC)
    return c.image()


def bone_spear():
    c = Canvas()
    c.line(2, 14, 11, 5, BONE[1], 2)
    c.line(3, 14, 11, 6, BONE[2])
    c.tri(12, 1, 6, 2, LINEN)
    c.ellipse(4, 12, 1, 1, BONE)
    return c.image()


def thunder_drum():
    c = Canvas()
    c.rect(3, 7, 13, 13, WOOD)
    c.ellipse(8, 7, 5, 2, LINEN)
    c.line(4, 10, 12, 10, GOLD[1])
    c.line(9, 1, 7, 4, MAG[2])
    c.line(7, 4, 10, 4, MAG[2])
    c.line(10, 4, 8, 6, MAG[1])
    return c.image()


def frost_lantern():
    c = Canvas()
    c.line(8, 1, 8, 3, IRON[1])
    c.rect(5, 3, 11, 4, IRON)
    c.ellipse(8, 9, 4, 5, ICE)
    c.ellipse(8, 9, 2, 3, ((150, 210, 240), (214, 240, 255), (250, 254, 255)))
    c.rect(5, 13, 11, 14, IRON)
    return c.image()


def rot_bellows():
    c = Canvas()
    c.ellipse(6, 10, 5, 4, WOOD)
    c.line(2, 13, 4, 11, WOOD[0], 2)
    c.line(10, 8, 12, 6, IRON[1], 2)
    c.ellipse(13, 4, 2, 2, TOXIC)
    c.ellipse(10, 2, 1, 1, TOXIC)
    c.put(14, 7, TOXIC[1])
    return c.image()


def grave_bell():
    c = Canvas()
    c.line(8, 1, 8, 2, IRON[1])
    c.ellipse(8, 6, 4, 3, STEEL)
    c.rect(4, 6, 12, 10, STEEL)
    c.line(2, 12, 14, 12, STEEL[1], 2)
    c.line(5, 7, 5, 10, STEEL[2])
    c.ellipse(8, 14, 1, 1, IRON)
    return c.image()


def tidal_ring():
    c = Canvas()
    c.ellipse(8, 8, 6, 6, SEA)
    c.ellipse(8, 8, 3, 3, ((20, 60, 70), (40, 120, 130), (120, 210, 220)))
    c.curve(3, 4, 9, 2, -2, SEA[2])
    c.curve(5, 12, 8, -1, 2, ICE[2])
    return c.image()


def ember_wall():
    c = Canvas()
    c.rect(1, 9, 15, 14, IRON)
    c.line(1, 11, 15, 11, IRON[0])
    c.line(5, 9, 5, 11, IRON[0])
    c.line(11, 11, 11, 14, IRON[0])
    c.tri(4, 3, 8, 2, EMBER)
    c.tri(8, 1, 8, 3, EMBER)
    c.tri(12, 4, 8, 2, EMBER)
    return c.image()


def sky_lance():
    c = Canvas()
    c.ellipse(5, 13, 4, 2, STAR)
    c.line(14, 1, 7, 9, MAG[1], 2)
    c.line(14, 2, 8, 9, MAG[2])
    c.ellipse(6, 11, 2, 2, STEEL)
    c.put(5, 10, (255, 255, 255))
    return c.image()


def wolf_totem():
    c = Canvas()
    c.rect(6, 6, 10, 15, WOOD)
    c.tri(6, 1, 5, 2, STEEL)
    c.tri(10, 1, 5, 2, STEEL)
    c.ellipse(8, 6, 3, 2, STEEL)
    c.put(7, 6, MAG[2])
    c.put(9, 6, MAG[2])
    c.line(6, 10, 10, 10, BLOOD[1])
    return c.image()


def mirror_shards():
    c = Canvas()
    c.tri(5, 2, 9, 2, ICE)
    c.tri(11, 6, 13, 2, ICE)
    c.tri(6, 10, 14, 2, LINEN)
    c.put(5, 4, (255, 255, 255))
    c.put(11, 8, (255, 255, 255))
    return c.image()


def runic_wall():
    c = Canvas()
    c.rect(1, 8, 15, 14, VOID)
    c.line(1, 11, 15, 11, VOID[0])
    c.line(4, 9, 4, 13, ARC[2])
    c.line(8, 9, 8, 13, ICE[2])
    c.line(12, 9, 12, 13, ARC[2])
    c.ellipse(8, 4, 2, 2, ARC)
    c.line(8, 6, 8, 7, ARC[1])
    return c.image()


def hawk_companion():
    c = Canvas()
    c.ellipse(8, 8, 2, 3, WOOD)
    c.tri(4, 4, 9, 3, BONE)
    c.tri(12, 4, 9, 3, BONE)
    c.ellipse(8, 5, 2, 2, LINEN)
    c.put(9, 5, GOLD[1])
    c.line(7, 11, 9, 13, WOOD[1])
    return c.image()


def shield_wall():
    c = Canvas()
    for x in (4, 8, 12):
        c.ellipse(x, 9, 3, 4, STEEL)
        c.put(x, 9, GOLD[2])
    c.line(1, 14, 15, 14, IRON[1])
    return c.image()


def philosophers_spiral():
    c = Canvas()
    c.ellipse(8, 8, 6, 6, GOLD)
    c.ellipse(8, 8, 3, 3, TOXIC)
    c.ellipse(8, 8, 1, 1, EMBER)
    c.curve(3, 5, 9, 1, -2, GOLD[2])
    return c.image()


def storm_whip():
    c = Canvas()
    c.line(2, 14, 5, 11, IRON[1], 2)
    c.line(5, 11, 9, 7, MAG[1])
    c.line(9, 7, 8, 5, MAG[2])
    c.line(8, 5, 13, 2, MAG[2])
    c.line(9, 8, 12, 9, MAG[1])
    return c.image()


def ghoul_pack():
    c = Canvas()
    c.ellipse(8, 6, 4, 4, ((70, 110, 60), (120, 170, 100), (190, 230, 160)))
    c.put(6, 6, BLOOD[2])
    c.put(10, 6, BLOOD[2])
    c.line(6, 9, 10, 9, CORE[0])
    c.rect(5, 10, 11, 14, ((60, 50, 60), (100, 90, 100), (150, 140, 150)))
    return c.image()


def sun_spiral():
    c = Canvas()
    for x1, y1 in ((8, 1), (8, 15), (1, 8), (15, 8), (3, 3), (13, 13), (3, 13), (13, 3)):
        c.line(8, 8, x1, y1, HOLY[1])
    c.ellipse(8, 8, 4, 4, HOLY)
    c.put(7, 7, (255, 255, 255))
    return c.image()


def riposte_whip():
    c = Canvas()
    c.line(2, 14, 5, 11, STEEL[1], 2)
    c.line(3, 10, 6, 13, GOLD[1])
    c.curve(5, 11, 9, -8, -3, (240, 190, 220))
    c.put(14, 3, BLOOD[2])
    return c.image()


def bramble_wall():
    c = Canvas()
    c.ellipse(4, 11, 3, 3, LEAF)
    c.ellipse(9, 10, 4, 4, LEAF)
    c.ellipse(13, 11, 2, 3, LEAF)
    c.line(2, 9, 14, 7, WOOD[0])
    c.put(6, 7, TOXIC[2])
    c.put(11, 6, TOXIC[2])
    return c.image()


def clockwork_drone():
    c = Canvas()
    c.ellipse(8, 9, 4, 3, GOLD)
    c.ellipse(8, 9, 1, 1, ICE)
    c.line(2, 4, 14, 4, IRON[1])
    c.line(8, 4, 8, 6, IRON[0])
    c.line(5, 12, 4, 14, IRON[1])
    c.line(11, 12, 12, 14, IRON[1])
    return c.image()


def fate_spiral():
    c = Canvas()
    c.ellipse(8, 8, 6, 6, ARC)
    c.ellipse(8, 8, 3, 3, VOID)
    c.curve(3, 5, 9, 2, -2, ARC[2])
    c.put(8, 8, RUNE[2])
    return c.image()


def powder_crew():
    c = Canvas()
    c.ellipse(7, 10, 5, 4, IRON)
    c.line(10, 6, 12, 3, WOOD[1])
    c.ellipse(13, 2, 1, 1, EMBER)
    c.put(5, 8, STEEL[2])
    return c.image()


SMOKE_PAL = ((90, 90, 104), (150, 150, 164), (214, 214, 226))


def smoke_bomb():
    c = Canvas()
    c.ellipse(6, 10, 4, 4, SMOKE_PAL)
    c.ellipse(10, 7, 4, 4, SMOKE_PAL)
    c.ellipse(8, 11, 3, 3, IRON)
    c.line(9, 8, 11, 5, WOOD[1])
    c.put(12, 4, EMBER[2])
    return c.image()


def blood_rite():
    c = Canvas()
    c.tri(8, 2, 13, 5, BLOOD)
    c.ellipse(8, 11, 4, 3, BLOOD)
    c.put(7, 9, (255, 200, 200))
    c.line(3, 14, 13, 14, DUSK[1])
    return c.image()


def gravity_well():
    c = Canvas()
    c.ellipse(8, 8, 7, 6, VOID)
    c.ellipse(8, 8, 4, 3, ARC)
    c.ellipse(8, 8, 2, 1, CORE)
    c.curve(2, 8, 6, -5, -1, ARC[2])
    return c.image()


def mirror_image():
    c = Canvas()
    c.ellipse(5, 8, 3, 5, ICE)
    c.ellipse(11, 8, 3, 5, ((90, 110, 160), (150, 170, 220), (220, 230, 255)))
    c.line(8, 2, 8, 14, LINEN[2])
    return c.image()


def rallying_horn():
    c = Canvas()
    for x, y, r in ((3, 13, 1), (5, 11, 1), (7, 9, 2), (10, 7, 3), (12, 5, 3)):
        c.ellipse(x, y, r, r, BONE)
    c.ellipse(12, 5, 1, 1, WOOD)
    c.line(5, 11, 8, 8, GOLD[1])
    return c.image()


def shatter_wave():
    c = Canvas()
    c.ellipse(8, 8, 6, 6, ICE)
    c.ellipse(8, 8, 3, 3, ((150, 210, 240), (214, 240, 255), (255, 255, 255)))
    c.line(2, 2, 6, 6, ICE[2])
    c.line(14, 2, 10, 6, ICE[2])
    c.line(2, 14, 6, 10, ICE[2])
    c.line(14, 14, 10, 10, ICE[2])
    return c.image()


def second_sight():
    c = Canvas()
    c.ellipse(8, 8, 7, 4, LINEN)
    c.ellipse(8, 8, 3, 3, ARC)
    c.ellipse(8, 8, 1, 1, CORE)
    c.put(7, 7, (255, 255, 255))
    return c.image()


def sprint():
    c = Canvas()
    c.tri(11, 3, 13, 3, STEEL)
    c.line(2, 6, 7, 6, LINEN[1])
    c.line(1, 9, 8, 9, LINEN[1])
    c.line(2, 12, 7, 12, LINEN[1])
    c.line(7, 6, 9, 9, LINEN[0])
    c.line(7, 12, 9, 9, LINEN[0])
    return c.image()


def prismatic_prison():
    c = Canvas()
    for x in (3, 6, 9, 12):
        c.line(x, 3, x, 13, PRISM_PAL[1 if x % 2 else 2])
    c.line(2, 2, 13, 2, ARC[1])
    c.line(2, 14, 13, 14, ARC[1])
    return c.image()


PRISM_PAL = ((120, 90, 180), (200, 160, 255), (240, 220, 255))


def call_the_hawk():
    c = Canvas()
    c.tri(5, 3, 9, 4, BONE)
    c.tri(11, 3, 9, 4, BONE)
    c.ellipse(8, 9, 2, 3, WOOD)
    c.ellipse(8, 6, 2, 2, LINEN)
    c.put(9, 6, GOLD[1])
    c.line(5, 14, 11, 14, BLOOD[1])
    return c.image()


def last_stand():
    c = Canvas()
    c.ellipse(8, 8, 5, 6, STEEL)
    c.line(8, 3, 8, 13, GOLD[1])
    c.line(4, 7, 12, 7, GOLD[1])
    c.put(8, 7, (255, 255, 255))
    return c.image()


def transmutation_circle():
    c = Canvas()
    c.ellipse(8, 8, 7, 7, GOLD)
    c.ellipse(8, 8, 5, 5, ((60, 40, 20), (90, 60, 30), (120, 90, 50)))
    c.tri(8, 4, 11, 3, TOXIC)
    c.put(8, 12, GOLD[2])
    return c.image()


def eye_of_the_storm():
    c = Canvas()
    c.ellipse(8, 8, 7, 5, ((50, 60, 90), (90, 110, 150), (150, 170, 210)))
    c.ellipse(8, 8, 3, 2, MAG)
    c.ellipse(8, 8, 1, 1, CORE)
    c.line(12, 2, 10, 5, MAG[2])
    c.line(4, 14, 6, 11, MAG[2])
    return c.image()


def raise_dead():
    c = Canvas()
    c.ellipse(8, 6, 4, 4, BONE)
    c.put(6, 6, CORE[0])
    c.put(10, 6, CORE[0])
    c.line(6, 9, 10, 9, BONE[0])
    c.line(3, 14, 13, 14, ((60, 50, 40), (90, 70, 50), (120, 100, 70))[1], 2)
    c.line(8, 10, 8, 13, BONE[1])
    return c.image()


def halo_of_mercy():
    c = Canvas()
    c.ellipse(8, 5, 6, 2, HOLY)
    c.ellipse(8, 5, 4, 1, ((90, 80, 50), (120, 110, 70), (150, 140, 90)))
    c.tri(8, 8, 15, 4, LINEN)
    c.put(8, 10, HOLY[2])
    return c.image()


def perfect_parry():
    c = Canvas()
    c.line(2, 14, 13, 3, STEEL[1], 2)
    c.line(3, 3, 14, 14, STEEL[2])
    c.ellipse(8, 8, 2, 2, GOLD)
    return c.image()


def earthen_wall():
    c = Canvas()
    c.tri(4, 4, 14, 3, ((90, 70, 50), (150, 120, 85), (200, 175, 135)))
    c.tri(9, 2, 14, 3, ((90, 70, 50), (150, 120, 85), (200, 175, 135)))
    c.tri(13, 6, 14, 2, ((90, 70, 50), (150, 120, 85), (200, 175, 135)))
    c.line(1, 14, 15, 14, LEAF[0])
    return c.image()


def overclock():
    c = Canvas()
    c.ellipse(8, 8, 6, 6, GOLD)
    c.ellipse(8, 8, 3, 3, IRON)
    c.line(8, 8, 8, 4, LINEN[2])
    c.line(8, 8, 11, 9, LINEN[2])
    c.put(14, 3, EMBER[2])
    return c.image()


def foresight():
    c = Canvas()
    c.ellipse(8, 9, 6, 5, ARC)
    c.ellipse(8, 9, 3, 3, ((200, 190, 255), (230, 225, 255), (255, 255, 255)))
    c.ellipse(8, 9, 1, 1, VOID)
    c.line(8, 1, 8, 3, ARC[2])
    c.line(3, 3, 4, 4, ARC[2])
    c.line(13, 3, 12, 4, ARC[2])
    return c.image()


def cannonade():
    c = Canvas()
    c.rect(2, 9, 11, 12, IRON)
    c.ellipse(4, 13, 2, 2, WOOD)
    c.ellipse(10, 13, 2, 2, WOOD)
    c.ellipse(13, 5, 2, 2, IRON)
    c.line(11, 9, 12, 7, SMOKE_PAL[2])
    return c.image()


WEAPONS = (ember_lance, hoarfrost_fan, arcane_orbit, starfall, void_rift,
           seeker_runes, flame_ring, lionheart_cleave, bastion_shields,
           hawk_talon, bear_trap, caustic_flask, witchfire_phial,
           ball_lightning, arc_lance, soul_harvest, bone_wheel, lance_of_dawn,
           seraph_wings, crescent_cut, flicker_darts, thornwhip, root_snare,
           clockwork_mine, flash_grenade, prophecy_ray, fate_wheel, chain_shot,
           riptide, bloodletter_fan, hex_whip, bone_spear, thunder_drum,
           frost_lantern, rot_bellows, grave_bell, tidal_ring, ember_wall,
           sky_lance, wolf_totem, mirror_shards, runic_wall, hawk_companion,
           shield_wall, philosophers_spiral, storm_whip, ghoul_pack, sun_spiral,
           riposte_whip, bramble_wall, clockwork_drone, fate_spiral, powder_crew)
SKILLS = (time_warp, mana_surge, arcane_missiles, singularity, arrow_storm,
          shield_charge, miasma, chain_storm, death_grip, sacred_light, lunge,
          brambles, minefield, comet, powder_keg, war_cry, bandage, tumble,
          whirlwind, ground_slam, fire_ring, lodestar, snare_net, war_banner,
          hunters_mark, flask_barrage, tempest, blood_pact, hallowed_ground,
          flash_step, overgrowth, scrap_barrage, eclipse, whirlpool, smoke_bomb,
          blood_rite, gravity_well, mirror_image, rallying_horn, shatter_wave,
          second_sight, sprint, prismatic_prison, call_the_hawk, last_stand,
          transmutation_circle, eye_of_the_storm, raise_dead, halo_of_mercy,
          perfect_parry, earthen_wall, overclock, foresight, cannonade)


BADGE = ((44, 36, 56), (72, 60, 88), (108, 94, 128))


def badge(glyph):
    def paint():
        c = Canvas()
        c.ellipse(8, 8, 7, 7, BADGE)
        glyph(c)
        return c.image()
    paint.__name__ = glyph.__name__
    return paint


@badge
def bloodthirst(c):
    c.tri(8, 3, 12, 4, BLOOD)
    c.put(7, 8, (255, 200, 200))


@badge
def glass_cannon(c):
    c.rect(4, 7, 11, 10, ICE)
    c.ellipse(12, 8, 2, 2, ICE)
    c.line(5, 7, 9, 10, (255, 255, 255))


@badge
def momentum(c):
    c.line(3, 10, 12, 10, WIND_PAL[2])
    c.line(5, 7, 12, 7, WIND_PAL[1])
    c.tri(12, 5, 12, 2, GOLD)


WIND_PAL = ((150, 170, 190), (200, 215, 230), (240, 248, 255))


@badge
def cold_blood(c):
    c.tri(8, 3, 12, 4, ICE)
    c.put(8, 9, BLOOD[2])


@badge
def pyromaniac(c):
    c.tri(6, 4, 12, 3, EMBER)
    c.tri(10, 3, 12, 3, EMBER)


@badge
def scavenger(c):
    c.ellipse(6, 9, 2, 2, GOLD)
    c.ellipse(10, 7, 2, 2, ICE)
    c.ellipse(9, 11, 1, 1, GOLD)


@badge
def thick_skin(c):
    c.ellipse(8, 8, 4, 5, WOOD)
    c.line(6, 6, 10, 10, WOOD[0])


@badge
def vengeance(c):
    c.ellipse(8, 8, 4, 4, RAGE)
    c.ellipse(8, 8, 2, 2, CORE)
    c.put(8, 8, RAGE[2])


@badge
def night_owl(c):
    c.ellipse(6, 8, 2, 2, GOLD)
    c.ellipse(10, 8, 2, 2, GOLD)
    c.put(6, 8, CORE[0])
    c.put(10, 8, CORE[0])
    c.tri(8, 10, 12, 1, WOOD)


@badge
def hunters_instinct(c):
    c.line(3, 8, 13, 8, BLOOD[1])
    c.line(8, 3, 8, 13, BLOOD[1])
    c.ellipse(8, 8, 2, 2, STEEL)


@badge
def overflow(c):
    c.ellipse(8, 9, 4, 3, HOLY)
    c.tri(8, 3, 7, 2, HOLY)


@badge
def afterimage(c):
    c.ellipse(6, 8, 2, 4, ((80, 90, 130), (120, 130, 180), (170, 180, 230)))
    c.ellipse(10, 8, 2, 4, LINEN)


@badge
def spellweave(c):
    c.curve(3, 6, 10, 0, 3, ARC[2])
    c.curve(3, 10, 10, 0, -3, ICE[2])


@badge
def volley_master(c):
    for y in (5, 8, 11):
        c.line(3, y, 12, y, WOOD[1])
        c.put(13, y, STEEL[2])


@badge
def juggernaut(c):
    c.rect(4, 6, 11, 11, STEEL)
    c.line(4, 8, 11, 8, GOLD[1])


@badge
def catalyst(c):
    c.ellipse(8, 10, 3, 3, TOXIC)
    c.rect(7, 4, 9, 7, LINEN)
    c.put(8, 10, EMBER[2])


@badge
def static_build(c):
    c.line(10, 3, 6, 8, MAG[2])
    c.line(6, 8, 10, 8, MAG[2])
    c.line(10, 8, 6, 13, MAG[1])


@badge
def soul_hoard(c):
    c.ellipse(8, 9, 3, 4, DUSK)
    c.tri(8, 3, 6, 2, DUSK)
    c.put(7, 9, (255, 220, 240))


@badge
def sanctified(c):
    c.line(8, 3, 8, 13, HOLY[2])
    c.line(5, 6, 11, 6, HOLY[2])
    c.put(8, 13, EMBER[2])


@badge
def flow(c):
    c.curve(3, 11, 10, -6, 2, PETAL_PAL[2])
    c.curve(4, 13, 9, -4, 1, PETAL_PAL[1])


PETAL_PAL = ((150, 100, 130), (230, 170, 210), (255, 220, 240))


@badge
def rooted(c):
    c.line(8, 3, 8, 10, WOOD[1])
    c.line(8, 10, 5, 13, WOOD[0])
    c.line(8, 10, 11, 13, WOOD[0])
    c.ellipse(8, 4, 2, 1, LEAF)


@badge
def spare_parts(c):
    c.ellipse(8, 8, 4, 4, IRON)
    c.ellipse(8, 8, 2, 2, GOLD)
    c.put(8, 3, IRON[2])
    c.put(13, 8, IRON[2])


@badge
def prophecy(c):
    c.ellipse(8, 8, 5, 3, LINEN)
    c.ellipse(8, 8, 2, 2, ARC)
    c.put(8, 8, CORE[0])


@badge
def plunderer(c):
    c.rect(4, 7, 11, 11, WOOD)
    c.line(4, 7, 11, 7, GOLD[1])
    c.put(8, 9, GOLD[2])


TALENTS = (bloodthirst, glass_cannon, momentum, cold_blood, pyromaniac, scavenger,
           thick_skin, vengeance, night_owl, hunters_instinct, overflow, afterimage,
           spellweave, volley_master, juggernaut, catalyst, static_build, soul_hoard,
           sanctified, flow, rooted, spare_parts, prophecy, plunderer)


def spellweave_thread():
    c = Canvas()
    c.ellipse(8, 9, 4, 5, ARC)
    c.curve(3, 4, 10, 2, 3, ICE[2])
    c.line(8, 4, 8, 13, ARC[2])
    return c.image()


def wide_grip():
    c = Canvas()
    c.ellipse(8, 8, 6, 6, LEAF)
    c.ellipse(8, 8, 3, 3, ((30, 70, 30), (60, 120, 50), (120, 190, 100)))
    c.line(1, 8, 4, 8, LINEN[2])
    c.line(12, 8, 15, 8, LINEN[2])
    return c.image()


def swift_quiver():
    c = Canvas()
    c.rect(4, 6, 8, 14, WOOD)
    c.line(6, 1, 6, 6, STEEL[2])
    c.line(9, 2, 11, 6, STEEL[2])
    c.line(12, 6, 14, 4, LINEN[1])
    return c.image()


def keen_edge():
    c = Canvas()
    c.line(3, 13, 12, 4, STEEL[1], 2)
    c.line(4, 13, 13, 4, STEEL[2])
    c.put(13, 3, (255, 255, 255))
    c.line(2, 10, 5, 13, GOLD[1])
    return c.image()


def fleet_heart():
    c = Canvas()
    c.ellipse(6, 7, 3, 3, BLOOD)
    c.ellipse(10, 7, 3, 3, BLOOD)
    c.tri(8, 8, 13, 5, BLOOD)
    c.line(1, 11, 4, 11, LINEN[2])
    return c.image()


def ember_heart():
    c = Canvas()
    c.ellipse(6, 8, 3, 3, EMBER)
    c.ellipse(10, 8, 3, 3, EMBER)
    c.tri(8, 9, 14, 5, EMBER)
    c.tri(8, 2, 6, 1, STAR)
    return c.image()


def collector():
    c = Canvas()
    c.ellipse(8, 10, 5, 4, WOOD)
    c.line(5, 6, 11, 6, WOOD[0])
    c.ellipse(6, 5, 1, 1, ICE)
    c.ellipse(10, 5, 1, 1, GOLD)
    return c.image()


def summoners_seal():
    c = Canvas()
    c.ellipse(8, 8, 6, 6, DUSK)
    c.ellipse(8, 8, 4, 4, ((40, 14, 30), (70, 30, 55), (110, 60, 90)))
    c.tri(8, 4, 11, 3, LINEN)
    return c.image()


PASSIVES = (spellweave_thread, wide_grip, swift_quiver, keen_edge, fleet_heart, ember_heart,
            collector, summoners_seal)


def shrinekeepers_lamp():
    c = Canvas()
    c.line(8, 1, 8, 3, IRON[1])
    c.rect(5, 4, 11, 12, IRON)
    c.ellipse(8, 8, 2, 3, STAR)
    c.line(4, 13, 12, 13, IRON[0])
    return c.image()


def cartographers_quill():
    c = Canvas()
    c.rect(2, 8, 11, 14, LINEN)
    c.line(4, 10, 9, 12, WOOD[0])
    c.line(13, 1, 8, 9, BONE[2], 2)
    return c.image()


def moon_dial():
    c = Canvas()
    c.ellipse(8, 8, 6, 6, STEEL)
    c.ellipse(9, 7, 3, 3, VOID)
    c.line(8, 8, 8, 4, GOLD[2])
    return c.image()


def storm_glass():
    c = Canvas()
    c.ellipse(8, 9, 5, 5, ICE)
    c.rect(6, 2, 10, 4, WOOD)
    c.line(9, 6, 7, 9, MAG[2])
    c.line(7, 9, 9, 12, MAG[2])
    return c.image()


def snow_globe():
    c = Canvas()
    c.ellipse(8, 7, 5, 5, ICE)
    c.rect(4, 12, 12, 14, WOOD)
    c.tri(8, 6, 10, 2, LEAF)
    c.put(6, 5, (255, 255, 255))
    return c.image()


def sun_shard():
    c = Canvas()
    c.tri(8, 1, 14, 4, STAR)
    c.put(7, 6, (255, 255, 255))
    c.line(3, 14, 13, 14, EMBER[1])
    return c.image()


def bog_heart():
    c = Canvas()
    c.ellipse(6, 8, 3, 3, TOXIC)
    c.ellipse(10, 8, 3, 3, TOXIC)
    c.tri(8, 9, 14, 5, TOXIC)
    c.put(6, 7, (230, 255, 200))
    return c.image()


def hollow_crown():
    c = Canvas()
    c.rect(3, 8, 13, 12, GOLD)
    c.tri(4, 4, 8, 1, GOLD)
    c.tri(8, 3, 8, 1, GOLD)
    c.tri(12, 4, 8, 1, GOLD)
    c.rect(6, 9, 10, 11, CORE)
    return c.image()


def grave_dirt():
    c = Canvas()
    c.ellipse(8, 12, 6, 3, WOOD)
    c.rect(6, 3, 10, 10, STEEL)
    c.line(6, 5, 10, 5, STEEL[0])
    return c.image()


def pilgrims_staff():
    c = Canvas()
    c.line(5, 14, 10, 3, WOOD[1], 2)
    c.ellipse(11, 3, 2, 2, BONE)
    c.line(4, 9, 6, 11, LINEN[1])
    return c.image()


def tidecaller():
    c = Canvas()
    c.ellipse(8, 10, 6, 4, SEA)
    c.curve(3, 8, 10, -4, 2, SEA[2])
    c.ellipse(8, 4, 2, 2, ICE)
    return c.image()


def lantern_of_ages():
    c = Canvas()
    c.line(8, 1, 8, 3, GOLD[1])
    c.ellipse(8, 8, 4, 5, GOLD)
    c.ellipse(8, 8, 2, 3, STAR)
    c.rect(5, 13, 11, 14, GOLD)
    return c.image()


def wanderers_boots():
    c = Canvas()
    c.rect(5, 3, 9, 11, WOOD)
    c.rect(5, 10, 13, 13, WOOD)
    c.line(5, 5, 9, 5, GOLD[1])
    return c.image()


def star_chart():
    c = Canvas()
    c.rect(2, 3, 13, 13, LINEN)
    c.put(5, 6, VOID[1])
    c.put(10, 5, VOID[1])
    c.put(8, 10, VOID[1])
    c.line(5, 6, 8, 10, ARC[1])
    c.line(10, 5, 8, 10, ARC[1])
    return c.image()


def thunder_idol():
    c = Canvas()
    c.rect(4, 4, 11, 14, STEEL)
    c.put(6, 7, MAG[2])
    c.put(9, 7, MAG[2])
    c.line(10, 1, 7, 4, MAG[2])
    c.line(5, 11, 10, 11, IRON[0])
    return c.image()


def dawnstone():
    c = Canvas()
    c.ellipse(8, 9, 5, 5, ((150, 90, 60), (240, 160, 110), (255, 220, 170)))
    c.line(2, 12, 14, 12, HOLY[2])
    c.put(7, 7, (255, 255, 255))
    return c.image()


RELICS = (shrinekeepers_lamp, cartographers_quill, moon_dial, storm_glass, snow_globe,
          sun_shard, bog_heart, hollow_crown, grave_dirt, pilgrims_staff, tidecaller,
          lantern_of_ages, wanderers_boots, star_chart, thunder_idol, dawnstone)


def icons():
    out = [(i, f()) for i, f in enumerate(WEAPONS)]
    start = ARSENAL["skills"].start
    out += [(start + i, f()) for i, f in enumerate(SKILLS)]
    out += [(ARSENAL["talents"].start + i, f()) for i, f in enumerate(TALENTS)]
    out += [(ARSENAL["passives"].start + i, f()) for i, f in enumerate(PASSIVES)]
    out += [(ARSENAL["relics"].start + i, f()) for i, f in enumerate(RELICS)]
    return out


def add_arsenal(atlas):
    from make_expansion import art_errors

    atlas = extend(atlas)
    errors = []
    for index, art in icons():
        errors += art_errors(art, f"arsenal {index}")
        place_cell(atlas, BAND, index, art)
    if errors:
        raise RuntimeError("invalid arsenal art:\n" + "\n".join(errors))
    return atlas


def preview(path, scale=8):
    items = icons()
    sheet = Image.new("RGBA", (len(items) * 18 * scale, 18 * scale), (40, 34, 52, 255))
    for n, (_, art) in enumerate(items):
        big = art.resize((16 * scale, 16 * scale), Image.NEAREST)
        sheet.alpha_composite(big, (n * 18 * scale + scale, scale))
    sheet.save(path)


def main():
    root = Path(__file__).resolve().parent.parent
    atlas_path = root / "res" / "atlas.png"
    atlas = add_arsenal(Image.open(atlas_path).convert("RGBA"))
    errors = validate(atlas)
    if errors:
        raise RuntimeError("invalid atlas layout:\n" + "\n".join(errors))
    atlas.save(atlas_path)
    if "--preview" in sys.argv:
        preview(Path(sys.argv[sys.argv.index("--preview") + 1]))
    print(f"wrote {atlas_path}")


if __name__ == "__main__":
    main()
