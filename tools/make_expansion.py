"""Paints expansion sprites and creates native-scale fidelity contact sheets."""

import sys
from pathlib import Path

from PIL import Image, ImageDraw

sys.path.insert(0, str(Path(__file__).resolve().parent))

from atlas_layout import BANDS, extend, place_cell, validate
from make_sprites import CHAR, LEAF, PALE, STONE, add_sprites
from sprite_kit import Canvas, OUTLINE as KIT_OUTLINE, _hash, mix


ACTORS = BANDS[6]
OUTLINE = (50, 30, 44)
LEECH = ((72, 44, 76), (126, 68, 104), (210, 126, 150))
LEATHER = ((92, 54, 34), (150, 94, 52), (222, 166, 88))
WOOD = ((76, 48, 34), (126, 78, 46), (190, 130, 72))
MONSTERS = ("Mire Leech", "Bog Lantern", "Glass Mantis", "Dust Vulture",
            "Frost Wisp", "Iceback", "Sporeling", "Mycelial Brute",
            "Ember Imp", "Cinder Golem")
HEROES = ("Warden", "Tinkerer", "Oracle", "Corsair")
WEAPONS = ("Ricochet Sling", "Harpoon", "Ground Sigil", "Boomerang",
           "Mortar", "Thorn Lash", "Prism Beam", "Spirit Swarm")
RELICS = ("Carnage Bell", "Last Bastion", "Pilgrim Compass", "Plague Seed",
          "Recall Mirror", "Weather Vane", "Hunter's Writ", "Mercy Locket")
BIOMES = ("Salt Flats", "Floodplain", "Moonlit Grove", "Sunken Ruins",
          "Ember Fen", "Amber Woods", "Storm Steppe", "Glass Desert",
          "Hollow Caverns", "Starfall Basin")
PALETTES = (
    ((132, 128, 132), (186, 184, 176), (234, 234, 216)),
    ((34, 76, 78), (54, 108, 100), (94, 148, 112)),
    ((30, 38, 72), (48, 66, 104), (90, 120, 152)),
    ((46, 80, 88), (76, 114, 124), (132, 160, 158)),
    ((66, 38, 42), (106, 64, 52), (174, 110, 62)),
    ((92, 66, 40), (138, 104, 54), (196, 152, 74)),
    ((64, 78, 68), (100, 120, 88), (156, 170, 116)),
    ((104, 110, 138), (156, 162, 182), (218, 218, 228)),
    ((42, 34, 54), (68, 54, 78), (106, 86, 118)),
    ((50, 44, 90), (84, 76, 130), (140, 130, 176)),
)
ACCENTS = ((244, 242, 224), (154, 198, 126), (170, 212, 238),
           (94, 212, 190), (255, 154, 54), (246, 198, 80),
           (226, 238, 172), (166, 242, 244), (170, 146, 216),
           (244, 218, 158))
METAL = ((64, 74, 94), (126, 146, 168), (218, 234, 238))
GOLD = ((128, 82, 36), (204, 150, 52), (250, 220, 122))
BONE = ((116, 104, 96), (190, 178, 154), (244, 232, 196))
SKIN = ((136, 76, 66), (204, 138, 110), (250, 192, 146))


def ink(c, x, y, rows, pal):
    for dy, row in enumerate(rows):
        for dx, k in enumerate(row):
            if k != ".":
                c.put(x + dx, y + dy, pal[int(k)])


def hero(index):
    if index == 0:
        c = Canvas()
        forest = ((24, 52, 34), (46, 98, 56), (86, 162, 92))
        emerald = ((18, 76, 44), (48, 156, 82), (134, 232, 126))
        bark = ((54, 36, 26), (98, 64, 42), (160, 110, 68))
        c.line(12, 3, 12, 14, bark[1])
        c.line(11, 4, 11, 14, bark[0])
        c.cluster(11, 1, (".##", "###"), emerald[2])
        c.put(12, 1, emerald[1])
        c.tri(7.5, 1, 4, 3.5, forest)
        c.put(7, 1, forest[2])
        c.put(8, 1, forest[2])
        c.ellipse(7.5, 5, 2.8, 2.2, SKIN)
        c.put(6, 5, OUTLINE)
        c.put(9, 5, OUTLINE)
        c.line(6, 4, 9, 4, forest[0])
        c.put(7, 7, GOLD[2])
        c.put(8, 7, emerald[2])
        c.rect(5, 8, 10, 12, forest)
        c.line(5, 8, 5, 12, forest[2])
        c.line(10, 8, 10, 12, forest[0])
        c.line(7, 8, 8, 12, bark[1])
        c.rect(3, 8, 4, 11, LEATHER)
        c.put(3, 9, LEATHER[2])
        c.rect(5, 13, 6, 14, bark)
        c.rect(9, 13, 10, 14, bark)
        return c.image(True)
    if index == 1:
        c = Canvas()
        brass = ((88, 52, 24), (164, 106, 42), (236, 182, 78))
        steel = ((52, 60, 74), (108, 124, 144), (196, 216, 228))
        cyan = ((20, 86, 114), (52, 168, 206), (160, 240, 255))
        apron = ((64, 40, 30), (114, 72, 50), (174, 118, 86))
        c.rect(4, 2, 11, 4, LEATHER)
        c.line(5, 2, 10, 2, LEATHER[2])
        c.rect(4, 3, 11, 4, brass)
        c.put(6, 3, cyan[2])
        c.put(6, 4, cyan[1])
        c.put(9, 3, cyan[2])
        c.put(9, 4, cyan[1])
        c.put(7, 3, brass[0])
        c.put(8, 3, brass[0])
        c.ellipse(7.5, 6, 2.5, 1.8, SKIN)
        c.put(7, 6, (170, 96, 76))
        c.rect(11, 7, 13, 12, METAL)
        c.line(12, 7, 12, 11, GOLD[2])
        c.put(12, 8, cyan[2])
        c.rect(2, 8, 4, 11, METAL)
        c.cluster(1, 6, ("###", "#.#"), brass[2])
        c.rect(5, 7, 10, 12, apron)
        c.line(5, 7, 5, 12, apron[2])
        c.line(7, 9, 8, 9, LEATHER[0])
        c.put(7, 8, GOLD[2])
        c.put(8, 8, GOLD[2])
        c.rect(5, 13, 6, 14, steel)
        c.rect(9, 13, 10, 14, steel)
        return c.image(True)
    if index == 2:
        c = Canvas()
        astral = ((32, 22, 56), (64, 48, 112), (124, 94, 184))
        star = ((60, 110, 170), (120, 190, 245), (230, 250, 255))
        veil = ((54, 38, 80), (98, 74, 140), (170, 146, 218))
        c.tri(7.5, 1, 5, 4.5, astral)
        c.line(5, 2, 7, 2, veil[2])
        c.cluster(6, 2, (".##.", "####", ".##."), GOLD[2])
        c.put(7, 3, star[2])
        c.ellipse(7.5, 5, 2.6, 2.2, SKIN)
        c.put(6, 5, (80, 50, 120))
        c.put(9, 5, (80, 50, 120))
        c.cluster(2, 7, (".#.", "###", ".#."), star[2])
        c.put(3, 8, (255, 255, 255))
        c.put(1, 8, GOLD[1])
        c.put(5, 8, GOLD[2])
        c.put(3, 6, GOLD[2])
        c.put(3, 10, GOLD[0])
        c.rect(5, 7, 10, 12, astral)
        c.line(5, 7, 5, 12, veil[2])
        c.line(10, 7, 10, 12, astral[0])
        c.put(6, 9, star[2])
        c.put(9, 10, GOLD[2])
        c.put(7, 11, star[1])
        c.rect(11, 7, 12, 11, veil)
        c.rect(5, 13, 6, 14, astral)
        c.rect(9, 13, 10, 14, astral)
        return c.image(True)
    c = Canvas()
    crimson = ((78, 24, 38), (152, 44, 58), (228, 92, 102))
    navy = ((24, 30, 50), (42, 54, 88), (80, 102, 150))
    steel = ((52, 60, 74), (108, 124, 144), (196, 216, 228))
    plume = ((140, 136, 146), (196, 194, 204), (250, 248, 255))
    c.rect(3, 2, 12, 3, navy)
    c.rect(5, 1, 10, 2, navy)
    c.line(3, 2, 12, 2, GOLD[2])
    c.put(4, 1, GOLD[2])
    c.put(11, 1, GOLD[1])
    c.cluster(10, 0, ("..#", ".##", "##."), plume[2])
    c.ellipse(7.5, 5, 2.7, 2.2, SKIN)
    c.line(5, 4, 10, 4, navy[0])
    c.put(6, 5, (40, 20, 30))
    c.put(9, 5, OUTLINE)
    c.cluster(7, 6, ("##", "##"), (245, 240, 235))
    c.put(7, 7, GOLD[2])
    c.rect(5, 8, 10, 12, crimson)
    c.line(5, 8, 5, 12, crimson[2])
    c.line(10, 8, 10, 12, crimson[0])
    c.line(6, 8, 9, 11, LEATHER[0])
    c.line(12, 6, 13, 12, steel[2], 2)
    c.cluster(11, 6, ("##", "##"), GOLD[2])
    c.rect(3, 8, 4, 11, navy)
    c.put(3, 11, GOLD[2])
    c.rect(5, 13, 6, 14, LEATHER)
    c.rect(9, 13, 10, 14, LEATHER)
    return c.image(True)


def monster(index):
    if index == 0:
        return mire_leech()
    if index == 1:
        c = Canvas()
        lantern_pale = ((36, 74, 48), (62, 140, 82), (148, 234, 150))
        c.cluster(6, 1, (".##.", "#..#"), WOOD[0])
        c.ellipse(7.5, 7.5, 4.5, 5, WOOD)
        c.ellipse(7.5, 7.5, 2.8, 3.2, lantern_pale)
        c.put(7, 7, (245, 255, 210))
        c.put(8, 7, (245, 255, 210))
        c.line(7, 3, 7, 12, WOOD[1])
        c.line(5, 5, 5, 10, WOOD[0])
        c.line(10, 5, 10, 10, WOOD[2])
        c.cluster(4, 12, ("#.", ".#", "#."), lantern_pale[1])
        c.cluster(9, 12, (".#", "#.", ".#"), lantern_pale[0])
        return c.image(True)
    if index == 2:
        c = Canvas()
        crystal = ((44, 94, 98), (88, 168, 164), (188, 244, 238))
        amber_eye = ((130, 80, 24), (210, 140, 42), (255, 215, 90))
        c.tri(7.5, 2, 5, 3, crystal)
        c.put(5, 3, amber_eye[2])
        c.put(9, 3, amber_eye[2])
        c.line(7, 5, 7, 13, crystal[1], 2)
        c.ellipse(7.5, 10, 2.5, 3.5, crystal)
        c.line(6, 5, 2, 8, crystal[2], 2)
        c.line(2, 8, 4, 11, crystal[1])
        c.line(9, 5, 13, 8, crystal[1], 2)
        c.line(13, 8, 11, 11, crystal[0])
        c.line(5, 11, 3, 14, crystal[0])
        c.line(10, 11, 12, 14, crystal[0])
        return c.image(True)
    if index == 3:
        c = Canvas()
        feather = ((42, 34, 38), (82, 68, 72), (142, 122, 126))
        ruff = ((84, 76, 70), (142, 132, 120), (210, 200, 186))
        raw_skin = ((124, 46, 42), (184, 78, 68), (238, 132, 118))
        wing_pts = ("..####......####..",
                    "##################",
                    ".####...##...####.")
        c.cluster(1, 4, wing_pts, feather[1])
        c.material(points(wing_pts, 1, 4), feather[0], feather[2], 42, 0.25)
        c.ellipse(7.5, 8.5, 3.5, 4, feather)
        c.ellipse(7.5, 6, 2.8, 2, ruff)
        c.ellipse(7.5, 4, 1.8, 1.8, raw_skin)
        c.put(8, 4, OUTLINE)
        c.line(8, 5, 10, 6, GOLD[2])
        c.put(9, 6, GOLD[0])
        c.put(6, 13, GOLD[1])
        c.put(9, 13, GOLD[1])
        return c.image(True)
    if index == 4:
        c = Canvas()
        glacial = ((64, 118, 156), (118, 184, 216), (218, 244, 252))
        cyan = ((20, 86, 114), (52, 168, 206), (160, 240, 255))
        c.ellipse(7.5, 7.5, 3.8, 4.5, glacial)
        c.tri(7.5, 8, 13, 2.5, glacial)
        c.put(6, 6, (255, 255, 255))
        c.put(8, 6, (255, 255, 255))
        c.put(7, 7, cyan[2])
        c.cluster(2, 4, (".#", "##", "#."), glacial[2])
        c.cluster(12, 5, ("#.", "##", ".#"), glacial[1])
        c.cluster(7, 1, (".#.", "###"), glacial[2])
        c.curve(7, 10, -2, 3, 1, glacial[0])
        return c.image(True)
    if index == 5:
        c = Canvas()
        glacial = ((64, 118, 156), (118, 184, 216), (218, 244, 252))
        fur = ((52, 64, 82), (96, 112, 136), (168, 184, 208))
        c.ellipse(7.5, 9.5, 5.5, 4.5, fur)
        c.tri(5, 2, 6, 2, glacial)
        c.tri(8, 1, 6, 2.2, glacial)
        c.tri(11, 3, 7, 1.8, glacial)
        c.ellipse(3.5, 8.5, 2.5, 2.2, fur)
        c.put(2, 7, glacial[2])
        c.put(3, 8, (120, 220, 255))
        c.rect(4, 12, 5, 14, fur)
        c.rect(9, 12, 10, 14, fur)
        c.put(4, 14, glacial[1])
        c.put(9, 14, glacial[1])
        return c.image(True)
    if index == 6:
        c = Canvas()
        spore_purple = ((68, 28, 82), (132, 58, 142), (202, 118, 214))
        stem = ((128, 118, 134), (188, 178, 196), (236, 228, 244))
        cyan = ((20, 86, 114), (52, 168, 206), (160, 240, 255))
        c.ellipse(7.5, 5.5, 5.8, 4.2, spore_purple)
        c.put(5, 3, cyan[2])
        c.put(8, 2, cyan[2])
        c.put(10, 4, cyan[2])
        c.put(6, 6, cyan[1])
        c.rect(6, 8, 9, 12, stem)
        c.put(6, 9, spore_purple[0])
        c.put(9, 9, spore_purple[0])
        c.cluster(3, 8, (".#", "##", ".#"), cyan[1])
        c.rect(5, 12, 6, 14, stem)
        c.rect(9, 12, 10, 14, stem)
        return c.image(True)
    if index == 7:
        c = Canvas()
        flesh = ((58, 42, 62), (104, 78, 110), (164, 132, 172))
        shelf = ((136, 78, 38), (196, 122, 58), (242, 182, 94))
        spore_purple = ((68, 28, 82), (132, 58, 142), (202, 118, 214))
        c.ellipse(7.5, 8.5, 5.5, 5, flesh)
        c.cluster(1, 4, ("..###", ".####", "#####"), shelf[1])
        c.cluster(10, 5, ("###..", "####.", "#####"), shelf[1])
        c.ellipse(7.5, 4.5, 3.2, 2.5, flesh)
        c.put(6, 4, spore_purple[2])
        c.put(8, 4, spore_purple[2])
        c.rect(11, 7, 13, 12, WOOD)
        c.cluster(11, 10, ("###", "###"), shelf[2])
        c.rect(4, 12, 6, 14, flesh)
        c.rect(9, 12, 11, 14, flesh)
        return c.image(True)
    if index == 8:
        c = Canvas()
        imp = ((88, 26, 32), (164, 46, 44), (234, 98, 72))
        horn = ((32, 24, 30), (62, 52, 60), (108, 96, 106))
        molten = ((138, 38, 24), (218, 72, 34), (255, 176, 56))
        c.cluster(1, 4, ("##...##", ".##.##."), imp[0])
        c.cluster(8, 4, ("##...##", ".##.##."), imp[0])
        c.ellipse(7.5, 6, 3, 2.8, imp)
        c.line(5, 4, 3, 2, horn[1])
        c.put(3, 2, horn[2])
        c.line(10, 4, 12, 2, horn[1])
        c.put(12, 2, horn[2])
        c.put(6, 6, molten[2])
        c.put(9, 6, molten[2])
        c.rect(6, 8, 9, 12, imp)
        c.put(7, 9, molten[1])
        c.line(9, 12, 12, 13, imp[1])
        c.put(13, 12, molten[2])
        c.rect(5, 12, 6, 14, imp)
        c.rect(8, 12, 9, 14, imp)
        return c.image(True)
    c = Canvas()
    basalt = ((40, 36, 46), (72, 66, 82), (120, 114, 134))
    molten = ((138, 38, 24), (218, 72, 34), (255, 176, 56))
    c.rect(3, 4, 12, 12, basalt)
    c.material(points(("##########", "##########", "##########",
                       "##########", "##########", "##########"), 3, 4),
               basalt[0], basalt[2], 77, 0.28)
    c.rect(5, 1, 10, 4, basalt)
    c.put(6, 3, molten[2])
    c.put(9, 3, molten[2])
    c.cluster(6, 6, (".##.", "####", ".##."), molten[2])
    c.put(7, 7, (255, 255, 200))
    c.line(4, 8, 6, 7, molten[1])
    c.line(11, 8, 9, 7, molten[1])
    c.line(7, 9, 7, 12, molten[1])
    c.rect(4, 12, 6, 14, basalt)
    c.rect(9, 12, 11, 14, basalt)
    c.put(5, 13, molten[1])
    c.put(10, 13, molten[1])
    return c.image(True)


def weapon(index):
    if index == 0:
        return ricochet_sling()
    c = Canvas()
    if index == 1:
        c.line(3, 13, 11, 5, WOOD[1], 2)
        c.tri(11, 1, 6, 2, METAL)
        c.line(9, 5, 13, 5, METAL[2])
    elif index == 2:
        c.ellipse(8, 9, 6, 4, PALETTES[9])
        c.ellipse(8, 9, 4, 2, GOLD)
        c.line(8, 3, 8, 12, ACCENTS[9])
        c.line(4, 9, 11, 9, ACCENTS[9])
    elif index == 3:
        c.line(3, 11, 7, 3, WOOD[1], 2)
        c.line(7, 3, 13, 10, WOOD[2], 2)
        c.line(4, 10, 7, 5, GOLD[1])
    elif index == 4:
        c.ellipse(8, 12, 6, 2, METAL)
        c.line(6, 10, 10, 4, METAL[1], 4)
        c.ellipse(11, 4, 2, 2, METAL)
        c.ellipse(11, 4, 1, 1, (OUTLINE,) * 3)
    elif index == 5:
        c.line(3, 13, 5, 9, WOOD[1], 2)
        c.curve(5, 9, 7, -6, 2, PALETTES[1][2], 2)
        for x, y in ((7, 6), (10, 5), (11, 7)):
            c.put(x + 1, y - 1, PALETTES[1][0])
    elif index == 6:
        c.tri(8, 2, 12, 5, METAL)
        c.line(8, 3, 4, 11, ACCENTS[7])
        c.line(8, 3, 11, 11, ACCENTS[9])
        c.line(4, 11, 11, 11, (224, 146, 198))
    else:
        for x, y in ((5, 5), (11, 8), (6, 12)):
            c.ellipse(x, y, 2, 2, PALETTES[2])
            c.put(x - 1, y - 1, ACCENTS[2])
        c.line(7, 6, 9, 7, PALETTES[2][1])
        c.line(9, 10, 7, 11, PALETTES[2][0])
    return c.image()


def relic(index):
    c = Canvas()
    if index == 0:
        c.ellipse(8, 8, 4, 5, GOLD)
        c.rect(3, 11, 12, 12, GOLD)
        c.ellipse(8, 13, 1, 1, BONE)
        c.line(6, 4, 9, 7, (174, 48, 58))
        c.put(8, 2, GOLD[2])
    elif index == 1:
        ink(c, 3, 2, ("111111111", "122222221", "123333321", "123333321",
                      ".1233321.", ".1233321.", "..12321..", "...121...", "....1...."),
            (OUTLINE, METAL[0], METAL[2], PALETTES[9][1]))
        c.line(7, 4, 7, 8, GOLD[2], 2)
    elif index == 2:
        c.ellipse(8, 8, 5, 5, GOLD)
        c.ellipse(8, 8, 3, 3, PALETTES[2])
        c.tri(8, 4, 10, 2, BONE)
        c.put(8, 7, (204, 60, 66))
    elif index == 3:
        c.ellipse(8, 9, 4, 5, PALETTES[1])
        c.cluster(5, 5, (".##.", "####", ".###"), (182, 122, 202))
        c.line(8, 8, 10, 12, PALETTES[1][0])
    elif index == 4:
        c.ellipse(8, 8, 5, 5, GOLD)
        c.ellipse(8, 8, 3, 3, (OUTLINE,) * 3)
        c.line(5, 6, 9, 5, ACCENTS[7])
        c.line(9, 5, 11, 8, ACCENTS[7])
        c.line(11, 8, 8, 10, ACCENTS[7])
    elif index == 5:
        c.line(8, 3, 8, 13, METAL[1], 2)
        c.line(3, 6, 13, 6, GOLD[1])
        c.tri(12, 3, 8, 2, GOLD)
        c.cluster(2, 4, ("##", "##", "##"), METAL[2])
        c.rect(5, 13, 11, 14, METAL)
    elif index == 6:
        c.rect(4, 3, 11, 12, BONE)
        c.rect(3, 2, 12, 3, GOLD)
        c.rect(3, 12, 12, 13, GOLD)
        c.line(6, 5, 9, 5, WOOD[0])
        c.line(6, 7, 10, 7, WOOD[0])
        c.ellipse(8, 10, 2, 2, ((112, 34, 42), (180, 56, 54), (228, 116, 84)))
    else:
        c.ellipse(8, 4, 3, 2, GOLD)
        c.ellipse(8, 10, 4, 4, GOLD)
        c.ellipse(8, 10, 2.5, 2.5, PALETTES[1])
        c.cluster(6, 8, (".#.", "###", ".#."), (198, 230, 174))
    return c.image()


def shrine_symbol(index):
    FIRE = ((140, 28, 24), (220, 64, 32), (255, 160, 48), (255, 235, 120))
    EMERALD = ((18, 64, 36), (42, 140, 72), (110, 220, 120), (200, 255, 190))
    ASTRAL = ((28, 18, 52), (62, 44, 110), (124, 92, 180))
    CYAN = ((18, 70, 110), (46, 150, 215), (140, 235, 255), (240, 255, 255))
    if index == 0:
        c = Canvas()
        c.line(7.5, 2, 7.5, 13, FIRE[1], 2)
        c.line(7.5, 4, 7.5, 11, FIRE[2], 1)
        c.put(7, 3, FIRE[3])
        c.put(8, 3, FIRE[3])
        c.curve(3, 11, 4, -8, 2, FIRE[0], 2)
        c.curve(12, 11, 11, -8, 2, FIRE[0], 2)
        c.line(4, 8, 4, 11, FIRE[1])
        c.line(11, 8, 11, 11, FIRE[1])
        c.put(4, 5, FIRE[2])
        c.put(11, 5, FIRE[2])
        c.rect(5, 12, 10, 14, ((70, 16, 20), (120, 24, 28), (180, 42, 40)))
        c.put(7, 13, GOLD[2])
        c.put(8, 13, GOLD[2])
        return c.image(True)
    if index == 1:
        c = Canvas()
        c.ellipse(7.5, 6.5, 3.5, 3.5, EMERALD)
        c.put(7, 6, EMERALD[3])
        c.put(8, 6, EMERALD[3])
        c.curve(2, 14, 5, -8, 2, WOOD[1], 2)
        c.curve(13, 14, 10, -8, 2, WOOD[0], 2)
        c.line(4, 11, 11, 13, WOOD[1], 2)
        c.line(11, 11, 4, 13, WOOD[0], 2)
        c.line(7.5, 10, 7.5, 14, WOOD[1], 2)
        c.cluster(6, 1, (".##", "###"), EMERALD[2])
        c.put(7, 2, EMERALD[3])
        c.cluster(1, 4, ("##", ".#"), EMERALD[1])
        c.cluster(12, 4, ("##", "#."), EMERALD[1])
        return c.image(True)
    if index == 2:
        c = Canvas()
        c.tri(7.5, 2, 9, 6, ASTRAL)
        c.tri(7.5, 13, 9, -6, ASTRAL)
        c.ellipse(7.5, 7.5, 4.0, 2.5, ((200, 210, 230), (235, 242, 255), (255, 255, 255)))
        c.ellipse(7.5, 7.5, 2.0, 2.0, CYAN)
        c.put(7, 7, GOLD[2])
        c.put(8, 7, GOLD[2])
        return c.image(True)
    c = Canvas()
    c.rect(4, 10, 11, 12, GOLD)
    c.line(5, 10, 10, 10, GOLD[2])
    c.line(6, 13, 9, 13, GOLD[1], 2)
    c.line(4, 14, 11, 14, GOLD[0])
    c.put(7, 11, (220, 50, 40))
    c.put(8, 11, (220, 50, 40))
    c.line(7.5, 2, 7.5, 9, FIRE[2], 2)
    c.line(7.5, 4, 7.5, 8, FIRE[3], 1)
    c.put(7, 3, (255, 255, 255))
    c.put(8, 3, (255, 255, 255))
    c.curve(4, 9, 4, -4, 1, FIRE[1], 2)
    c.curve(11, 9, 11, -4, 1, FIRE[1], 2)
    c.put(5, 5, FIRE[2])
    c.put(10, 5, FIRE[2])
    return c.image(True)



def biome_prop(biome, variant):
    c = Canvas()
    pal, accent = PALETTES[biome], ACCENTS[biome]
    if biome == 0:
        if variant == 0:
            c.tri(8, 3, 13, 5, pal)
            c.line(7, 5, 4, 12, accent)
        elif variant == 1:
            c.ellipse(8, 12, 6, 2, pal)
            c.cluster(4, 10, (".##..##", "##.###."), accent)
        else:
            c.line(3, 12, 11, 9, BONE[1], 2)
            c.ellipse(11, 7, 3, 3, BONE)
            c.put(10, 6, OUTLINE)
            c.put(12, 6, OUTLINE)
    elif biome == 1:
        if variant == 0:
            for x, h in ((4, 8), (7, 12), (11, 10)):
                c.line(x, 14, x, 14 - h, pal[2])
                c.rect(x, 14 - h, x + 1, 17 - h, WOOD)
        elif variant == 1:
            c.ellipse(8, 11, 6, 2, pal)
            c.cluster(5, 8, (".#.", "###", ".#."), (232, 196, 182))
        else:
            c.rect(5, 5, 10, 14, WOOD)
            c.line(6, 7, 3, 4, WOOD[0], 2)
            c.line(9, 9, 13, 6, WOOD[1], 2)
            c.cluster(4, 5, ("###.", ".###"), pal[1])
    elif biome == 2:
        if variant == 0:
            c.rect(7, 4, 9, 14, pal)
            for x, y, rx in ((5, 5, 3), (10, 4, 4), (7, 8, 5)):
                c.ellipse(x, y, rx, 2, pal)
            c.cluster(4, 3, (".##", "##."), accent)
        elif variant == 1:
            c.ellipse(8, 11, 5, 3, pal)
            for x, y in ((5, 10), (8, 8), (11, 11)):
                c.put(x, y, accent)
        else:
            c.line(8, 13, 8, 6, pal[2])
            for x, y in ((5, 6), (8, 4), (11, 7)):
                c.ellipse(x, y, 2, 2, pal)
                c.put(x, y - 1, accent)
    elif biome == 3:
        if variant == 0:
            c.rect(5, 4, 10, 13, pal)
            c.rect(3, 3, 12, 5, pal)
            c.rect(3, 13, 12, 14, pal)
            c.line(8, 6, 7, 10, pal[0])
        elif variant == 1:
            c.rect(3, 9, 12, 13, pal)
            c.rect(5, 6, 10, 9, pal)
            c.line(4, 10, 10, 10, accent)
        else:
            c.ellipse(8, 11, 5, 3, WOOD)
            c.line(4, 10, 11, 12, METAL[1])
            c.line(7, 8, 7, 13, METAL[0])
    elif biome == 4:
        if variant == 0:
            c.rect(7, 5, 9, 14, pal)
            c.line(7, 8, 3, 3, pal[0], 2)
            c.line(9, 8, 12, 4, pal[1])
            c.put(4, 4, accent)
            c.put(10, 7, accent)
        elif variant == 1:
            c.ellipse(8, 11, 6, 3, pal)
            c.line(4, 11, 7, 10, accent)
            c.line(7, 10, 10, 12, (224, 70, 34))
        else:
            c.rect(7, 8, 9, 14, pal)
            c.ellipse(8, 6, 5, 3, pal)
            c.cluster(5, 4, (".###", "###."), accent)
    elif biome == 5:
        if variant == 0:
            c.rect(7, 4, 9, 14, WOOD)
            c.ellipse(8, 5, 6, 4, pal)
            c.cluster(3, 3, (".###", "###."), accent)
        elif variant == 1:
            c.tri(8, 2, 13, 4, pal)
            c.line(8, 3, 6, 12, accent)
            c.ellipse(8, 8, 1, 2, WOOD)
        else:
            c.ellipse(8, 11, 5, 3, pal)
            c.cluster(5, 8, ("##..##", ".###.."), accent)
    elif biome == 6:
        if variant == 0:
            for x, h in ((3, 6), (6, 10), (9, 12), (12, 7)):
                c.line(8, 14, x, 14 - h, pal[1])
                c.put(x, 14 - h, accent)
        elif variant == 1:
            c.rect(6, 3, 10, 14, pal)
            c.line(7, 5, 9, 7, accent)
            c.line(9, 7, 7, 11, accent)
        else:
            c.line(5, 14, 9, 2, WOOD[1], 2)
            c.line(9, 3, 13, 6, pal[2], 2)
            c.line(8, 8, 2, 6, pal[0], 2)
    elif biome == 7:
        if variant == 0:
            c.tri(6, 2, 14, 3, pal)
            c.tri(11, 6, 14, 2, pal)
            c.line(6, 3, 5, 12, accent)
        elif variant == 1:
            c.ellipse(8, 11, 6, 3, pal)
            c.cluster(3, 10, (".####..", "##..###"), accent)
        else:
            c.line(7, 14, 7, 4, pal[1], 2)
            c.line(7, 7, 3, 5, pal[2], 2)
            c.line(8, 9, 12, 6, pal[2], 2)
            c.put(7, 3, accent)
    elif biome == 8:
        if variant == 0:
            c.tri(5, 3, 14, 3, pal)
            c.tri(11, 7, 14, 2, pal)
            c.line(5, 5, 4, 12, pal[2])
        elif variant == 1:
            for x, y in ((4, 12), (8, 9), (12, 13)):
                c.rect(x, y - 2, x, 14, pal)
                c.ellipse(x, y - 3, 2, 1.5, pal)
                c.put(x - 1, y - 3, accent)
        else:
            c.rect(3, 5, 12, 13, pal)
            c.ellipse(8, 7, 4, 4, (OUTLINE,) * 3)
            c.line(3, 13, 12, 13, pal[1])
    else:
        if variant == 0:
            c.ellipse(8, 10, 6, 4, pal)
            c.line(4, 8, 10, 12, accent)
            c.line(9, 7, 7, 13, accent)
        elif variant == 1:
            c.line(8, 14, 8, 6, pal[1])
            c.cluster(4, 4, ("...#...", ".#####.", "..###..", "#######",
                              "..###..", ".#...#."), accent)
        else:
            c.tri(8, 2, 13, 4, pal)
            c.line(8, 3, 6, 12, accent)
            c.line(8, 6, 10, 9, (196, 170, 228))
    return c.image()


def biome_marker(biome):
    c = Canvas()
    pal, accent = PALETTES[biome], ACCENTS[biome]
    c.rect(2, 12, 13, 14, pal)
    if biome in (0, 5, 8):
        c.rect(4, 6, 11, 11, WOOD)
        c.rect(4, 5, 11, 7, pal)
        c.line(5, 8, 10, 8, GOLD[1])
        c.put(8, 9, accent)
    elif biome in (1, 3, 6):
        c.rect(3, 3, 5, 12, pal)
        c.rect(10, 3, 12, 12, pal)
        c.rect(4, 2, 11, 4, pal)
        c.ellipse(8, 7, 1.5, 2, (accent,) * 3)
    else:
        c.rect(6, 6, 10, 12, pal)
        c.rect(4, 5, 12, 7, pal)
        c.tri(8, 1, 5, 2, (pal[0], accent, accent))
    return c.image()


def ground_fill(biome, variant):
    pal, accent = PALETTES[biome], ACCENTS[biome]
    out = Image.new("RGBA", (16, 16), (*pal[1], 255))
    px = out.load()
    for y in range(16):
        for x in range(16):
            cluster = _hash(x // 3, y // 2, 230 + biome)
            shade = pal[0] if cluster < 0.14 else pal[2] if cluster > 0.88 else pal[1]
            if biome == 0 and ((x + y // 5 * 2) % 9 == 0 or (y + x // 7) % 11 == 0):
                shade = pal[2]
            elif biome == 1 and y % 6 == 2 and x % 9 < 4:
                shade = (62, 134, 148)
            elif biome == 2 and (x + 3 * y) % 29 == 0:
                shade = pal[2]
            elif biome == 3 and (y % 7 == 0 or (x + (y // 7 % 2) * 5) % 10 == 0):
                shade = pal[0]
            elif biome == 4 and y == (x // 4 * 3 + 4) % 16:
                shade = mix(pal[0], accent, 0.35)
            elif biome == 5 and (x + y * 2) % 13 < 2:
                shade = mix(pal[1], accent, 0.35)
            elif biome == 6 and y % 5 == 0 and x % 7 < 3:
                shade = pal[2]
            elif biome == 7 and (x - y) % 11 == 0:
                shade = mix(pal[2], accent, 0.3)
            elif biome == 8 and y % 7 == 3 and x % 8 < 5:
                shade = pal[0]
            elif biome == 9 and (x * 5 + y * 7) % 43 == 0:
                shade = accent
            px[x, y] = (*shade, 255)
    if variant:
        c = Canvas()
        c.px = {(x, y): px[x, y][:3] for y in range(16) for x in range(16)}
        if variant == 1:
            c.cluster(4, 5, ("##..", ".###", "..#."), pal[0])
            c.cluster(10, 12, (".##", "##."), pal[2])
        else:
            c.cluster(7, 7, (".#.", "###", ".#."), accent)
            c.put(7, 8, pal[0])
        return c.image(False)
    return out


def ground_edge(atlas, biome, cell):
    row, col = divmod(cell - 3, 3)
    original = sprite(atlas, 216 + col * 18 + 1, (row + 1) * 18 + 1)
    base = ground_fill(biome, 0)
    px = original.load()
    out = original.copy()
    dst = out.load()
    fill = base.load()
    pal = PALETTES[biome]
    for y in range(16):
        for x in range(16):
            r, g, b, a = px[x, y]
            if a and r > g > b and r - b > 40:
                t = max(0, min(1, (r + g + b - 210) / 300))
                color = mix(fill[x, y][:3], pal[0] if t < 0.45 else pal[2], 0.2)
                dst[x, y] = (*color, a)
    return out


def points(rows, x=0, y=0):
    return {(x + dx, y + dy) for dy, row in enumerate(rows)
            for dx, mark in enumerate(row) if mark != "."}


def mire_leech():
    c = Canvas()
    body = points(("..#####...", ".########.", "##########",
                   ".########.", "..######.."), 3, 6)
    c.contour(body, OUTLINE)
    c.paint(body, LEECH[1])
    c.shadow({p for p in body if p[1] >= 9 or p[0] >= 10}, LEECH[0])
    c.highlight({(5, 7), (6, 7), (7, 7), (8, 8)}, LEECH[2])
    c.material(body, LEECH[0], LEECH[2], 101, 0.12)
    c.cluster(4, 8, ("#",), (242, 214, 130))
    c.cluster(11, 8, ("#",), (242, 214, 130))
    return c.image(False)


def ricochet_sling():
    c = Canvas()
    shape = points(("#.......#", ".#.....#.", "..#...#..", "...#.#...",
                    "....#....", "....#....", "....#....", "....#...."), 3, 2)
    pouch = points((".###.", "#####", ".###."), 6, 5)
    whole = shape | pouch
    c.contour(whole, OUTLINE)
    c.paint(shape, WOOD[1])
    c.shadow({p for p in shape if p[0] > 7}, WOOD[0])
    c.highlight({p for p in shape if p[0] < 7 and p[1] < 8}, WOOD[2])
    c.paint(pouch, LEATHER[1])
    c.shadow({p for p in pouch if p[1] == 7}, LEATHER[0])
    c.highlight({(7, 5), (8, 5)}, LEATHER[2])
    c.material(pouch, LEATHER[0], LEATHER[2], 102, 0.18)
    return c.image(False)


def add_expansion(atlas):
    out = extend(atlas)
    for i in range(10):
        place_cell(out, ACTORS, i, monster(i))
    for i in range(4):
        place_cell(out, ACTORS, 10 + i, hero(i))
    for i in range(8):
        place_cell(out, ACTORS, 14 + i, weapon(i))
        place_cell(out, ACTORS, 22 + i, relic(i))
    for i in range(4):
        place_cell(out, ACTORS, 30 + i, shrine_symbol(i))
    for biome in range(10):
        for variant in range(3):
            place_cell(out, BANDS[7], biome * 3 + variant, biome_prop(biome, variant))
        place_cell(out, BANDS[7], 30 + biome, biome_marker(biome))
        for cell in range(12):
            art = ground_fill(biome, cell) if cell < 3 else ground_edge(out, biome, cell)
            place_cell(out, BANDS[8], biome * 12 + cell, art)
    errors = reference_errors(out)
    errors += expansion_errors(out)
    if errors:
        raise RuntimeError("invalid reference art:\n" + "\n".join(errors))
    return out


def sprite(atlas, x, y):
    return atlas.crop((x, y, x + 16, y + 16))


def tree(atlas, index):
    top = sprite(atlas, index * 18 + 1, 481)
    foot = sprite(atlas, index * 18 + 1, 499)
    out = Image.new("RGBA", (16, 32))
    out.alpha_composite(top)
    out.alpha_composite(foot, (0, 16))
    return out


def art_errors(art, name):
    px = art.load()
    solid = {(x, y) for y in range(art.height) for x in range(art.width)
             if px[x, y][3]}
    if not solid:
        return [f"{name} is empty"]
    seen = set()
    todo = [next(iter(solid))]
    while todo:
        p = todo.pop()
        if p in seen:
            continue
        seen.add(p)
        x, y = p
        todo.extend(n for n in ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1))
                    if n in solid and n not in seen)
    errors = []
    if seen != solid:
        errors.append(f"{name} has {len(solid - seen)} disconnected pixels")
    outlines = {OUTLINE, KIT_OUTLINE}
    for x, y in solid:
        neighbours = ((x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1))
        exposed = any(0 <= nx < art.width and 0 <= ny < art.height and
                      (nx, ny) not in solid for nx, ny in neighbours)
        if exposed and px[x, y][:3] not in outlines:
            errors.append(f"{name} has an unoutlined edge at {x},{y}")
            break
    return errors


def reference_errors(atlas):
    checks = ((tree(atlas, 0), "birch"),
              (sprite(atlas, 23 * 18 + 1, 481), "boulder"),
              (mire_leech(), "mire leech"),
              (ricochet_sling(), "ricochet sling"))
    return [error for art, name in checks for error in art_errors(art, name)]


def expansion_errors(atlas):
    names = MONSTERS + HEROES + WEAPONS + RELICS + tuple(f"shrine {i}" for i in range(4))
    checks = [(sprite(atlas, i % 24 * 18 + 1,
                      ACTORS.y + i // 24 * 18 + 1), name)
              for i, name in enumerate(names)]
    for biome, name in enumerate(BIOMES):
        for variant in range(4):
            index = biome * 3 + variant if variant < 3 else 30 + biome
            checks.append((sprite(atlas, index % 24 * 18 + 1,
                                  BANDS[7].y + index // 24 * 18 + 1),
                           f"{name} prop {variant}"))
    return [error for art, name in checks for error in art_errors(art, name)]


def legacy_birch():
    c = Canvas(16, 32)
    for y in range(13, 32):
        c.put(7, y, PALE[2])
        c.put(8, y, PALE[1])
        c.put(9, y, PALE[0])
        if y % 4 == 1:
            c.put(7 + y % 3, y, CHAR[0])
    c.ellipse(8, 9, 6.5, 7, LEAF, dither=0.18, seed=1)
    c.ellipse(4.5, 14, 3, 3, LEAF, seed=2)
    c.ellipse(11.5, 13, 3, 3.5, LEAF, seed=3)
    return c.image()


def legacy_boulder():
    c = Canvas()
    c.ellipse(8, 11, 6.5, 4.8, STONE, dither=0.15, seed=13)
    return c.image()


def contact(after, path, zoom):
    empty = Image.new("RGBA", (16, 16))
    samples = (
        ("BIRCH", legacy_birch(), tree(after, 0)),
        ("BOULDER", legacy_boulder(), sprite(after, 23 * 18 + 1, 481)),
        ("LEECH", empty, mire_leech()),
        ("SLING", empty, ricochet_sling()),
    )
    cell_w, header, row_h = 48, 12, 38
    sheet = Image.new("RGBA", (len(samples) * cell_w, header + row_h * 4),
                      (28, 24, 34, 255))
    draw = ImageDraw.Draw(sheet)
    for col, (name, old, new) in enumerate(samples):
        x = col * cell_w
        draw.text((x + 2, 1), name, fill=(246, 224, 150, 255))
        rows = ((old, (220, 214, 188, 255)),
                (new, (220, 214, 188, 255)),
                (old, (38, 42, 54, 255)),
                (new, (38, 42, 54, 255)))
        for row, (art, bg) in enumerate(rows):
            y = header + row * row_h
            sheet.paste(Image.new("RGBA", (16, 32), bg), (x + 30, y))
            sheet.alpha_composite(art, (x + 30, y + 32 - art.height))
            label = "OLD" if row % 2 == 0 else "NEW"
            draw.text((x + 2, y + 4), label, fill=(190, 194, 208, 255))
    if zoom != 1:
        sheet = sheet.resize((sheet.width * zoom, sheet.height * zoom),
                             Image.Resampling.NEAREST)
    sheet.save(path)


def catalogue(atlas, path, zoom=1):
    groups = ((MONSTERS, 0), (HEROES + WEAPONS[:6], 10),
              (WEAPONS[6:] + RELICS, 20))
    sheet = Image.new("RGBA", (960, 144 + 10 * 32), (28, 24, 34, 255))
    draw = ImageDraw.Draw(sheet)
    for row, (names, first) in enumerate(groups):
        for col, name in enumerate(names):
            x, y = col * 96, row * 48
            draw.text((x + 2, y + 1), name, fill=(244, 224, 158))
            art = sprite(atlas, (first + col) % 24 * 18 + 1,
                         ACTORS.y + (first + col) // 24 * 18 + 1)
            for off, bg in ((14, (220, 214, 188, 255)), (38, (38, 42, 54, 255))):
                sheet.paste(Image.new("RGBA", (16, 16), bg), (x + off, y + 17))
                sheet.alpha_composite(art, (x + off, y + 17))
    for biome, name in enumerate(BIOMES):
        y = 144 + biome * 32
        draw.text((2, y + 9), name, fill=(*ACCENTS[biome], 255))
        for i in range(4):
            index = biome * 3 + i if i < 3 else 30 + biome
            x0 = index % 24 * 18 + 1
            y0 = BANDS[7].y + index // 24 * 18 + 1
            sheet.alpha_composite(sprite(atlas, x0, y0), (114 + i * 22, y + 7))
        for cell in range(12):
            index = biome * 12 + cell
            x0 = index % 24 * 18 + 1
            y0 = BANDS[8].y + index // 24 * 18 + 1
            sheet.alpha_composite(sprite(atlas, x0, y0), (212 + cell * 18, y + 7))
    if zoom != 1:
        sheet = sheet.resize((sheet.width * zoom, sheet.height * zoom),
                             Image.Resampling.NEAREST)
    sheet.save(path)


def main():
    root = Path(__file__).resolve().parent.parent
    atlas_path = root / "res" / "atlas.png"
    from make_frontier import add_frontier
    from make_grounds import add_grounds
    from make_rims import add_rims

    before = Image.open(atlas_path).convert("RGBA")
    after = add_rims(add_expansion(add_frontier(add_grounds(add_sprites(before)))))
    errors = validate(after)
    if errors:
        raise RuntimeError("invalid atlas layout:\n" + "\n".join(errors))
    after.save(atlas_path)
    if "--preview" in sys.argv:
        at = sys.argv.index("--preview")
        out = Path(sys.argv[at + 1])
        contact(after, out, 1)
        contact(after, out.with_name(out.stem + "-4x" + out.suffix), 4)
    if "--catalogue" in sys.argv:
        at = sys.argv.index("--catalogue")
        out = Path(sys.argv[at + 1])
        catalogue(after, out)
        catalogue(after, out.with_name(out.stem + "-4x" + out.suffix), 4)
    print(f"wrote {atlas_path}")


if __name__ == "__main__":
    main()
