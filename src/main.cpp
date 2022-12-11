#include <config-loader/ConfigLoader.h>
#include <config-loader/core/DefineSchema.h>
#include <config-loader/deserialize/Deserializer.h>

#include <CLI/App.hpp>
#include <CLI/CLI.hpp>
#include <CLI/Validators.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <list>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <tuple>
#include <utility>
#include <vector>

#include "clipping.h"
#include "container.h"
#include "line.h"
#include "shape.h"

namespace config {
DEFINE_SCHEMA(Point, (double)x, (double)y);
DEFINE_SCHEMA(BackgroundColor, (uint8_t)r, (uint8_t)g, (uint8_t)b);
DEFINE_SCHEMA(Shape, (std::vector<config::Point>)points);
DEFINE_SCHEMA(Layer, (std::vector<config::Shape>)shapes);
DEFINE_SCHEMA(Config, (BackgroundColor)background_color,
              (std::vector<config::Layer>)shapes_layers);
}  // namespace config

std::optional<config::Config> read_config(const std::string& file_path) {
  config::Config config;
  auto res = config_loader::loadJSON2Obj(config, file_path);
  if (res == config_loader::Result::SUCCESS) {
    return config;
  } else {
    return {};
  }
}

int main(int argc, char* argv[]) {
  bool show_masked = false;
  int mask_width = 200;
  int mask_height = 200;

  CLI::App app("lab-4 app", "lab-4");
  app.add_flag("--mode-b", show_masked, "Mode B");

  app.add_option("-a,--width", mask_width, "Mask width")
      ->default_val(mask_width);
  app.add_option("-b,--height", mask_height, "Mask height")
      ->default_val(mask_height);

  std::string config_path = "config.json";
  app.add_option("--config", config_path, "App settings")
      ->check(CLI::ExistingFile)
      ->default_val(config_path);

  CLI11_PARSE(app, argc, argv);

  auto config = read_config(config_path);
  if (!config.has_value()) {
    std::cerr << "Bad config!" << std::endl;
    return 1;
  }

  Container container;

  container.add_layer(std::make_shared<BackgroundLayer>(
      sf::Color{config->background_color.r, config->background_color.g,
                config->background_color.b}));

  for (const auto& layer : config->shapes_layers) {
    std::vector<Shape> shapes;
    for (const auto& shape : layer.shapes) {
      std::list<Point> points;
      for (const auto& [x, y] : shape.points) {
        points.push_back({x, y});
      }
      shapes.emplace_back(std::move(points));
    }
    auto shapes_layer = std::make_shared<ShapesLayer>(std::move(shapes));
    container.add_layer(std::move(shapes_layer));
  }

  auto mask = std::make_shared<WindowMaskLayer>(mask_width, mask_height);
  container.add_layer(mask);

  sf::RenderWindow window(sf::VideoMode(800, 800), "lab-4");
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

    container.draw(window, show_masked);

    window.display();
  }
}
