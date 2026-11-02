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
     8  Bog Toad       a fat mottled toad with a pale belly
     9  Drowned        a waterlogged corpse in kelp, glowing eyes
    10  Snapjaw        a broad marsh crocodile, jaws open
    11  Strangler Vine a coiled thorny vine with a bulb mouth
    12  Amber Beetle   a beetle with a polished amber shell
    13  Spore Puffer   a round violet puffball with tiny feet
    14  Sunken Priest  a hooded priest in sea-green robes with a coral staff
    15  Yeti           a hulking white ape with a blue-grey face
    16  Ice Wraith     a floating frost-blue hood trailing tatters
    17  Barrow Wight   a gaunt corpse in a rotted shroud, green eyes
    18  Crystal Golem  a violet crystal giant with spiked shoulders (the shard reuses it, scaled)
    19  Moon Hound     a lean silver-blue hound with glowing eyes
    20  Lynx           a tawny spotted cat with black ear tufts
    21  Barrow Archer  a hooded skeleton with a bow and a frost arrow

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
TOAD = {"o": (64, 86, 40), "V": (104, 132, 58), "P": (170, 182, 104), "Z": (214, 206, 150)}
SEA = {"w": (34, 70, 84), "A": (60, 112, 120), "a": (110, 164, 160)}
AMBER = {"y": (120, 64, 20), "Y": (196, 120, 36), "z": (244, 190, 90)}
SPORE = {"v": (84, 52, 104), "I": (140, 96, 170), "i": (200, 160, 226)}
YETI = {"f": (150, 164, 186), "u": (206, 216, 230), "n": (246, 250, 255), "p": (96, 110, 140)}
WRAITH = {"f": (40, 60, 100), "u": (110, 160, 210), "n": (200, 232, 250)}
WIGHT = {"k": (44, 54, 50), "K": (78, 96, 84), "Y": (118, 140, 120), "e": (140, 255, 160)}
CRYSTAL = {"v": (78, 56, 138), "I": (146, 116, 214), "i": (206, 186, 250), "n": (250, 246, 255)}
HOUND = {"s": (48, 52, 82), "S": (98, 106, 146), "U": (170, 180, 216)}
LYNX = {"h": (130, 86, 44), "H": (196, 146, 78), "J": (236, 206, 150)}
BONE = {"k": (40, 40, 52), "K": (70, 70, 90), "e": (150, 230, 255), "n": (200, 236, 250)}
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


def bog_toad():
    return draw((
        "................",
        "................",
        "................",
        "................",
        "................",
        "....oo....oo....",
        "...oVEo..oEVo...",
        "..oVVVVooVVVVo..",
        ".oVVPVVVVVVPVVo.",
        ".oVVVVVVVVVVVVo.",
        ".oVZZZZZZZZZZVo.",
        "..oZZZZZZZZZZo..",
        ".ooVoZZZZZZoVoo.",
        "oVVo.oooooo.oVVo",
        "ooo..........ooo",
        "................",
    ), TOAD)


def drowned():
    return draw((
        "................",
        "......www.......",
        ".....wAAAw......",
        "....wAOAOAw.....",
        "....wAAAAAw.....",
        ".....waaaw......",
        "...gwAAAAAwg....",
        "..gwAAaAAAAwg...",
        "..w.wAAAAAw.w...",
        "..a.wAAaAAw.a...",
        "....wAAAAAw.....",
        "....gwAAAwg.....",
        ".....wA.Aw......",
        ".....wA.Aw......",
        "....ww...ww.....",
        "................",
    ), SEA, LEAF)


def snapjaw():
    return draw((
        "................",
        "................",
        "................",
        "................",
        "................",
        "..TT............",
        ".oVVoo..........",
        "oVEVVVoooooo....",
        "oVVVVVVVVVVVoo..",
        ".TTVPVVVPVVVVVo.",
        "..oVVVVVVVVVVVVo",
        "...oVVVVVVVVVVo.",
        "...oo.oo..oo.oo.",
        "...oo.oo..oo.oo.",
        "................",
        "................",
    ), TOAD)


def strangler_vine():
    return draw((
        "................",
        "......gGGg......",
        ".....gGRRGg.....",
        "....gGRTTRGg....",
        "....gGRRRRGg....",
        ".....gGGGGg.....",
        "......gGgt......",
        ".....tgGg.......",
        "......gGg.......",
        ".......gGgt.....",
        "......tgGg......",
        ".....gGGgg......",
        "....gGgGGGg.....",
        "...gGg.ggGGg....",
        "..ggb...bbgg....",
        "................",
    ), LEAF, BARK)


def amber_beetle():
    return draw((
        "................",
        "................",
        "................",
        "....s......s....",
        ".....s....s.....",
        "......ssss......",
        ".....sEssEs.....",
        "....yyyyyyyy....",
        "...yYYzYYzYYy...",
        "..yYzzYYYYzzYy..",
        "..yYYYYyyYYYYy..",
        "..yYYYYyyYYYYy..",
        "...yYYYyyYYYy...",
        "..s.yyyyyyyy.s..",
        ".s..s......s..s.",
        "................",
    ), AMBER, DUSK)


def spore_puffer():
    return draw((
        "................",
        "................",
        "................",
        "................",
        ".......i........",
        ".....vvvvv..i...",
        "....vIIiIIv.....",
        "...vIiIIIIIv....",
        "..ivIIEIIEIv....",
        "...vIIIIIIIv.i..",
        "...vIiIIIiIv....",
        "....vIIIIIv.....",
        ".....vvvvv......",
        "......b.b.......",
        ".....bb.bb......",
        "................",
    ), SPORE, BARK)


def sunken_priest():
    return draw((
        "...........R....",
        "......www..RR...",
        ".....wAAAw.R....",
        "....wAEAEAw.W...",
        "....wAAAAAw.W...",
        ".....waaaw..W...",
        "....wAAAAAwAW...",
        "...wAAaAAAAwW...",
        "...wAAAaAAAw.W..",
        "...wAAAAAAAw.W..",
        "..wAAaAAAAAAwW..",
        "..wAAAAAaAAAw.W.",
        "..wAAAAAAAAAw.W.",
        "..wwwwwwwwwww.b.",
        "....ww...ww.....",
        "................",
    ), SEA, BARK)


def yeti():
    return draw((
        "................",
        ".....uuuuuu.....",
        "....unnnnnnu....",
        "....nppppppn....",
        "...unpEppEpnu...",
        "...unppppppnu...",
        "..uunnpTTpnnuu..",
        ".unnnnnnnnnnnnu.",
        "unnnnnnnnnnnnnnu",
        "unnuunnnnnnuunnu",
        "unu.unnnnnnu.unu",
        "pnu.unnnnnnu.unp",
        "pp..uunnnnuu..pp",
        "....unnu.unnu...",
        "...uffu..uffu...",
        "................",
    ), YETI)


def ice_wraith():
    return draw((
        "................",
        "......uuuu......",
        ".....unnnnu.....",
        "....unffffnu....",
        "....ufQffQfu....",
        "....uffffffu....",
        "...unuffffunu...",
        "..un.unnnnu.nu..",
        ".un..unnnnu..nu.",
        ".u...unnnnu...u.",
        ".....uunnuu.....",
        "......unnu......",
        ".....u.uu.u.....",
        "......u..u......",
        "................",
        "................",
    ), WRAITH)


def barrow_wight():
    return draw((
        "................",
        "......kkkk......",
        ".....kKKKKk.....",
        "....kKXXXXKk....",
        "....kXeXXeXk....",
        "....kXXXXXXk....",
        ".....kXTTXk.....",
        "...kkKKKKKKkk...",
        "..kKKYKKKKYKKk..",
        ".XkKKKYKKYKKKkX.",
        "XX.kKKKKKKKKk.XX",
        "...kKKYKKYKKk...",
        "...kKKKKKKKKk...",
        "..kKKKYKKYKKKk..",
        "..kkk.kkkk.kkk..",
        "................",
    ), WIGHT)


def crystal_golem():
    return draw((
        "................",
        "...n........n...",
        "..nI..iiii..In..",
        "..vI.iIIIIi.Iv..",
        "..vIiIOIIOIiIv..",
        ".vIIiIIIIIIiIIv.",
        ".vIiiiIIIIiiiIv.",
        "vIIv.iIIIIi.vIIv",
        "vIv..ivIIvi..vIv",
        ".v...iIIIIi...v.",
        "....iIIvvIIi....",
        "....iIv..vIi....",
        "...vIIv..vIIv...",
        "...vvvv..vvvv...",
        "................",
        "................",
    ), CRYSTAL)


def moon_hound():
    return draw((
        "................",
        "................",
        "................",
        "...s.s..........",
        "..sUsUs.........",
        ".sSSSSSs........",
        "sSOSSSSSs.....s.",
        "EsSSSSUUSsssssUs",
        ".ssSSUUUUUUUSSs.",
        "...sSSSSSSSSSs..",
        "...sSs.....sSs..",
        "...sS.......sS..",
        "...sS.......sS..",
        "..ss.......ss...",
        "................",
        "................",
    ), HOUND)


def lynx():
    return draw((
        "................",
        "................",
        "................",
        "..E.E...........",
        "..hHhH..........",
        ".hHJHHh.........",
        "hHOHHJHh........",
        "EhJJJHHhhhhhhh..",
        ".hhHHHbHHHbHHHh.",
        "..hHHHHHbHHHHHhE",
        "..hJJHHHHHHJJh..",
        "...hH......hH...",
        "...hH......hH...",
        "..hh......hh....",
        "................",
        "................",
    ), LYNX, BARK)


def barrow_archer():
    return draw((
        "................",
        "................",
        "......kkk.......",
        ".....kKKKk...b..",
        "....kKXXXKk..Bb.",
        "....kXeXeXk..B.b",
        ".....XXTXX...B.b",
        "......kXk...B..b",
        "....kKXXXKnnnnnb",
        "....kXKXKXk.B..b",
        "....kKXXXKk..B.b",
        ".....kKKKk...B.b",
        ".....X...X...Bb.",
        ".....X...X...b..",
        "....XX...XX.....",
        "................",
    ), BONE, BARK)


PAINTERS = (thornling, boar, goblin_archer, goblin_shaman, sapling, treant, moth, cutthroat,
            bog_toad, drowned, snapjaw, strangler_vine, amber_beetle, spore_puffer,
            sunken_priest, yeti, ice_wraith, barrow_wight, crystal_golem, moon_hound, lynx,
            barrow_archer)


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
