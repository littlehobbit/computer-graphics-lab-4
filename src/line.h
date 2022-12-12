#ifndef LINE_H
#define LINE_H

#include <SFML/Graphics.hpp>
#include <cmath>
#include <optional>

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
// #include "clipping.h"

using Vector2d = sf::Vector2<double>;
using Point = Vector2d;

constexpr float eps = 0.01;

enum class IntersectType { In, Out, Parallel };

using IntersectPoint = std::pair<Point, IntersectType>;

enum class LineStyle { Line, Dashed };

inline bool equals(double a, double b) { return std::abs(a - b) <= eps; }

inline bool less(double a, double b) { return b - a > eps; }

inline bool bigger(double a, double b) { return a - b > eps; }

inline bool operator==(const Point& lhs, const Point& rhs) {
  return equals(lhs.x, rhs.x) && equals(lhs.y, rhs.y);
}

inline bool operator!=(const Point& lhs, const Point& rhs) {
  return !(lhs == rhs);
}

namespace std {
inline bool operator<(const Point& lhs, const Point& rhs) {
  return ::less(lhs.x, rhs.x) || equals(lhs.x, rhs.x) && ::less(lhs.y, rhs.y);
}
}  // namespace std

struct Line {
  Line(const Point& start, const Point& end)
      : start(start),
        end(end),
        a(end.y - start.y),
        b(start.x - end.x),
        c(a * start.x + b * start.y) {}

  Point start;
  Point end;

  double a;
  double b;
  double c;

  std::optional<IntersectPoint> get_intersection(const Line& rhs) const {
    double determinant = this->a * rhs.b - rhs.a * this->b;

    if (std::abs(determinant) < eps) {
      return {};
    } else {
      double x = (rhs.b * this->c - this->b * rhs.c) / determinant;
      double y = (this->a * rhs.c - rhs.a * this->c) / determinant;

      // check if point belongs to segment
      if (less(x, std::min(start.x, end.x)) ||
          bigger(x, std::max(start.x, end.x)))
        return {};
      if (less(y, std::min(start.y, end.y)) ||
          bigger(y, std::max(start.y, end.y)))
        return {};

      if (less(x, std::min(rhs.start.x, rhs.end.x)) ||
          bigger(x, std::max(rhs.start.x, rhs.end.x)))
        return {};
      if (less(y, std::min(rhs.start.y, rhs.end.y)) ||
          bigger(y, std::max(rhs.start.y, rhs.end.y)))
        return {};

      auto dir = less(determinant, 0) ? IntersectType::In : IntersectType::Out;
      auto point = Vector2d{x, y};
      if (rhs.end == point || rhs.start == point || start == point || end == point) {
        return {};
      }

      IntersectPoint ret = {point, dir};
      return ret;
    }
  }

  void draw(sf::RenderTarget& render, sf::Color color, LineStyle style) const {
    if (style == LineStyle::Line) {
      sf::Vertex vertex[] = {
          {sf::Vector2f(start.x, start.y), color},
          {sf::Vector2f(end.x, end.y), color},
      };
      render.draw(vertex, 2, sf::PrimitiveType::Lines);
    } else {
      const auto dash_len = 5;

      auto x = end.x - start.x;
      auto y = end.y - start.y;

      auto line_len = std::sqrt(x * x + y * y);

      const int dash_count = line_len / (2 * dash_len);

      const auto len_x = x * dash_len / line_len;
      const auto len_y = y * dash_len / line_len;

      auto start_dash = start;

      for (int i = 0; i < dash_count; i++) {
        auto end_dash = start_dash + Vector2d{len_x, len_y};

        sf::Vertex vertex[] = {
            {sf::Vector2f(start_dash.x, start_dash.y), color},
            {sf::Vector2f(end_dash.x, end_dash.y), color},
        };
        render.draw(vertex, 2, sf::PrimitiveType::Lines);

        start_dash += Vector2d{2 * len_x, 2 * len_y};
      }

      sf::Vertex vertex[] = {
          {sf::Vector2f(start_dash.x, start_dash.y), color},
          {sf::Vector2f(end.x, end.y), color},
      };
      render.draw(vertex, 2, sf::PrimitiveType::Lines);
    }
  }
};

#endif
