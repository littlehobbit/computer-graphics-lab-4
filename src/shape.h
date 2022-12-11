#ifndef SHAPE_H
#define SHAPE_H

#include <list>

#include "line.h"

class Shape {
 public:
  Shape(std::list<Point> points) : _points(points.begin(), points.end()) {
    auto edge_start = _points.end(), edge_end = _points.begin();
    edge_start--;
    while (edge_end != _points.end()) {
      _edges.push_back({*edge_start, *edge_end});
      edge_start = edge_end++;
    }

    _edges.push_back({_points[1], _points[3]});
  }

  Shape(const std::initializer_list<Point>& points) : _points(points) {
    auto edge_start = _points.end(), edge_end = _points.begin();
    edge_start--;
    while (edge_end != _points.end()) {
      _edges.push_back({*edge_start, *edge_end});
      edge_start = edge_end++;
    }
    _edges.push_back({_points[1], _points[3]});
  }

  const auto& get_edges() const { return _edges; }

  const auto& get_points() const { return _points; }

  bool contains(const Point& point) const {
    for (const auto& edge : get_edges()) {
      auto det = ((edge.end.x - edge.start.x) * (-point.y + edge.start.y) -
                  (-edge.end.y + edge.start.y) * (point.x - edge.start.x));
      if (det > 0) {
        return false;
      }
    }
    return true;
  }

  bool contains(const Shape& other) const {
    for (const auto& point : other.get_points()) {
      if (!contains(point)) {
        return false;
      }
    }
    return true;
  }

 private:
  std::vector<Point> _points;
  std::list<Line> _edges;
};

#endif