#ifndef CONTAINER_H
#define CONTAINER_H

#include "shape.h"

// 1. Двигать область видимости
// 3. 0 слой - фон (совпадает с областью видимости)
// 4. N-й слой - окно


using Layer = std::vector<Shape>;

class Container {
 public:
  Container() = default;

  void add_layer(Layer layer) { _layers.push_back(std::move(layer)); }



 private:
  std::vector<Layer> _layers;
};

#endif
