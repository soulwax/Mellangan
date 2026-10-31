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


WEAPONS = (ember_lance, hoarfrost_fan, arcane_orbit, starfall, void_rift,
           seeker_runes, flame_ring)
SKILLS = (time_warp, mana_surge, arcane_missiles, singularity)


def icons():
    out = [(i, f()) for i, f in enumerate(WEAPONS)]
    start = ARSENAL["skills"].start
    out += [(start + i, f()) for i, f in enumerate(SKILLS)]
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
