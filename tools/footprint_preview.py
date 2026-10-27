import json
import sys

from PIL import Image, ImageDraw


def render(atlas, d, zoom):
    w, h = d["size"]
    img = Image.new("RGBA", (w, h), (0, 0, 0, 255))
    for layer in ("ground", "props"):
        for px, py, sx, sy in d[layer]:
            img.alpha_composite(atlas.crop((sx, sy, sx + 16, sy + 16)), (px, py))
    return img.resize((w * zoom, h * zoom), Image.NEAREST)


if __name__ == "__main__":
    atlas = Image.open(sys.argv[1]).convert("RGBA")
    designs = json.load(open(sys.argv[2]))
    zoom = int(sys.argv[4]) if len(sys.argv) > 4 else 3
    pieces = [(d["name"], render(atlas, d, zoom)) for d in designs]
    cols = int(sys.argv[5]) if len(sys.argv) > 5 else 3
    cw, ch = max(p.width for _, p in pieces), max(p.height for _, p in pieces)
    rows = (len(pieces) + cols - 1) // cols
    out = Image.new("RGBA", (cols * (cw + 8) + 8, rows * (ch + 22) + 8), (40, 40, 60, 255))
    draw = ImageDraw.Draw(out)
    for i, (name, p) in enumerate(pieces):
        x, y = 8 + (i % cols) * (cw + 8), 8 + (i // cols) * (ch + 22)
        draw.text((x, y), name, fill=(255, 230, 120, 255))
        out.paste(p, (x, y + 14))
    out.save(sys.argv[3])
