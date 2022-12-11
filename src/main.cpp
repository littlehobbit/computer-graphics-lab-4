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
#include <memory>
#include <optional>
#include <set>
#include <tuple>
#include <utility>
#include <vector>

#include "SFML/Window/Mouse.hpp"
#include "clipping.h"
#include "container.h"
#include "line.h"
#include "shape.h"

int main(int argc, char* argv[]) {
  Shape big_mask = {{200, 200}, {600, 200}, {600, 600}, {200, 600}};

  const auto square_side = 50;
  float start_x = 190;
  Shape triangle = {{100, 500}, {400, 100}, {700, 500}};

  sf::RenderWindow window(sf::VideoMode(800, 800), "lab-4");

  Container container;

  container.add_layer(std::make_shared<BackgroundLayer>(sf::Color::Black));

  container.add_layer(
      std::make_shared<ShapesLayer>(std::vector<Shape>{triangle}));
  container.add_layer(
      std::make_shared<ShapesLayer>(std::vector<Shape>{big_mask}));

  auto mask = std::make_shared<WindowMaskLayer>(200, 200);
  container.add_layer(mask);
  mask->set_pos({575, 202});

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) {
        window.close();
      }
    }

    window.clear();

    auto pos = sf::Mouse::getPosition(window);
    mask->set_pos(pos);

    // std::cout << pos.x << ' ' << pos.y << std::endl;

    container.draw(window, true);

    window.display();
  }
}
