"""Tiny procedural pixel painter matching Kenney's Tiny Town look: flat 3-tone shading lit from the top left,
a 1px dark outline, no anti-aliasing. Shapes are painted into a layer dict and resolved into an RGBA image."""
import math

from PIL import Image

OUTLINE = (63, 38, 49)
LIGHT = (-0.6, -0.8)


class Canvas:
    def __init__(self, w=16, h=16):
        self.w, self.h = w, h
        self.px = {}

    def put(self, x, y, c):
        x, y = int(round(x)), int(round(y))
        if 0 <= x < self.w and 0 <= y < self.h:
            self.px[(x, y)] = c

    def paint(self, points, c):
        for x, y in points:
            self.put(x, y, c)

    def cluster(self, x, y, rows, c):
        points = set()
        for dy, row in enumerate(rows):
            for dx, mark in enumerate(row):
                if mark != ".":
                    points.add((x + dx, y + dy))
        self.paint(points, c)
        return points

    def contour(self, points, c=OUTLINE):
        points = set(points)
        edge = set()
        for x, y in points:
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                n = x + dx, y + dy
                if n not in points:
                    edge.add(n)
        self.paint(edge, c)
        return edge

    def shadow(self, points, c):
        self.paint(points, c)

    def highlight(self, points, c):
        self.paint(points, c)

    def material(self, points, dark, light, seed, density=0.2):
        for x, y in sorted(set(points)):
            roll = _hash(x, y, seed)
            if roll < density * 0.45:
                self.put(x, y, light)
            elif roll > 1 - density * 0.55:
                self.put(x, y, dark)

    def ellipse(self, cx, cy, rx, ry, pal, light=LIGHT, dither=0.0, seed=0):
        for y in range(int(cy - ry) - 1, int(cy + ry) + 2):
            for x in range(int(cx - rx) - 1, int(cx + rx) + 2):
                nx, ny = (x + 0.5 - cx) / rx, (y + 0.5 - cy) / ry
                d = nx * nx + ny * ny
                if d > 1:
                    continue
                lit = -(nx * light[0] + ny * light[1])
                if dither and _hash(x, y, seed) < dither:
                    lit += 0.35 if _hash(y, x, seed) < 0.5 else -0.35
                self.put(x, y, pal[2] if lit > 0.35 else pal[0] if lit < -0.3 or d > 0.82 and lit < 0 else pal[1])

    def rect(self, x0, y0, x1, y1, pal):
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                self.put(x, y, pal[2] if x == x0 else pal[0] if x == x1 else pal[1])

    def line(self, x0, y0, x1, y1, c, width=1):
        steps = max(1, int(max(abs(x1 - x0), abs(y1 - y0)) * 2))
        for i in range(steps + 1):
            t = i / steps
            x, y = x0 + (x1 - x0) * t, y0 + (y1 - y0) * t
            for k in range(width):
                self.put(x + (k if abs(y1 - y0) > abs(x1 - x0) else 0), y + (0 if abs(y1 - y0) > abs(x1 - x0) else k), c)

    def curve(self, x0, y0, dx, dy, droop, c, width=1, steps=12):
        for i in range(steps + 1):
            t = i / steps
            for k in range(width):
                self.put(x0 + dx * t + k * 0, y0 + dy * t + droop * t * t + k, c)

    def tri(self, cx, top, bottom, half, pal):
        for y in range(top, bottom + 1):
            w = half * (y - top + 1) / (bottom - top + 1)
            for x in range(int(cx - w), int(cx + w) + 1):
                rel = (x - cx) / max(w, 0.5)
                self.put(x, y, pal[2] if rel < -0.35 else pal[0] if rel > 0.45 else pal[1])

    def image(self, outline=True):
        img = Image.new("RGBA", (self.w, self.h), (0, 0, 0, 0))
        p = img.load()
        for (x, y), c in self.px.items():
            p[x, y] = (*c, 255)
        if outline:
            for (x, y) in list(self.px):
                for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    n = (x + dx, y + dy)
                    if 0 <= n[0] < self.w and 0 <= n[1] < self.h and n not in self.px:
                        img.putpixel(n, (*OUTLINE, 255))
        return img


def _hash(x, y, seed):
    h = (x * 374761393 + y * 668265263 + seed * 2246822519) & 0xFFFFFFFF
    h = ((h ^ (h >> 13)) * 1274126177) & 0xFFFFFFFF
    return (h & 0xFFFF) / 65535.0


def mix(a, b, t):
    return tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(3))


def angle(deg, r):
    return math.cos(math.radians(deg)) * r, math.sin(math.radians(deg)) * r
