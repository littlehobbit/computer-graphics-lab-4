#ifndef CLIPPING_H
#define CLIPPING_H

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <iostream>
#include <list>
#include <set>

#include "line.h"
#include "shape.h"

double len(const Vector2d& vec) {
  return std::sqrt(vec.x * vec.x + vec.y * vec.y);
}

void insert_by_line(const Point& point, const Line& line,
                    std::list<Point>& list) {
  if (std::find(list.begin(), list.end(), point) != list.end()) return;

  auto begin = std::find(list.begin(), list.end(), line.start);
  auto end = std::find(list.begin(), list.end(), line.end);

  const auto distance_from_start = len(point - line.start);
  auto it = begin;
  it++;

  while (it != end && it != list.end()) {
    if (len(*it - *begin) >= distance_from_start) {
      break;
    }
    it++;
  }

  list.insert(it, point);
}

std::list<Shape> get_masked_shapes(std::list<Point> clipped,
                                   std::list<Point> clipping,
                                   std::set<Point> in_points,
                                   std::set<Point> out_points) {
  std::set<Point> used;
  std::list<Shape> masked_shapes;

  for (const auto& point : in_points) {
    if (used.count(point)) {
      continue;
    }

    const auto start_point = point;
    std::list<Point> masked{};

    auto total = std::find(clipped.begin(), clipped.end(), start_point);

    do {
      used.insert(*total);

      do {
        used.insert(*total);
        masked.push_back(*total);

        total++;
        if (total == clipped.end()) {
          total = clipped.begin();
        }
      } while (in_points.count(*total) == 0 && out_points.count(*total) == 0);

      used.insert(*total);
      if (*total == start_point) break;

      used.insert(*total);
      total = std::find(clipping.begin(), clipping.end(), *total);

      do {
        used.insert(*total);
        masked.push_back(*total);

        total++;
        if (total == clipping.end()) {
          total = clipping.begin();
        }
      } while (in_points.count(*total) == 0 && out_points.count(*total) == 0);

      total = std::find(clipped.begin(), clipped.end(), *total);

    } while (*total != start_point);

    masked_shapes.emplace_back(std::move(masked));
  }

  return masked_shapes;
}

std::list<Shape> get_merged_shapes(std::list<Point> clipped,
                                   std::list<Point> clipping,
                                   std::set<Point> in_points,
                                   std::set<Point> out_points) {
  std::set<Point> used;
  std::list<Shape> merged_shapes;

  for (const auto& point : out_points) {
    if (used.count(point)) {
      continue;
    }

    const auto start_point = point;

    std::list<Point> merged{};

    auto total = std::find(clipped.begin(), clipped.end(), start_point);

    do {
      do {
        used.insert(*total);
        merged.push_back(*total);

        total++;
        if (total == clipped.end()) {
          total = clipped.begin();
        }
      } while (in_points.count(*total) == 0 && out_points.count(*total) == 0);

      used.insert(*total);
      if (*total == start_point) break;

      auto reversed_total =
          std::find(clipping.rbegin(), clipping.rend(), *total);

      do {
        used.insert(*reversed_total);
        merged.push_back(*reversed_total);

        reversed_total++;
        if (reversed_total == clipping.rend()) {
          reversed_total = clipping.rbegin();
        }
      } while (in_points.count(*reversed_total) == 0 &&
               out_points.count(*reversed_total) == 0);

      total = std::find(clipped.begin(), clipped.end(), *reversed_total);

    } while (*total != start_point);

    merged_shapes.emplace_back(std::move(merged));
  }

  return merged_shapes;
}

using OutShapes = std::list<Shape>;
using InnerShapes = std::list<Shape>;
std::pair<OutShapes, InnerShapes> clipping(const Shape& subj, const Shape& mask,
                                           bool ret_masked) {
  if (mask.contains(subj)) {
    if (ret_masked)
      return {{}, {subj}};
    else
      return {};
  } else {
    std::set<Point> in_points, out_points;

    std::list<Point> clipped_points(subj.get_points().begin(),
                                    subj.get_points().end());
    std::list<Point> clipping_points(mask.get_points().begin(),
                                     mask.get_points().end());

    for (const auto& line : subj.get_edges()) {
      for (const auto& edge : mask.get_edges()) {
        auto intersection = line.get_intersection(edge);

        if (intersection.has_value()) {
          if (intersection->second == IntersectType::In) {
            in_points.insert(intersection->first);
          } else {
            out_points.insert(intersection->first);
          }

          insert_by_line(intersection->first, line, clipped_points);
          insert_by_line(intersection->first, edge, clipping_points);
        }
      }
    }

    if (out_points.empty() || in_points.empty() ||
        out_points.size() == 1 && in_points.size() == 1 &&
            *out_points.begin() == *in_points.begin()) {
      return {{subj}, {}};
    } else {
      auto merged_shapes = get_merged_shapes(clipped_points, clipping_points,
                                             in_points, out_points);
      auto masked_shapes = get_masked_shapes(clipped_points, clipping_points,
                                             in_points, out_points);

      if (ret_masked)
        return std::make_pair(std::move(merged_shapes),
                              std::move(masked_shapes));
      else
        return std::make_pair(std::move(merged_shapes), InnerShapes{});
    }
  }
}

#endif
