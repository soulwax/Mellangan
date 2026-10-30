"""Builds res/weather.png (256 x 128) from the NS1 weather VFX packs.

The NS1 textures are smooth 512 px particle maps made for 3D engines. This script reads them straight out of their
.unitypackage files, shrinks them, and quantises their alpha into a few bands with an ordered Bayer dither, so they
match the game's pixel look. Ripples and raindrops are drawn procedurally.

Layout (x, y, w, h), mirrored by atlas::wx in src/Atlas.hpp:
    cloud shadow   0,   0, 96, 96   from NS1_Fog T_Cloud
    fog bank A    96,   0, 64, 64   from NS1_Fog T_Cloud
    fog bank B   160,   0, 64, 64   from NS1_Snow T_Snow (soft blob)
    ripples        0,  96,  8,  8   x4 frames
    splash        32,  96,  8,  8   x2 frames, from NS1_RainLight T_WaterSplash
    flakes        48,  96,  4,  4   x4, drawn by hand
    raindrop      64,  96,  1,  6
    ash flake     72,  96,  3,  3   x2
    puddles        0, 104, 16,  8   and 16, 104, 12, 6, edged by a highlight
    drifts        32, 104, 16,  8   and 48, 104, 12, 6

Usage: python tools/make_weather.py "<Assets folder holding the NS1_* folders>" [--preview out.png]
"""
import io
import sys
import tarfile
from pathlib import Path

from PIL import Image

BAYER = [[0, 8, 2, 10], [12, 4, 14, 6], [3, 11, 1, 9], [15, 7, 13, 5]]


def texture(assets: Path, pack: str, name: str) -> Image.Image:
    package = next(assets.glob(f"NS1_{pack}_*/NS1_{pack}/Unity/{pack}.unitypackage"))
    with tarfile.open(package, "r:gz") as tar:
        members = {m.name: m for m in tar.getmembers()}
        for key, member in members.items():
            if not key.endswith("/pathname"):
                continue
            path = tar.extractfile(member).read().decode("utf-8").splitlines()[0]
            if path.endswith("/" + name):
                data = tar.extractfile(members[key[: -len("pathname")] + "asset"]).read()
                return Image.open(io.BytesIO(data)).convert("RGBA")
    raise SystemExit(f"{name} not found in {package}")


def coverage(img: Image.Image) -> Image.Image:
    r, g, b, a = img.split()
    lum = Image.merge("RGB", (r, g, b)).convert("L")
    return Image.composite(lum, Image.new("L", img.size, 0), a) if a.getextrema() != (255, 255) else lum


def banded(mask: Image.Image, size: int, levels: int, gain: float = 1.0) -> Image.Image:
    small = mask.resize((size, size), Image.LANCZOS)
    out = Image.new("RGBA", (size, size))
    for y in range(size):
        for x in range(size):
            v = min(1.0, small.getpixel((x, y)) / 255 * gain) * levels
            band = int(v)
            if v - band > (BAYER[y % 4][x % 4] + 0.5) / 16:
                band += 1
            alpha = round(255 * min(band, levels) / levels)
            out.putpixel((x, y), (255, 255, 255, alpha))
    return out


def ripple(frame: int) -> Image.Image:
    img = Image.new("RGBA", (8, 8))
    rx, ry = 1.2 + frame * 0.9, 0.6 + frame * 0.45
    alpha = [255, 220, 160, 90][frame]
    for y in range(8):
        for x in range(8):
            d = ((x - 3.5) / rx) ** 2 + ((y - 3.5) / ry) ** 2
            if 0.55 < d < 1.45:
                img.putpixel((x, y), (255, 255, 255, alpha))
    return img


FLAKES = [
    ["W.W.", ".W..", "W.W.", "...."],
    [".W..", "WWW.", ".W..", "...."],
    ["WW..", "WW..", "....", "...."],
    ["W...", "...", "....", "...."],
]


def flakes(snow: Image.Image) -> list:
    out = []
    for rows in FLAKES:
        img = Image.new("RGBA", (4, 4))
        for y, row in enumerate(rows):
            for x, c in enumerate(row):
                if c == "W":
                    img.putpixel((x, y), (255, 255, 255, 255))
        out.append(img)
    return out


def patch(cloud: Image.Image, w: int, h: int, turn: int, drift: bool) -> Image.Image:
    rough = cloud.rotate(turn).resize((w, h), Image.LANCZOS)
    img = Image.new("RGBA", (w, h))
    for y in range(h):
        for x in range(w):
            d = ((x + 0.5 - w / 2) / (w / 2)) ** 2 + ((y + 0.5 - h / 2) / (h / 2)) ** 2
            v = (1 - d) * 1.6 + rough.getpixel((x, y)) / 255 * 0.6 - 0.45
            if v <= (BAYER[y % 4][x % 4] + 0.5) / 16 * 0.5:
                continue
            if drift:
                alpha = 255 if v > 0.55 else 170
            else:
                alpha = 150
                if y > 0 and img.getpixel((x, y - 1))[3] == 0 and x % 3 != 0:
                    alpha = 255
            img.putpixel((x, y), (255, 255, 255, alpha))
    return img


def build(assets: Path) -> Image.Image:
    cloud = coverage(texture(assets, "Fog", "T_Cloud.png"))
    snow = texture(assets, "Snow", "T_Snow.png")
    splash = coverage(texture(assets, "RainLight", "T_WaterSplash.png"))
    sheet = Image.new("RGBA", (256, 128))
    sheet.alpha_composite(banded(cloud, 96, 3, 1.3), (0, 0))
    sheet.alpha_composite(banded(cloud.rotate(90), 64, 4, 1.1), (96, 0))
    half = snow.size[0] // 2
    sheet.alpha_composite(banded(coverage(snow.crop((0, half, half, snow.size[1]))), 64, 4, 1.2), (160, 0))
    for f in range(4):
        sheet.alpha_composite(ripple(f), (f * 8, 96))
    sheet.alpha_composite(banded(splash, 8, 2, 2.2), (32, 96))
    sheet.alpha_composite(banded(splash, 8, 1, 1.4), (40, 96))
    for i, flake in enumerate(flakes(snow)):
        sheet.alpha_composite(flake, (48 + i * 4, 96))
    for y, a in enumerate([60, 110, 160, 200, 235, 255]):
        sheet.putpixel((64, 96 + y), (255, 255, 255, a))
    for i, rows in enumerate([[".W.", "WWW", ".W."], ["W..", ".WW", ".W."]]):
        for y, row in enumerate(rows):
            for x, c in enumerate(row):
                if c == "W":
                    sheet.putpixel((72 + i * 3 + x, 96 + y), (255, 255, 255, 255))
    sheet.alpha_composite(patch(cloud, 16, 8, 0, False), (0, 104))
    sheet.alpha_composite(patch(cloud, 12, 6, 180, False), (16, 104))
    sheet.alpha_composite(patch(cloud, 16, 8, 90, True), (32, 104))
    sheet.alpha_composite(patch(cloud, 12, 6, 270, True), (48, 104))
    return sheet


def main():
    if len(sys.argv) < 2:
        raise SystemExit(__doc__)
    sheet = build(Path(sys.argv[1]))
    if len(sys.argv) == 4 and sys.argv[2] == "--preview":
        preview = Image.new("RGBA", sheet.size, (40, 44, 56, 255))
        preview.alpha_composite(sheet)
        preview.resize((sheet.width * 4, sheet.height * 4), Image.NEAREST).save(sys.argv[3])
        return
    out = Path(__file__).resolve().parent.parent / "res" / "weather.png"
    sheet.save(out)
    print(f"wrote {out.name}")


if __name__ == "__main__":
    main()
