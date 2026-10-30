"""Paints the ability effect icons into res/particles.png (an 8 x 8 grid of 8 px cells).
Rows 0-3 come from dittany and are left alone; this script owns the cells listed in ICONS and
repaints them from scratch, so it is safe to re-run. Cell n is drawn by Screen::particle(n, ...).

Usage: python tools/make_particles.py [--preview out.png]
"""
import sys
from pathlib import Path

from PIL import Image

PALETTE = {
    "W": (255, 255, 255, 255),
    "L": (205, 230, 255, 255),
    "M": (140, 190, 240, 255),
    "D": (80, 120, 190, 255),
    "H": (176, 146, 116, 255),
    "R": (124, 98, 78, 255),
    "S": (84, 64, 54, 255),
    "K": (52, 36, 38, 255),
    "Y": (255, 242, 170, 255),
    "O": (255, 156, 58, 255),
    "F": (206, 72, 34, 255),
    "G": (255, 255, 255, 150),
    "C": (255, 212, 84, 255),
    "c": (214, 148, 40, 255),
    "d": (122, 76, 30, 255),
    "A": (150, 222, 96, 255),
    "a": (70, 160, 60, 255),
    "P": (255, 176, 206, 255),
    "p": (214, 104, 150, 255),
    "l": (96, 170, 64, 255),
    "g": (52, 104, 44, 255),
}

ICONS = {
    32: [  # ice shard
        ".......W",
        "......WL",
        ".....WLM",
        "....LLM.",
        "...LMM..",
        "..MMD...",
        ".MDD....",
        ".D......",
    ],
    33: [  # snowflake
        "...W....",
        ".W.W.W..",
        "..WLW...",
        "WWLWLWW.",
        "..WLW...",
        ".W.W.W..",
        "...W....",
        "........",
    ],
    34: [  # frost glint
        "........",
        "...W....",
        "...L....",
        ".WLWLW..",
        "...L....",
        "...W....",
        "........",
        "........",
    ],
    35: [  # rock chunk
        "........",
        "..KKK...",
        ".KHHRK..",
        "KHHRRSK.",
        "KRRRSSK.",
        ".KSSSK..",
        "..KKK...",
        "........",
    ],
    36: [  # pebble
        "........",
        "........",
        "...KK...",
        "..KHRK..",
        "..KRSK..",
        "...KK...",
        "........",
        "........",
    ],
    37: [  # meteor core
        "..KKKK..",
        ".KOOOFK.",
        "KOYYOOFK",
        "KOYYOOFK",
        "KOOOOFFK",
        "KFOOFFSK",
        ".KFFFSK.",
        "..KKKK..",
    ],
    39: [  # arcane star
        "...W....",
        "...W....",
        ".G.L.G..",
        "WWLWLWW.",
        ".G.L.G..",
        "...W....",
        "...W....",
        "........",
    ],
    40: [  # soul wisp
        "....G...",
        "...GL...",
        "..GLW...",
        "..LWWG..",
        ".GWWWL..",
        ".LWWWL..",
        "..LWL...",
        "...G....",
    ],
    41: [  # rune glyph
        "..WWW...",
        ".W...W..",
        "W..W..W.",
        "W.W.W.W.",
        "WWWWWWW.",
        ".W...W..",
        "..WWW...",
        "........",
    ],
    42: [  # holy glint
        "G..W..G.",
        ".G.W.G..",
        "..LWL...",
        "WWWWWWW.",
        "..LWL...",
        ".G.W.G..",
        "G..W..G.",
        "........",
    ],
    43: [  # gold coin
        "........",
        "..dddd..",
        ".dCWCcd.",
        ".dWCCcd.",
        ".dCCccd.",
        ".dCcccd.",
        "..dddd..",
        "........",
    ],
    44: [  # acid bubble
        "........",
        "..aaa...",
        ".aAWAa..",
        ".aWAAa..",
        ".aAAAa..",
        "..aaa...",
        "........",
        "........",
    ],
    45: [  # petal
        "........",
        "....PP..",
        "...PpPP.",
        "..PpPP..",
        ".PpPP...",
        ".pPP....",
        "..p.....",
        "........",
    ],
    46: [  # leaf
        "........",
        "......Al",
        "....AAlg",
        "...AAlg.",
        "..AAlg..",
        ".Algg...",
        ".lg.....",
        "g.......",
    ],
    47: [  # feather
        ".......W",
        "......WL",
        ".....WLW",
        "....WLW.",
        "...WLW..",
        "..WLW...",
        ".LW.....",
        "L.......",
    ],
    48: [  # smoke puff
        "........",
        "..GGG...",
        ".GWWWG..",
        "GWWLWWG.",
        "GWLLLWG.",
        ".GWWWG..",
        "..GGG...",
        "........",
    ],
    49: [  # crescent
        "..WWW...",
        ".WW.....",
        "WW......",
        "WL......",
        "WW......",
        ".WW.....",
        "..WWW...",
        "........",
    ],
    50: [  # bone
        "........",
        ".WW.....",
        ".WLW....",
        "..WLW...",
        "...WLW..",
        "....WLW.",
        ".....WW.",
        "........",
    ],
    51: [  # maple leaf
        "...F....",
        ".F.OF.F.",
        ".FOOOOF.",
        "FOOCOOOF",
        ".FOOCOF.",
        "..FOdF..",
        "....d...",
        "...d....",
    ],
    52: [  # raindrop
        "......G.",
        ".....L..",
        "....L...",
        "...L....",
        "..W.....",
        "........",
        "........",
        "........",
    ],
    53: [  # gust
        "........",
        "........",
        "..GGWW..",
        "GWWWWWWG",
        "....GWWG",
        "........",
        "........",
        "........",
    ],
    54: [  # seed fluff
        "..W.W...",
        ".W.W.W..",
        "..WLW...",
        "...L....",
        "...G....",
        "...G....",
        "........",
        "........",
    ],
    55: [  # falling star
        "G.......",
        ".G......",
        "..GL....",
        "...LW...",
        "....WYW.",
        ".....WY.",
        "......W.",
        "........",
    ],
    56: [  # butterfly
        "........",
        "WW...WW.",
        "WLW.WLW.",
        ".WWKWW..",
        ".WLKLW..",
        "..W.W...",
        "........",
        "........",
    ],
    57: [  # spore
        "........",
        "...G....",
        "..GWG...",
        ".GWLWG..",
        "..GWG...",
        "...G....",
        "........",
        "........",
    ],
    38: [  # ember flake
        "........",
        "........",
        "...G....",
        "..GYG...",
        "...OG...",
        "....F...",
        "........",
        "........",
    ],
}


def paint(sheet: Image.Image):
    for cell, rows in ICONS.items():
        ox, oy = cell % 8 * 8, cell // 8 * 8
        for y in range(8):
            for x in range(8):
                sheet.putpixel((ox + x, oy + y), PALETTE.get(rows[y][x], (0, 0, 0, 0)))


def main():
    path = Path(__file__).resolve().parent.parent / "res" / "particles.png"
    sheet = Image.open(path).convert("RGBA")
    paint(sheet)
    if len(sys.argv) == 3 and sys.argv[1] == "--preview":
        sheet.resize((512, 512), Image.NEAREST).save(sys.argv[2])
        return
    sheet.save(path)
    print(f"painted {len(ICONS)} icons into {path.name}")


if __name__ == "__main__":
    main()
