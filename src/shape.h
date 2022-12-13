#ifndef SHAPE_H
#define SHAPE_H

#include <SFML/Graphics/Color.hpp>
#include <list>

#include "SFML/Graphics/RenderTarget.hpp"
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
  }

  Shape(const std::initializer_list<Point>& points) : _points(points) {
    auto edge_start = _points.end(), edge_end = _points.begin();
    edge_start--;
    while (edge_end != _points.end()) {
      _edges.push_back({*edge_start, *edge_end});
      edge_start = edge_end++;
    }
  }

  const auto& get_edges() const { return _edges; }

  const auto& get_points() const { return _points; }

  bool contains(const Point& point) const {
    for (const auto& edge : get_edges()) {
      auto det = ((edge.end.x - edge.start.x) * (-point.y + edge.start.y) -
                  (-edge.end.y + edge.start.y) * (point.x - edge.start.x));
      if (bigger(det, 0)) {
        return false;
      }
    }
    return true;
  }

  bool contains_point_of(const Shape& shape) const {
    for (const auto& p : shape._points) {
      if (contains(p)) return true;
    }
    return false;
  }

  bool contains(const Shape& other) const {
    for (const auto& point : other.get_points()) {
      if (!contains(point)) {
        return false;
      }
    }
    return true;
  }

  void draw(sf::RenderTarget& render, sf::Color line_color,
            LineStyle line_style = LineStyle::Line) const {
    for (const auto& edge : _edges) {
      edge.draw(render, line_color, line_style);
    }
  }

 private:
  std::list<Point> _points;
  std::list<Line> _edges;
};

#endif
