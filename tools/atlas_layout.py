"""Declares and validates Mellangan's generated atlas layout."""

from dataclasses import dataclass
from pathlib import Path
import sys

from PIL import Image


TILE = 16
CELL = 18
ATLAS_W = 24 * CELL


@dataclass(frozen=True)
class Band:
    name: str
    x: int
    y: int
    cols: int
    rows: int

    @property
    def width(self):
        return self.cols * CELL

    @property
    def height(self):
        return self.rows * CELL


BANDS = (
    Band("base", 0, 0, 24, 11),
    Band("winter", 12 * CELL, 264, 12, 7),
    Band("frontier", 0, 270, 12, 11),
    Band("flora", 12 * CELL, 390, 12, 5),
    Band("kit", 0, 480, 24, 4),
    Band("grounds", 0, 552, 24, 4),
    Band("expansion actors", 0, 624, 24, 2),
    Band("expansion biome props", 0, 660, 24, 2),
    Band("expansion biome grounds", 0, 696, 24, 5),
)

EXPANSION_ACTORS = {
    "monsters": range(0, 10),
    "heroes": range(10, 14),
    "weapons": range(14, 22),
    "relics": range(22, 30),
    "effects": range(30, 48),
}
EXPANSION_PROPS = {
    "biome props": range(0, 30),
    "structures": range(30, 40),
    "spare": range(40, 48),
}
EXPANSION_GROUNDS = {f"biome {i}": range(i * 12, (i + 1) * 12) for i in range(10)}


def required_size():
    return ATLAS_W, max(b.y + b.height for b in BANDS)


def extend(atlas):
    width, height = required_size()
    if atlas.width > width or atlas.height > height:
        raise ValueError(f"atlas is {atlas.width}x{atlas.height}, layout is {width}x{height}")
    if atlas.size == (width, height):
        return atlas
    out = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    out.paste(atlas, (0, 0))
    return out


def cell_position(band, index):
    if not 0 <= index < band.cols * band.rows:
        raise ValueError(f"{band.name} cell {index} is outside its {band.cols}x{band.rows} grid")
    return band.x + index % band.cols * CELL + 1, band.y + index // band.cols * CELL + 1


def place_cell(atlas, band, index, sprite):
    if sprite.size != (TILE, TILE):
        raise ValueError(f"{band.name} cell {index} source is {sprite.width}x{sprite.height}, expected 16x16")
    atlas.paste(sprite, cell_position(band, index))


def _overlap(a, b):
    return (a.x < b.x + b.width and b.x < a.x + a.width and
            a.y < b.y + b.height and b.y < a.y + a.height)


def _allocation_errors(name, groups, cells):
    errors = []
    owners = {}
    for owner, indices in groups.items():
        for index in indices:
            if not 0 <= index < cells:
                errors.append(f"{name} {owner} cell {index} is outside 0..{cells - 1}")
            if index in owners:
                errors.append(f"{name} cell {index} belongs to both {owners[index]} and {owner}")
            owners[index] = owner
    if len(owners) != cells:
        missing = sorted(set(range(cells)) - set(owners))
        errors.append(f"{name} has unallocated cells: {missing}")
    return errors


def validate(atlas):
    errors = []
    width, height = required_size()
    if atlas.size != (width, height):
        errors.append(f"atlas is {atlas.width}x{atlas.height}, expected {width}x{height}")

    for i, a in enumerate(BANDS):
        if a.x < 0 or a.y < 0 or a.x + a.width > width or a.y + a.height > height:
            errors.append(f"{a.name} is clipped by the declared atlas bounds")
        for b in BANDS[i + 1:]:
            if _overlap(a, b):
                errors.append(f"bands overlap: {a.name} and {b.name}")

    errors += _allocation_errors("expansion actors", EXPANSION_ACTORS, 48)
    errors += _allocation_errors("expansion biome props", EXPANSION_PROPS, 48)
    errors += _allocation_errors("expansion biome grounds", EXPANSION_GROUNDS, 120)

    px = atlas.load()
    if px is not None and atlas.size == (width, height):
        for band in BANDS:
            for row in range(band.rows):
                for col in range(band.cols):
                    x = band.x + col * CELL
                    y = band.y + row * CELL
                    border = [(x + n, y) for n in range(CELL)]
                    border += [(x + n, y + CELL - 1) for n in range(CELL)]
                    border += [(x, y + n) for n in range(1, CELL - 1)]
                    border += [(x + CELL - 1, y + n) for n in range(1, CELL - 1)]
                    if any(px[gx, gy][3] for gx, gy in border):
                        errors.append(f"{band.name} cell {row * band.cols + col} has a non-transparent gutter")

    return errors


def main():
    prepare = "--prepare" in sys.argv[1:]
    args = [arg for arg in sys.argv[1:] if arg != "--prepare"]
    path = Path(args[0]) if args else Path(__file__).resolve().parent.parent / "res" / "atlas.png"
    atlas = Image.open(path).convert("RGBA")
    if prepare:
        atlas = extend(atlas)
        atlas.save(path)
    errors = validate(atlas)
    if errors:
        for error in errors:
            print(f"error: {error}")
        raise SystemExit(1)
    print(f"atlas layout valid: {atlas.width}x{atlas.height}, {len(BANDS)} bands")


if __name__ == "__main__":
    main()
