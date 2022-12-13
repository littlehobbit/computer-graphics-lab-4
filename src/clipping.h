#ifndef CLIPPING_H
#define CLIPPING_H

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <iostream>
#include <list>
#include <set>

#include "line.h"
#include "shape.h"

using ExternalShapes = std::list<Shape>;
using InnerShapes = std::list<Shape>;

double len(const Vector2d& vec) {
  return std::sqrt(vec.x * vec.x + vec.y * vec.y);
}

class WeilerAthertonAlgorithm {
 public:
  WeilerAthertonAlgorithm(const Shape& subj, const Shape& mask)
      : _subj(subj), _mask(mask) {}

  std::pair<ExternalShapes, InnerShapes> clipping(bool ret_masked) {
    if (_mask.contains(_subj)) {
      if (ret_masked)
        return {{}, {_subj}};
      else
        return {};
    } else {
      _subj_points = _subj.get_points();
      _mask_points = _mask.get_points();

      calc_and_classify_intersections();

      if (has_intersections()) {
        auto external_shapes = get_external_shapes();

        if (ret_masked) {
          auto inner_shapes = get_inner_shapes();
          return {std::move(external_shapes), std::move(inner_shapes)};
        } else {
          return {std::move(external_shapes), InnerShapes{}};
        }
      } else {
        return {{_subj}, {}};
      }
    }
  }

 private:
  void calc_and_classify_intersections() {
    for (const auto& line : _subj.get_edges()) {
      for (const auto& edge : _mask.get_edges()) {
        auto intersection = line.get_intersection(edge);

        if (intersection.has_value()) {
          if (intersection->second == IntersectType::In) {
            _in_points.insert(intersection->first);
          } else {
            _out_points.insert(intersection->first);
          }

          insert_on_line(intersection->first, line, _subj_points);
          insert_on_line(intersection->first, edge, _mask_points);
        }
      }
    }
  }

  void insert_on_line(const Point& point, const Line& line,
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

  std::list<Shape> get_external_shapes() const {
    std::set<Point> used;
    std::list<Shape> merged_shapes;

    for (const auto& point : _out_points) {
      if (used.count(point)) {
        continue;
      }

      const auto start_point = point;

      std::list<Point> merged{};

      auto total =
          std::find(_subj_points.begin(), _subj_points.end(), start_point);

      do {
        // go forward on subj
        do {
          used.insert(*total);
          merged.push_back(*total);

          total++;
          if (total == _subj_points.end()) {
            total = _subj_points.begin();
          }
        } while (_in_points.count(*total) == 0 &&
                 _out_points.count(*total) == 0);

        used.insert(*total);
        if (*total == start_point) break;

        auto reversed_total =
            std::find(_mask_points.rbegin(), _mask_points.rend(), *total);

        // go backward on mask
        do {
          used.insert(*reversed_total);
          merged.push_back(*reversed_total);

          reversed_total++;
          if (reversed_total == _mask_points.rend()) {
            reversed_total = _mask_points.rbegin();
          }
        } while (_in_points.count(*reversed_total) == 0 &&
                 _out_points.count(*reversed_total) == 0);

        total = std::find(_subj_points.begin(), _subj_points.end(),
                          *reversed_total);

      } while (*total != start_point);

      merged_shapes.emplace_back(std::move(merged));
    }

    return merged_shapes;
  }

  std::list<Shape> get_inner_shapes() const {
    std::set<Point> used;
    std::list<Shape> masked_shapes;

    for (const auto& point : _in_points) {
      if (used.count(point)) {
        continue;
      }

      const auto start_point = point;
      std::list<Point> masked{};

      auto total =
          std::find(_subj_points.begin(), _subj_points.end(), start_point);

      do {
        used.insert(*total);

        // go forward on subj
        do {
          used.insert(*total);
          masked.push_back(*total);

          total++;
          if (total == _subj_points.end()) {
            total = _subj_points.begin();
          }
        } while (_in_points.count(*total) == 0 &&
                 _out_points.count(*total) == 0);

        used.insert(*total);
        if (*total == start_point) break;

        used.insert(*total);
        total = std::find(_mask_points.begin(), _mask_points.end(), *total);

        // go forward on mask
        do {
          used.insert(*total);
          masked.push_back(*total);

          total++;
          if (total == _mask_points.end()) {
            total = _mask_points.begin();
          }
        } while (_in_points.count(*total) == 0 &&
                 _out_points.count(*total) == 0);

        total = std::find(_subj_points.begin(), _subj_points.end(), *total);

      } while (*total != start_point);

      masked_shapes.emplace_back(std::move(masked));
    }

    return masked_shapes;
  }

  bool has_intersections() const {
    return !(_in_points.empty() && _out_points.empty());
  }

  const Shape& _subj;
  const Shape& _mask;

  std::list<Point> _subj_points;
  std::list<Point> _mask_points;

  std::set<Point> _in_points, _out_points;
};

#endif
