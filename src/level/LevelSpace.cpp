#include "level/Level.hpp"
#include "item/Upgrade.hpp"
#include <spdlog/spdlog.h>
#include <cstring>
#include "Game.hpp"
#include "Prof.h"
#include "Workers.hpp"
#include "audio/Sound.hpp"
#include "entity/Player.hpp"
#include "entity/pickup/Chest.hpp"
#include "hero/Hero.hpp"
#include "level/Encounter.hpp"
#include "level/ExpansionEncounter.hpp"
#include "level/Flavour.hpp"
#include "particle/Arc.hpp"
#include "particle/Drift.hpp"
#include "particle/Shockwave.hpp"
#include "particle/Spark.hpp"
#include "particle/VisualDetail.hpp"

int Level::gather(double x, double y, float r, Enemy **out, int max) const {
  int own = cell(y, yGrid) * GRID + cell(x, xGrid), n = 0;
  for (int i = cellStart[own]; i < cellStart[own + 1]; i++)
    if (!cells[i]->removed && (out[n++] = cells[i], n == max))
      return n;
  int x0 = cell(x - r, xGrid), x1 = cell(x + r, xGrid);
  int y0 = cell(y - r, yGrid), y1 = cell(y + r, yGrid);
  for (int yc = y0; yc <= y1; yc++)
    for (int xc = x0, c = yc * GRID + xc; xc <= x1; xc++, c++) {
      if (c == own)
        continue;
      for (int i = cellStart[c]; i < cellStart[c + 1]; i++)
        if (!cells[i]->removed && (out[n++] = cells[i], n == max))
          return n;
    }
  return n;
}

void Level::buildGrid() {
  xGrid = player->x - GRID * CELL * 0.5;
  yGrid = player->y - GRID * CELL * 0.5;
  std::fill(cellStart.begin(), cellStart.end(), 0);
  cellOf.clear();
  each<Enemy>([&](Enemy &e) {
    if (e.buried)
      return;
    int c = cell(e.y, yGrid) * GRID + cell(e.x, xGrid);
    cellOf.push_back(c);
    cellStart[c + 1]++;
  });
  for (int i = 1; i <= GRID * GRID; i++)
    cellStart[i] += cellStart[i - 1];
  std::copy(cellStart.begin(), cellStart.end() - 1, cellFill.begin());
  cells.resize(cellStart.back());
  size_t k = 0;
  each<Enemy>([&](Enemy &e) {
    if (!e.buried)
      cells[cellFill[cellOf[k++]]++] = &e;
  });
}

Mob *Level::playerIn(double x, double y, float r) {
  double xd = player->x - x, yd = player->y - y;
  return !player->dead() && xd * xd + yd * yd < double(r) * r ? player : nullptr;
}

std::vector<Enemy *> Level::nearest(double x, double y, float range, int n) {
  std::vector<std::pair<double, Enemy *>> found;
  each<Enemy>([&](Enemy &e) {
    double xd = e.x - x, yd = e.y - y, d = xd * xd + yd * yd;
    if (d < range * range && !e.buried)
      found.emplace_back(d, &e);
  });
  int k = std::min(n, int(found.size()));
  std::partial_sort(found.begin(), found.begin() + k, found.end(),
                    [](auto &a, auto &b) { return a.first < b.first; });
  std::vector<Enemy *> out;
  for (int i = 0; i < k; i++)
    out.push_back(found[i].second);
  return out;
}

void Level::collide(double &x, double &y, float radius) const {
  int tx = int(std::floor(x / 16)), ty = int(std::floor((y - 3) / 16));
  for (int yt = ty - 1; yt <= ty + 1; yt++)
    for (int xt = tx - 1; xt <= tx + 1; xt++) {
      int gx = xt - solidX, gy = yt - solidY;
      if (gx < 0 || gy < 0 || gx >= SOLID_W || gy >= SOLID_H ||
          solids[gy * SOLID_W + gx] <= 0)
        continue;
      double xc = xt * 16 + 8.0, yc = yt * 16 + 13.0, xd = x - xc, yd = y - yc;
      double min = solids[gy * SOLID_W + gx] + radius, d2 = xd * xd + yd * yd;
      if (d2 >= min * min)
        continue;
      double len = std::sqrt(d2);
      if (len > 1e-4)
        x = xc + xd / len * min, y = yc + yd / len * min;
      else
        x = xc, y = yc + min;
    }
}

int Level::surfaceAt(double x, double y) const {
  int gx = int(std::floor(x / 16)) - solidX,
      gy = int(std::floor((y - 2) / 16)) - solidY;
  if (gx < 0 || gy < 0 || gx >= SOLID_W || gy >= SOLID_H)
    return 0;
  return surfaces[gy * SOLID_W + gx] & 3;
}

Mob *Level::mob(entt::entity id) {
  if (!reg.valid(id))
    return nullptr;
  auto *o = reg.try_get<Owned>(id);
  if (!o || !o->self || o->self->removed)
    return nullptr;
  return dynamic_cast<Mob *>(o->self.get());
}

int Level::heightAt(double x, double y) const {
  int gx = int(std::floor(x / 16)) - solidX, gy = int(std::floor((y - 2) / 16)) - solidY;
  if (gx < 0 || gy < 0 || gx >= SOLID_W || gy >= SOLID_H)
    return 0;
  return surfaces[gy * SOLID_W + gx] >> 2 & 3;
}

float Level::climb(double x, double y, double dx, double dy) const {
  double d = std::sqrt(dx * dx + dy * dy);
  if (d < 1e-3)
    return 1;
  auto cell = [&](double px, double py) -> int {
    int gx = int(std::floor(px / 16)) - solidX,
        gy = int(std::floor((py - 2) / 16)) - solidY;
    return gx < 0 || gy < 0 || gx >= SOLID_W || gy >= SOLID_H
               ? -1
               : surfaces[gy * SOLID_W + gx];
  };
  int here = cell(x, y), ahead = cell(x + dx / d * 7, y + dy / d * 7);
  if (here < 0 || ahead < 0 || (ahead >> 2 & 3) <= (here >> 2 & 3) ||
      ahead & 16)
    return 1;
  return 0.35f;
}

bool Level::swimming(const SpriteObject *e) const {
  if (!waterInView || surfaceAt(e->x, e->y) != 1)
    return false;
  if (auto *en = dynamic_cast<const Enemy *>(e))
    return !en->type.floats;
  return dynamic_cast<const Player *>(e) != nullptr;
}

bool Level::blocks(double x, double y) const {
  int tx = int(std::floor(x / 16)), ty = int(std::floor(y / 16));
  for (int yt = ty - 1; yt <= ty + 1; yt++)
    for (int xt = tx - 1; xt <= tx + 1; xt++) {
      int gx = xt - solidX, gy = yt - solidY;
      if (gx < 0 || gy < 0 || gx >= SOLID_W || gy >= SOLID_H)
        continue;
      float r = solids[gy * SOLID_W + gx];
      double xd = x - (xt * 16 + 8.0), yd = y - (yt * 16 + 13.0);
      if (r > 0 && xd * xd + yd * yd < r * r)
        return true;
    }
  return false;
}
