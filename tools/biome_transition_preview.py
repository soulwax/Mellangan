import argparse
import csv
import colorsys
from pathlib import Path

from PIL import Image, ImageDraw


def color(biome):
    return tuple(round(c * 255) for c in colorsys.hsv_to_rgb((biome * 0.61803398875) % 1, 0.62, 0.92))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("csv", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--atlas", type=Path, default=Path("res/atlas.png"))
    args = parser.parse_args()
    sprites, tiles = {}, {}
    with args.csv.open(newline="", encoding="utf-8") as src:
        for row in csv.DictReader(src):
            if row["record"] == "sprite":
                sprites[int(row["x"]), int(row["y"])] = tuple(
                    int(float(row[k])) for k in ("old", "new", "count", "id0")
                )
            elif row["record"] == "tile":
                entries = [(int(row[f"id{i}"]), float(row[f"w{i}"])) for i in range(3)]
                tiles[int(row["x"]), int(row["y"])] = (int(row["old"]), int(row["new"]), entries)
    atlas = Image.open(args.atlas).convert("RGB")
    min_x, max_x = min(x for x, _ in tiles), max(x for x, _ in tiles)
    min_y, max_y = min(y for _, y in tiles), max(y for _, y in tiles)
    width, height = max_x - min_x + 1, max_y - min_y + 1
    old_panel, new_panel = Image.new("RGB", (width, height)), Image.new("RGB", (width, height))
    mixes = []
    for (x, y), (old, new, entries) in tiles.items():
        px, py = x - min_x, y - min_y
        old_panel.putpixel((px, py), color(old))
        new_panel.putpixel((px, py), color(new))
        if min_x + 12 <= x <= max_x - 12 and sum(weight > 0.01 for _, weight in entries) > 1:
            mixes.append((abs(x) + abs(y), x, y))
    mix = min(mixes)[1:] if mixes else None
    scale = 3
    old_panel = old_panel.resize((width * scale, height * scale), Image.Resampling.NEAREST)
    new_panel = new_panel.resize((width * scale, height * scale), Image.Resampling.NEAREST)
    strip = Image.new("RGB", (25 * 16, 16), (25, 23, 31))
    if mix:
        x0, y0 = mix
        for col, x in enumerate(range(x0 - 12, x0 + 13)):
            row = tiles.get((x, y0))
            if not row:
                continue
            _, _, entries = row
            composite, total = None, 0.0
            for biome, weight in entries:
                if weight <= 0:
                    continue
                sx, sy, sw, sh = sprites[biome, 0]
                tile = atlas.crop((sx, sy, sx + sw, sy + sh))
                composite = tile if composite is None else Image.blend(composite, tile, weight / (total + weight))
                total += weight
            if composite:
                strip.paste(composite, (col * 16, 0))
    else:
        strip = Image.new("RGB", (25 * 16, 16), (25, 23, 31))
    canvas = Image.new("RGB", (width * scale * 2 + 32, height * scale + 96), (18, 16, 24))
    draw = ImageDraw.Draw(canvas)
    draw.text((8, 5), "Legacy biome map", fill=(240, 230, 200))
    draw.text((16 + width * scale, 5), "Larger biome map (2.5x scale)", fill=(240, 230, 200))
    canvas.paste(old_panel, (8, 20))
    canvas.paste(new_panel, (16 + width * scale, 20))
    draw.text((8, height * scale + 28), "Actual atlas ground surfaces weighted across a sampled transition", fill=(240, 230, 200))
    canvas.paste(strip.resize((25 * 16 * 2, 32), Image.Resampling.NEAREST), (8, height * scale + 48))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    canvas.save(args.output)


if __name__ == "__main__":
    main()
