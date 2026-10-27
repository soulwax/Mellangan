import json
import sys

from PIL import Image, ImageDraw

CELL, STEP, TOWN_X = 16, 18, 12 * 18
GRASS = 0


def tile(atlas, index):
    x, y = TOWN_X + (index % 12) * STEP + 1, (index // 12) * STEP + 1
    return atlas.crop((x, y, x + CELL, y + CELL))


def compose(atlas, rows, margin=1):
    w, h = max(len(r) for r in rows) + margin * 2, len(rows) + margin * 2
    img = Image.new("RGBA", (w * CELL, h * CELL))
    for y in range(h):
        for x in range(w):
            img.paste(tile(atlas, GRASS), (x * CELL, y * CELL))
    for y, row in enumerate(rows):
        for x, index in enumerate(row):
            if index is not None:
                img.alpha_composite(tile(atlas, index), ((x + margin) * CELL, (y + margin) * CELL))
    return img


def sheet(atlas, designs, zoom):
    pieces = [(name, compose(atlas, rows)) for name, rows in designs]
    label = 14
    width = sum(p.width * zoom for _, p in pieces) + 8 * (len(pieces) + 1)
    height = max(p.height * zoom for _, p in pieces) + label + 16
    out = Image.new("RGBA", (width, height), (40, 40, 60, 255))
    draw = ImageDraw.Draw(out)
    x = 8
    for name, p in pieces:
        out.paste(p.resize((p.width * zoom, p.height * zoom), Image.NEAREST), (x, label + 8))
        draw.text((x, 4), name, fill=(255, 230, 120, 255))
        x += p.width * zoom + 8
    return out


if __name__ == "__main__":
    atlas = Image.open(sys.argv[1]).convert("RGBA")
    designs = json.load(open(sys.argv[2]))
    zoom = int(sys.argv[4]) if len(sys.argv) > 4 else 4
    sheet(atlas, [(d["name"], d["rows"]) for d in designs], zoom).save(sys.argv[3])
