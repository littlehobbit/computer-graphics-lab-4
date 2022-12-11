#ifndef LINE_H
#define LINE_H

#include <SFML/Graphics.hpp>
#include <cmath>
#include <optional>

using Vector2d = sf::Vector2<double>;
using Point = sf::Vector2<double>;

constexpr float eps = 0.01;
namespace std {
inline bool operator<(const Point& lhs, const Point& rhs) {
  return lhs.x < rhs.x || lhs.x == rhs.x && lhs.y < rhs.y;
}
}  // namespace std

inline bool operator==(const Point& lhs, const Point& rhs) {
  return std::abs(lhs.x - rhs.x) <= eps && std::abs(lhs.y - rhs.y) <= eps;
}

inline bool operator!=(const Point& lhs, const Point& rhs) { return !(lhs == rhs); }

enum class IntersectType { In, Out, Parallel };

using IntersectPoint = std::pair<Point, IntersectType>;

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

    if (determinant == 0) {
      return {};
    } else {
      double x = (rhs.b * this->c - this->b * rhs.c) / determinant;
      double y = (this->a * rhs.c - rhs.a * this->c) / determinant;

      // check if point belongs to segment
      if (x < std::min(start.x, end.x) || x > std::max(start.x, end.x))
        return {};
      if (y < std::min(start.y, end.y) || y > std::max(start.y, end.y))
        return {};

      if (x < std::min(rhs.start.x, rhs.end.x) ||
          x > std::max(rhs.start.x, rhs.end.x))
        return {};
      if (y < std::min(rhs.start.y, rhs.end.y) ||
          y > std::max(rhs.start.y, rhs.end.y))
        return {};

      IntersectPoint point = {
          {x, y}, determinant < 0 ? IntersectType::In : IntersectType::Out};
      return point;
    }
  }
};

#endif
