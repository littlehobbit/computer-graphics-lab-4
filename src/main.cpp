#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <algorithm>
#include <cmath>
#include <initializer_list>
#include <iostream>
#include <list>
#include <optional>
#include <set>
#include <tuple>
#include <utility>
#include <vector>

#include "point.h"

using Vector2d = sf::Vector2<double>;
using Point = sf::Vector2<double>;

constexpr float eps = 0.01;
namespace std {
bool operator<(const Point& lhs, const Point& rhs) {
  return lhs.x < rhs.x || lhs.x == rhs.x && lhs.y < rhs.y;
}
}  // namespace std

bool operator==(const Point& lhs, const Point& rhs) {
  return std::abs(lhs.x - rhs.x) <= eps && std::abs(lhs.y - rhs.y) <= eps;
}

bool operator!=(const Point& lhs, const Point& rhs) { return !(lhs == rhs); }

enum class IntersectType { In, Out, Parallel };

using IntersectPoint = std::pair<Point, IntersectType>;

struct Line {
  Point start;
  Point end;

  std::optional<IntersectPoint> get_intersection(const Line& rhs) const {
    // TODO: stopre a, b ,c in line
    double a2 = rhs.end.y - rhs.start.y;
    double b2 = rhs.start.x - rhs.end.x;
    double c2 = a2 * rhs.start.x + b2 * rhs.start.y;

    double _a1 = end.y - start.y;
    double _b1 = start.x - end.x;
    double _c1 = _a1 * start.x + _b1 * start.y;

    double determinant = _a1 * b2 - a2 * _b1;

    if (determinant == 0) {
      return {};
    } else {
      double x = (b2 * _c1 - _b1 * c2) / determinant;
      double y = (_a1 * c2 - a2 * _c1) / determinant;

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

class Shape {
 public:
  Shape(std::list<Point> points) : _points(std::move(points)) {
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

 private:
  std::list<Point> _points;
  std::list<Line> _edges;
};

double len(const Vector2d& vec) {
  return std::sqrt(vec.x * vec.x + vec.y * vec.y);
}

Vector2d get_in_normal(const Line& line) {
  return Vector2d{line.end.y - line.start.y, -(line.end.x - line.start.x)};
}

double dot_product(const Vector2d& lhs, const Vector2d& rhs) {
  return lhs.x * rhs.x + lhs.y * rhs.y;
}

// Cyrus–Beck

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
      } while (out_points.count(*total) == 0 && in_points.count(*total) == 0);

      total = std::find(clipping.begin(), clipping.end(), *total);

      do {
        masked.push_back(*total);

        total++;
        if (total == clipping.end()) {
          total = clipping.begin();
        }
      } while (out_points.count(*total) == 0 && in_points.count(*total) == 0);

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

      auto reversed_total =
          std::find(clipping.rbegin(), clipping.rend(), *total);

      do {
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

bool point_in_shape(const Point& point, const Shape& shape) {
  for (const auto& edge : shape.get_edges()) {
    auto det = ((edge.end.x - edge.start.x) * (-point.y + edge.start.y) -
                (-edge.end.y + edge.start.y) * (point.x - edge.start.x));
    if (det > 0) {
      return false;
    }
  }
  return true;
}

bool shape_in(const Shape& shape, const Shape& other) {
  for (const auto& point : shape.get_points()) {
    if (!point_in_shape(point, other)) {
      return false;
    }
  }
  return true;
}

bool shape_out(const Shape& shape, const Shape& other) {
  for (const auto& point : shape.get_points()) {
    if (point_in_shape(point, other)) {
      return false;
    }
  }
  return true;
}

using OutShapes = std::list<Shape>;
using InnerShapes = std::list<Shape>;
std::pair<OutShapes, InnerShapes> clipping(const Shape& subj,
                                           const Shape& mask) {
  // TODO: check subj in mask

  std::set<Point> in_points, out_points;

  auto clipped_points = subj.get_points();
  auto clipping_points = mask.get_points();

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

  if (out_points.empty() && in_points.empty()) {
    if (shape_in(subj, mask)) {
      return {{}, {subj}};
    } else if (shape_out(subj, mask)) {
      return {{subj}, {}};
    }
  }

  // if (in_points.empty() && out_points.empty()) {
  //   return shape_in(subj, mask)
  //              ? std::make_pair<OutShapes, InnerShapes>({}, {subj})
  //              : std::make_pair<OutShapes, InnerShapes>({subj}, {});
  // }
  // if true - return subj as internal
  // esle - subj is out of mask, return subj as out

  // TODO: if there some in/out points
  auto merged_shapes =
      get_merged_shapes(clipped_points, clipping_points, in_points, out_points);
  auto masked_shapes =
      get_masked_shapes(clipped_points, clipping_points, in_points, out_points);

  return std::make_pair(std::move(merged_shapes), std::move(masked_shapes));
}

int main(int argc, char* argv[]) {
  Shape triangle = {{200, 550}, {400, 200}, {600, 550}};

  const auto square_side = 50;
  float start_x = 300;
  float step = 0.01;

  sf::RenderWindow window(sf::VideoMode(800, 800), "lab-4");

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        window.close();
      }
    }

    window.clear();

    Shape square = {{start_x, 300},
                    {start_x + square_side, 300},
                    {start_x + square_side, 400},
                    {start_x, 400}};

    // for (const auto& [start, end] : square.get_edges()) {
    //   sf::Vertex vertex[] = {sf::Vector2f(start.x, start.y),
    //                          sf::Vector2f(end.x, end.y)};
    //   window.draw(vertex, 2, sf::PrimitiveType::Lines);
    // }

    auto [merged, masked] = clipping(square, triangle);
    for (const auto& shape : merged) {
      for (const auto& [start, end] : shape.get_edges()) {
        sf::Vertex vertex[] = {{sf::Vector2f(start.x, start.y), sf::Color::Red},
                               {sf::Vector2f(end.x, end.y), sf::Color::Red}};
        window.draw(vertex, 2, sf::PrimitiveType::Lines);
      }
    }

    for (const auto& shape : masked) {
      for (const auto& [start, end] : shape.get_edges()) {
        sf::Vertex vertex[] = {{sf::Vector2f(start.x, start.y),
        sf::Color::Red},
                               {sf::Vector2f(end.x, end.y), sf::Color::Red}};
        window.draw(vertex, 2, sf::PrimitiveType::Lines);
      }
    }

    for (const auto& [start, end] : triangle.get_edges()) {
      sf::Vertex vertex[] = {sf::Vector2f(start.x, start.y),
                             sf::Vector2f(end.x, end.y)};
      window.draw(vertex, 2, sf::PrimitiveType::Lines);
    }

    start_x += step;
    if (start_x + square_side > window.getSize().x || start_x <= 0) {
      step *= -1;
    }

    window.display();
  }
}
