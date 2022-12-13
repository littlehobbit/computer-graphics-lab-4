#ifndef CONTAINER
#define CONTAINER

#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <iterator>
#include <memory>
#include <optional>
#include <vector>

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Vertex.hpp"
#include "SFML/System/Vector2.hpp"
#include "clipping.h"
#include "line.h"
#include "shape.h"

struct LayerMergeResult {
  sf::RectangleShape background{};
  std::list<Shape> shapes;
  std::list<Shape> masked_shapes;
};

class Layer {
 public:
  virtual ~Layer() = default;

  virtual LayerMergeResult merge(const LayerMergeResult& prev,
                                 bool show_masked) const = 0;
};

class BackgroundLayer : public Layer {
 public:
  explicit BackgroundLayer(const sf::Color& background_color)
      : _color(background_color) {}

  LayerMergeResult merge(const LayerMergeResult& prev,
                         bool show_masked) const override {
    auto ret = prev;
    ret.background.setFillColor(_color);
    return ret;
  }

 private:
  sf::Color _color;
};

class ShapesLayer : public Layer {
 public:
  ShapesLayer(std::vector<Shape> shapes) : _shapes(std::move(shapes)) {}

  LayerMergeResult merge(const LayerMergeResult& prev,
                         bool show_masked) const override {
    std::list<Shape> new_out_shapes;
    std::list<Shape> masked;

    for (const auto& upper_shape : _shapes) {
      for (const auto& prev_shape : prev.shapes) {
        WeilerAthertonAlgorithm algo(prev_shape, upper_shape);
        auto [out, inner] = algo.clipping(show_masked);

        std::move(out.begin(), out.end(), std::back_inserter(new_out_shapes));
        std::move(inner.begin(), inner.end(), std::back_inserter(masked));
      }
      new_out_shapes.push_back(upper_shape);
    }

    auto res = prev;
    res.shapes = std::move(new_out_shapes);

    if (show_masked) {
      std::move(masked.begin(), masked.end(),
                std::back_inserter(res.masked_shapes));
    }

    return res;
  }

 private:
  std::vector<Shape> _shapes;
};

class WindowMaskLayer : public Layer {
 public:
  WindowMaskLayer(int width, int height) : _width(width), _height(height) {}

  void set_pos(const sf::Vector2i& pos) { _pos = pos; }

  LayerMergeResult merge(const LayerMergeResult& prev,
                         bool show_masked) const override {
    std::list<Shape> masked_out;
    std::list<Shape> inner;

    Shape mask = {{_pos.x - _width / 2.0, _pos.y - _height / 2.0},
                  {_pos.x + _width / 2.0, _pos.y - _height / 2.0},
                  {_pos.x + _width / 2.0, _pos.y + _height / 2.0},
                  {_pos.x - _width / 2.0, _pos.y + _height / 2.0}};

    for (const auto& prev_shape : prev.shapes) {
      WeilerAthertonAlgorithm algo(prev_shape, mask);
      auto [out, in] = algo.clipping(true);

      std::move(out.begin(), out.end(), std::back_inserter(masked_out));
      std::move(in.begin(), in.end(), std::back_inserter(inner));
    }

    inner.push_back(mask);

    auto res = prev;
    res.shapes = std::move(inner);

    if (show_masked) {
      std::move(masked_out.begin(), masked_out.end(),
                std::back_inserter(res.masked_shapes));
    }

    res.background.setPosition(_pos.x - _width / 2.0, _pos.y - _height / 2.0);
    res.background.setSize(sf::Vector2f(_width, _height));

    return res;
  }

 private:
  sf::Vector2i _pos;
  int _width;
  int _height;
};

class Container {
 public:
  Container() = default;

  void add_layer(std::shared_ptr<Layer> layer) {
    _layers.push_back(std::move(layer));
  }

  void draw(sf::RenderTarget& render, bool show_masked) {
    LayerMergeResult res{};
    for (const auto& layer : _layers) {
      res = layer->merge(res, show_masked);
    }

    render.draw(res.background);

    if (show_masked) {
      for (const auto& shape : res.masked_shapes) {
        shape.draw(render, {0x20, 0x20, 0x20}, LineStyle::Dashed);
      }
    }

    for (const auto& shape : res.shapes) {
      shape.draw(render, sf::Color::White);
    }
  }

 private:
  std::vector<std::shared_ptr<Layer>> _layers;
};

#endif
