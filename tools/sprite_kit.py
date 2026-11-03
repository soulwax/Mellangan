"""Tiny procedural pixel painter matching Kenney's Tiny Town look: flat 3-tone shading lit from the top left,
a 1px dark outline, no anti-aliasing. Shapes are painted into a layer dict and resolved into an RGBA image.

render() is the material pipeline: each material gets a 5-tone OKLab ramp grown from its mean colour (Kenney's
lightness step, shadows hue-shifted toward violet, lights toward warm, tones snapped onto Kenney's own colours when
close), the hand-drawn tones are mapped onto it in order, and the top edge gets a rim light, the base a core shadow.
lint() counts colours and orphans (Kenney's characters average 4.7 orphans and 7 colours)."""
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
        region = set(points)
        step = 4
        for x, y in sorted(region):
            if y % step or (x + (y // step) * 2 + seed) % step or _hash(x, y, seed) > min(1.0, density * 2.5):
                continue
            if (x + 1, y) not in region or (x, y - 1) not in region:
                continue
            self.put(x, y, dark)
            self.put(x + 1, y, dark)
            self.put(x, y - 1, light)

    def ellipse(self, cx, cy, rx, ry, pal, light=LIGHT, dither=0.0, seed=0):
        for y in range(int(cy - ry) - 1, int(cy + ry) + 2):
            for x in range(int(cx - rx) - 1, int(cx + rx) + 2):
                nx, ny = (x + 0.5 - cx) / rx, (y + 0.5 - cy) / ry
                d = nx * nx + ny * ny
                if d > 1:
                    continue
                lit = -(nx * light[0] + ny * light[1])
                if dither:
                    for edge in (0.35, -0.3):
                        if abs(lit - edge) < 0.08 + 0.2 * dither:
                            lit = edge + (0.01 if (x + y) % 2 else -0.01)
                self.put(x, y, pal[2] if lit > 0.35 else pal[0] if lit < -0.3 or d > 0.82 and lit < 0 else pal[1])

    def column(self, x0, y0, x1, y1, pal):
        w = x1 - x0
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                t = (x - x0 + 0.5) / (w + 1)
                self.put(x, y, pal[2] if 0.12 < t < 0.4 else pal[0] if t > 0.72 else pal[1])

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


KENNEY = (
    (118, 59, 54), (195, 75, 53), (232, 69, 55), (242, 132, 98), (189, 108, 74), (227, 134, 40),
    (207, 130, 84), (234, 165, 108), (253, 190, 83), (247, 194, 130), (252, 188, 143),
    (132, 198, 105), (71, 159, 74), (37, 149, 106), (67, 225, 179), (105, 255, 212),
    (117, 227, 255), (0, 154, 220), (82, 96, 124), (139, 155, 180), (192, 203, 220), (38, 43, 68),
    (155, 76, 163), (209, 118, 208), (255, 255, 255),
)
STEP = 0.135
SHADOW_HUE, LIGHT_HUE = 300.0, 85.0


def _lin(v):
    v /= 255
    return v / 12.92 if v <= 0.04045 else ((v + 0.055) / 1.055) ** 2.4


def _gam(v):
    v = max(0.0, min(1.0, v))
    return int(round(255 * (v * 12.92 if v <= 0.0031308 else 1.055 * v ** (1 / 2.4) - 0.055)))


def oklab(c):
    r, g, b = (_lin(v) for v in c[:3])
    l = (0.4122214708 * r + 0.5363325363 * g + 0.0514459929 * b) ** (1 / 3)
    m = (0.2119034982 * r + 0.6806995451 * g + 0.1073969566 * b) ** (1 / 3)
    s = (0.0883024619 * r + 0.2817188376 * g + 0.6299787005 * b) ** (1 / 3)
    return (0.2104542553 * l + 0.7936177850 * m - 0.0040720468 * s,
            1.9779984951 * l - 2.4285922050 * m + 0.4505937099 * s,
            0.0259040371 * l + 0.7827717662 * m - 0.8086757660 * s)


def rgb(lab):
    L, a, b = lab
    l = (L + 0.3963377774 * a + 0.2158037573 * b) ** 3
    m = (L - 0.1055613458 * a - 0.0638541728 * b) ** 3
    s = (L - 0.0894841775 * a - 1.2914855480 * b) ** 3
    return (_gam(4.0767416621 * l - 3.3077115913 * m + 0.2309699292 * s),
            _gam(-1.2684380046 * l + 2.6097574011 * m - 0.3413193965 * s),
            _gam(-0.0041960863 * l - 0.7034186147 * m + 1.7076147010 * s))


def lch(c):
    L, a, b = oklab(c)
    return L, math.hypot(a, b), math.degrees(math.atan2(b, a)) % 360


def _toward(h, target, t):
    d = (target - h + 540) % 360 - 180
    return (h + d * t) % 360


def _delta(p, q):
    a, b = oklab(p), oklab(q)
    return math.sqrt(sum((a[i] - b[i]) ** 2 for i in range(3)))


def ramp(base, n=5):
    L0, C0, H0 = lch(base)
    mid = n // 2
    out = []
    for i in range(n):
        k = i - mid
        L = min(0.97, max(0.24, L0 + k * STEP))
        C = C0 * (1 - 0.18 * abs(k)) if k < 0 else C0 * (1 - 0.22 * k)
        H = _toward(H0, SHADOW_HUE if k < 0 else LIGHT_HUE, min(0.5, 0.11 * abs(k)) if C0 > 0.025 else 0)
        c = rgb((L, C * math.cos(math.radians(H)), C * math.sin(math.radians(H))))
        near = min(KENNEY, key=lambda q: _delta(c, q))
        out.append(near if _delta(c, near) < 0.035 else c)
    return out


def _assign(colours, tones):
    ls = [oklab(c)[0] for c in colours]
    order = sorted(range(len(colours)), key=lambda i: ls[i])
    n, m = len(tones), len(order)
    best = {}

    def go(i, lo):
        if i == m:
            return 0.0, ()
        if (i, lo) in best:
            return best[(i, lo)]
        res = (1e9, ())
        for t in range(lo, n - (m - i - 1)):
            cost, rest = go(i + 1, t + 1)
            cost += _delta(colours[order[i]], tones[t])
            if cost < res[0]:
                res = (cost, (t,) + rest)
        best[(i, lo)] = res
        return res

    picks = go(0, 0)[1]
    out = [0] * m
    for k, i in enumerate(order):
        out[i] = picks[k]
    return out


def materials(palette):
    keys = [k for k, c in palette.items() if lch(c)[0] > 0.26]
    parent = {k: k for k in keys}

    def root(k):
        while parent[k] != k:
            k = parent[k]
        return k

    for a in keys:
        for b in keys:
            La, Ca, Ha = lch(palette[a])
            Lb, Cb, Hb = lch(palette[b])
            grey = Ca < 0.045 and Cb < 0.045
            if a < b and (grey or (Ca >= 0.045 and Cb >= 0.045 and abs((Ha - Hb + 540) % 360 - 180) < 20
                                     and max(Ca, Cb) < 2.2 * min(Ca, Cb))):
                parent[root(a)] = root(b)
    groups = {}
    for k in keys:
        groups.setdefault(root(k), []).append(k)
    return list(groups.values())


def render(rows, families, flat=None, rim=True, minimum=8):
    h, w = len(rows), len(rows[0])
    ink = {k for k, v in (flat or {}).items() if v == OUTLINE}
    inside = {(x, y) for y in range(h) for x in range(w) if rows[y][x] != "." and rows[y][x] not in ink}
    count = {}
    for y in range(h):
        for x in range(w):
            count[rows[y][x]] = count.get(rows[y][x], 0) + 1
    tone, ramps = {}, {}
    for fam, palette in families:
        if sum(count.get(k, 0) for k in fam) < minimum:
            continue
        cols = [palette[k] for k in fam]
        lab = [oklab(c) for c in cols]
        weights = [count.get(k, 0) + 1 for k in fam]
        mean = tuple(sum(lab[i][j] * weights[i] for i in range(len(fam))) / sum(weights) for j in range(3))
        r = ramp(rgb(mean), 5 if len(fam) <= 3 else max(7, len(fam)))
        for k, t in zip(fam, _assign(cols, r)):
            tone[k], ramps[k] = t, r
    c = Canvas(w, h)
    flat = dict(flat or {})
    for _, palette in families:
        flat.update({k: v for k, v in palette.items() if k not in flat})
    for x, y in sorted((x, y) for y in range(h) for x in range(w) if rows[y][x] != "."):
        k = rows[y][x]
        if k not in tone:
            c.put(x, y, flat[k])
            continue
        r, t = ramps[k], tone[k]
        if rim:
            same = lambda dx, dy: (x + dx, y + dy) in inside and rows[y + dy][x + dx] in tone and ramps[rows[y + dy][x + dx]] is r
            if (x, y - 1) not in inside and (x, y + 1) in inside and t >= len(r) // 2:
                t = min(len(r) - 1, t + 1)
            elif (x, y + 1) not in inside and (x, y - 1) in inside and same(0, -1):
                t = max(0, t - 1)
        c.put(x, y, r[t])
    return c


def shift(c, Lmid):
    L, C, H = lch(c)
    if C < 0.025:
        return c
    k = (L - Lmid) / STEP
    H = _toward(H, SHADOW_HUE if k < 0 else LIGHT_HUE, min(0.4, 0.08 * abs(k)))
    out = rgb((L, C * math.cos(math.radians(H)), C * math.sin(math.radians(H))))
    near = min(KENNEY, key=lambda q: _delta(out, q))
    return near if _delta(out, near) < 0.03 else out


def refine(img, rim=True):
    alpha = [c[3] for c in img.getdata()]
    if any(a not in (0, 255) for a in alpha) or sum(a == 255 for a in alpha) > 0.8 * len(alpha):
        return img
    p = img.load()
    w, h = img.width, img.height
    count = {}
    for y in range(h):
        for x in range(w):
            if p[x, y][3]:
                count[p[x, y][:3]] = count.get(p[x, y][:3], 0) + 1
    cols = [c for c in count if c != OUTLINE]
    keys = {c: str(i) for i, c in enumerate(cols)}
    tone, mapped = {}, {}
    for fam in materials({keys[c]: c for c in cols}):
        fc = sorted((cols[int(k)] for k in fam), key=lambda c: oklab(c)[0])
        Lmid = sum(oklab(c)[0] * count[c] for c in fc) / sum(count[c] for c in fc)
        shifted = [shift(c, Lmid) for c in fc]
        for i, c in enumerate(fc):
            tone[c] = (shifted, i)
    out = img.copy()
    q = out.load()
    solid = lambda x, y: 0 <= x < w and 0 <= y < h and p[x, y][3] and p[x, y][:3] != OUTLINE
    for y in range(h):
        for x in range(w):
            c = p[x, y][:3]
            if not p[x, y][3] or c not in tone:
                continue
            ramp_, t = tone[c]
            if rim and len(ramp_) > 1:
                if not solid(x, y - 1) and solid(x, y + 1) and t >= len(ramp_) // 2:
                    t = min(len(ramp_) - 1, t + 1)
                elif not solid(x, y + 1) and solid(x, y - 1) and tone.get(p[x, y - 1][:3], (None,))[0] is ramp_:
                    t = max(0, t - 1)
            q[x, y] = (*ramp_[t], 255)
    return out


def orphans(img):
    p, w, h = img.load(), img.width, img.height
    n = 0
    for y in range(h):
        for x in range(w):
            if p[x, y][3] == 0 or p[x, y][:3] == OUTLINE:
                continue
            nb = [p[x + dx, y + dy] for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1))
                  if 0 <= x + dx < w and 0 <= y + dy < h]
            diag = [p[x + dx, y + dy] for dx, dy in ((1, 1), (-1, 1), (1, -1), (-1, -1))
                    if 0 <= x + dx < w and 0 <= y + dy < h]
            if len(nb) == 4 and all(q == nb[0] for q in nb) and nb[0][3] and nb[0][:3] != OUTLINE \
                    and _delta(nb[0], p[x, y]) < 0.12 and diag.count(p[x, y]) < 2:
                n += 1
    return n


def lint(img):
    p = img.load()
    cols, solid = set(), 0
    for y in range(img.height):
        for x in range(img.width):
            if p[x, y][3]:
                solid += 1
                cols.add(p[x, y][:3])
    return {"pixels": solid, "colours": len(cols), "orphans": orphans(img)}
