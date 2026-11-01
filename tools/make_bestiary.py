"""Paints the bestiary band into res/atlas.png: the foes added after the expansion, one 16 x 16 cell each.

Band at y=1380, 24 columns by 6 rows (144 cells). Cell i is atlas::beast(i); the order below is the
`atlas::bs` order in src/Atlas.hpp. Sprites are drawn as character maps: each letter is a palette entry,
'.' is empty, and the Canvas adds the 1px Kenney outline.

     0  Thornling      a walking bramble knot with red eyes
     1  Wild Boar      a bristled boar with ivory tusks
     2  Goblin Archer  a green goblin in a leather hood with a short bow
     3  Goblin Shaman  a goblin in a red robe with a feathered staff
     4  Sapling        a young walking tree
     5  Treant         the sapling grown: a broad oak with a face in the bark
     6  Moth Wisp      a pale moth with a glowing body
     7  Cutthroat      a hooded bandit with a knife

Usage: python tools/make_bestiary.py [--preview out.png]   (make_assets.py calls add_bestiary too)
"""
import sys
from pathlib import Path

from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))

from make_grounds import CELL
from sprite_kit import Canvas

BAND_Y = 1380
ROWS = 6

LEAF = {"g": (40, 92, 50), "G": (70, 140, 66), "L": (122, 190, 88)}
BARK = {"b": (78, 52, 40), "B": (114, 78, 54), "W": (156, 110, 76)}
HIDE = {"h": (82, 54, 40), "H": (126, 86, 58), "J": (168, 124, 86)}
GOBLIN = {"k": (62, 110, 52), "K": (96, 154, 70), "Y": (150, 200, 96)}
CLOTH = {"c": (110, 34, 40), "C": (164, 56, 58), "D": (210, 96, 84)}
DUSK = {"s": (38, 36, 48), "S": (64, 62, 80), "U": (100, 98, 120)}
MOTH = {"m": (122, 104, 150), "M": (178, 160, 206), "N": (232, 222, 246)}
EXTRA = {
    "t": (210, 190, 120),
    "R": (220, 60, 50),
    "T": (246, 238, 214),
    "E": (24, 18, 22),
    "O": (255, 236, 150),
    "F": (232, 214, 160),
    "X": (200, 206, 214),
    "Q": (255, 250, 220),
}


def draw(rows, *palettes):
    pal = {}
    for p in palettes + (EXTRA,):
        pal.update(p)
    assert len(rows) == 16 and all(len(r) == 16 for r in rows), rows
    c = Canvas()
    for y, row in enumerate(rows):
        for x, k in enumerate(row):
            if k != ".":
                c.put(x, y, pal[k])
    return c.image(True)


def thornling():
    return draw((
        "................",
        "................",
        "................",
        "................",
        "......g..g......",
        ".....gGg.Gg.....",
        "....gGGLGGLg....",
        "...tgGRGGRGgt...",
        "...gGGGGGGGGg...",
        "...gGLGGGLGGg...",
        "....gGGGGGGg....",
        "...t.gGGGGg.t...",
        ".....g....g.....",
        ".....b....b.....",
        "....bb....bb....",
        "................",
    ), LEAF, BARK)


def boar():
    return draw((
        "................",
        "................",
        "................",
        "................",
        "......hhhhh.....",
        "....hhHHHHHhh...",
        "...hHHJJJHHHHh..",
        "..hHHJJHHHHHHHh.",
        ".ThEHHHHHHHHHHh.",
        "TThHHHHHHHHHHhh.",
        ".hhHHHHHHHHHHh..",
        "..hhHHHHHHHHh...",
        "...hh.hh.hh.hh..",
        "...hh.hh.hh.hh..",
        "...bb.bb.bb.bb..",
        "................",
    ), HIDE, BARK)


def goblin_archer():
    return draw((
        "................",
        "................",
        "......hhh.......",
        ".....hHHHh...b..",
        "....hHKKKHh..Wb.",
        "...kKEKKEKk..W.b",
        "....kKKKKk...W.b",
        ".....hHHh...W..b",
        "....hHHHHhXXXXXb",
        "....hHJHHh..W..b",
        "....kHHHHk...W.b",
        ".....hHHh....W.b",
        ".....h..h....Wb.",
        ".....k..k....b..",
        "....kk..kk......",
        "................",
    ), HIDE, GOBLIN, BARK)


def goblin_shaman():
    return draw((
        "................",
        ".............FR.",
        "......kkk...FRF.",
        ".....kKKKk...W..",
        "....kKYYYKk..W..",
        "...kKEKKEKk..W..",
        "....kKKKKk...W..",
        ".....cCCc...KW..",
        "....cCCCCcKK.W..",
        "....cCDCCc...W..",
        "...cCCCCCCc..W..",
        "...cCCDCCCc..W..",
        "..cCCCCCCCCc.W..",
        "..ccccccccc..W..",
        "....kk..kk...b..",
        "................",
    ), GOBLIN, CLOTH, BARK)


def sapling():
    return draw((
        "................",
        "................",
        "................",
        "......gg.g......",
        ".....gGLgGg.....",
        "....gGLGGLGg....",
        "....gGGGGGGg....",
        ".....gGgGgg.....",
        "......bEbE......",
        "......BBBB......",
        ".....bBWBBb.....",
        "....b.BBBB.b....",
        "......BWBB......",
        "......B..B......",
        ".....bb..bb.....",
        "................",
    ), LEAF, BARK)


def treant():
    return draw((
        "...gg.gLg.gg....",
        "..gGLgGGLgGLg...",
        ".gGGLGGLGGGLGg..",
        ".gGLGGGGGLGGGg..",
        "..gGGgGGgGGGg...",
        "...ggbBBBbgg....",
        "....bBEBBEBb....",
        "...bBBWBBWBBb...",
        "..bBBBEEEEBBBb..",
        ".bWbBBBBBBBBbWb.",
        "bW.bBBWBBBWBb.Wb",
        "...bBBBBBBBBb...",
        "...bBWBBBBWBb...",
        "...bBBb..bBBb...",
        "..bbBb....bBbb..",
        "................",
    ), LEAF, BARK)


def moth():
    return draw((
        "................",
        "................",
        "....F......F....",
        ".....F....F.....",
        "..mm..F..F..mm..",
        ".mMMm.OOO.mMMm..",
        "mMNMMmOQOmMMNMm.",
        "mMMNMmOQOmMNMMm.",
        ".mMMMmOOOmMMMm..",
        "..mMm.OOO.mMm...",
        "...mMmOOOmMm....",
        "....mm.O.mm.....",
        ".......O........",
        "................",
        "................",
        "................",
    ), MOTH)


def cutthroat():
    return draw((
        "................",
        "................",
        "......sss.......",
        ".....sSSSs......",
        "....sSUUUSs.....",
        "....sSERESs.....",
        "....sSSSSSs.....",
        ".....sSSSs..X...",
        "....sSSSSSsXX...",
        "...sSUSSSSSX....",
        "...sSSSSSSsh....",
        "...sSSUSSSs.....",
        "...sSSSSSSs.....",
        "....sSs.sSs.....",
        "....bb...bb.....",
        "................",
    ), DUSK, BARK, HIDE)


PAINTERS = (thornling, boar, goblin_archer, goblin_shaman, sapling, treant, moth, cutthroat)


def slot(i):
    return (i % 24) * CELL + 1, BAND_Y + (i // 24) * CELL + 1


def add_bestiary(atlas):
    out = Image.new("RGBA", (atlas.width, max(atlas.height, BAND_Y + ROWS * CELL)), (0, 0, 0, 0))
    out.paste(atlas.crop((0, 0, atlas.width, min(atlas.height, BAND_Y))), (0, 0))
    for i, paint in enumerate(PAINTERS):
        out.paste(paint(), slot(i))
    return out


def preview(atlas, path, zoom=6):
    img = Image.new("RGBA", (len(PAINTERS) * 20 + 4, 24), (70, 120, 64, 255))
    for i in range(len(PAINTERS)):
        x, y = slot(i)
        img.alpha_composite(atlas.crop((x, y, x + 16, y + 16)), (4 + i * 20, 4))
    img.resize((img.width * zoom, img.height * zoom), Image.Resampling.NEAREST).save(path)


if __name__ == "__main__":
    root = Path(__file__).resolve().parent.parent / "res" / "atlas.png"
    atlas = add_bestiary(Image.open(root).convert("RGBA"))
    if len(sys.argv) > 2 and sys.argv[1] == "--preview":
        preview(atlas, sys.argv[2])
        print(f"wrote {sys.argv[2]}")
    else:
        atlas.save(root)
        print(f"wrote {root} ({atlas.width}x{atlas.height})")
