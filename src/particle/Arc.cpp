#include "particle/Arc.hpp"

namespace {
float jitter(uint32_t seed, int segment, int piece) {
  uint32_t h = seed ^ uint32_t(segment) * 0x9E3779B1u ^ uint32_t(piece) * 0x85EBCA6Bu;
  h ^= h >> 15, h *= 0x2C1B3C6Du, h ^= h >> 12;
  return float(h & 0xFFFF) / 32767.5f - 1;
}
} // namespace

Arc::Arc(std::vector<ArcNode> nodes, float life, Color color)
    : Particle(nodes.front().x, nodes.front().y, life), nodes(std::move(nodes)),
      color(color), seed(rng.next()) {}

void Arc::tick() {
  Particle::tick();
  if ((flicker -= DT) <= 0)
    flicker = 0.05f, seed = rng.next();
}

void Arc::render(Screen &s) {
  float a = std::min(1.0f, frac() * 1.6f);
  for (size_t i = 0; i + 1 < nodes.size(); i++) {
    double x0 = nodes[i].x, y0 = nodes[i].y;
    double dx = nodes[i + 1].x - x0, dy = nodes[i + 1].y - y0;
    double len = std::sqrt(dx * dx + dy * dy);
    if (len < 1)
      continue;
    int n = std::max(2, int(len / 10));
    double nx = -dy / len, ny = dx / len, amp = std::min(6.0, len * 0.15);
    double px = x0, py = y0;
    for (int k = 1; k <= n; k++) {
      double t = double(k) / n, off = k < n ? jitter(seed, int(i), k) * amp : 0;
      double qx = x0 + dx * t + nx * off, qy = y0 + dy * t + ny * off;
      s.line(px, py, qx, qy, 3, color.alpha(0.55f * a));
      s.line(px, py, qx, qy, 1, col::white.alpha(a));
      px = qx, py = qy;
    }
  }
}

void Arc::renderLight(Screen &s) {
  for (const ArcNode &n : nodes)
    s.glow(n.x, n.y, 40, color * (0.22f * frac()));
}
