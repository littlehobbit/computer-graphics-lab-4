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

using OutShapes = std::list<Shape>;
using InnerShapes = std::list<Shape>;
std::pair<OutShapes, InnerShapes> clipping(const Shape& subj,
                                           const Shape& mask) {
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

  if (out_points.empty() || in_points.empty()) {
    if (mask.contains(subj)) {
      return {{}, {subj}};
    } else {
      return {{subj}, {}};
    }
  }

  auto merged_shapes =
      get_merged_shapes(clipped_points, clipping_points, in_points, out_points);
  auto masked_shapes =
      get_masked_shapes(clipped_points, clipping_points, in_points, out_points);

  return std::make_pair(std::move(merged_shapes), std::move(masked_shapes));
}

int main(int argc, char* argv[]) {
  Shape big_mask = {{200, 200}, {600, 200}, {600, 600}, {200, 600}};

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

    auto [merged, masked] = clipping(square, big_mask);
    for (const auto& shape : merged) {
      for (const auto& edge : shape.get_edges()) {
        sf::Vertex vertex[] = {
            {sf::Vector2f(edge.start.x, edge.start.y), sf::Color::Red},
            {sf::Vector2f(edge.end.x, edge.end.y), sf::Color::Red}};
        window.draw(vertex, 2, sf::PrimitiveType::Lines);
      }
    }

    for (const auto& shape : masked) {
      for (const auto& edge : shape.get_edges()) {
        sf::Vertex vertex[] = {
            {sf::Vector2f(edge.start.x, edge.start.y), sf::Color::Red},
            {sf::Vector2f(edge.end.x, edge.end.y), sf::Color::Red}};
        window.draw(vertex, 2, sf::PrimitiveType::Lines);
      }
    }

    for (const auto& edge : big_mask.get_edges()) {
      sf::Vertex vertex[] = {sf::Vector2f(edge.start.x, edge.start.y),
                             sf::Vector2f(edge.end.x, edge.end.y)};
      window.draw(vertex, 2, sf::PrimitiveType::Lines);
    }

    start_x += step;
    if (start_x + square_side > window.getSize().x || start_x <= 0) {
      step *= -1;
    }

    window.display();
  }
}
