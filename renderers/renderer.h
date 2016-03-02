
#ifndef B_RENDERERS_RENDERER_H_
#define B_RENDERERS_RENDERER_H_

#include <glm/glm.hpp>
#include <vector>

#include "renderers/timer.h"

namespace renderer {

class Rasterizable {
public:
  virtual ~Rasterizable() {}
  virtual std::vector<glm::vec3> Vertices() = 0;
};

// Renderable interface
class Renderable {
public:
  virtual ~Renderable(){};
  virtual std::vector<glm::mat4> Render() = 0;

  std::vector<glm::mat4> Apply(std::vector<glm::mat4> m) {
    std::vector<glm::mat4> nm;
    auto rendering = Render();
    for (auto n : m) {
      for (auto r : rendering) {
        nm.push_back(n * r);
      }
    }
    return nm;
  }
  std::vector<glm::mat4> Apply(Renderable *r) { return Apply(r->Render()); }
};

namespace shapes {

class Factory {
public:
  virtual ~Factory() {}
  virtual Rasterizable *Cube() = 0;
};

} // namespace shapes

// Renderer interface
class Renderer {
public:
  virtual ~Renderer() {}
  virtual void Render() = 0;
  virtual void AddRenderable(Rasterizable *r, std::vector<Renderable *> m) = 0;
  virtual shapes::Factory *ShapeFactory() = 0;
};

} // namespace renderer

#endif // B_RENDERERS_RENDERER_H_
