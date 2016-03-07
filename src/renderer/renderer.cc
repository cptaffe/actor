// Copyright 2016 Connor Taffe

#include "src/renderer/renderer.h"

namespace renderer {

Rasterizable::~Rasterizable() {}

Renderable::~Renderable() {}

std::vector<glm::mat4> Renderable::Apply(std::vector<glm::mat4> m) {
  std::vector<glm::mat4> nm;
  auto rendering = Render();
  for (auto n : m) {
    for (auto r : rendering) {
      nm.push_back(n * r);
    }
  }
  return nm;
}
std::vector<glm::mat4> Renderable::Apply(Renderable *r) {
  return Apply(r->Render());
}

namespace shapes {

Factory::~Factory() {}

}  // namespace shapes

Renderer::~Renderer() {}

}  // namespace renderer
