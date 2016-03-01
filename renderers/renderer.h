
#ifndef B_RENDERERS_RENDERER_H_
#define B_RENDERERS_RENDERER_H_

#include <glm/glm.hpp>
#include <vector>

#include "timer.h"

namespace renderer {

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

// Renderer interface
class Renderer {
  virtual ~Renderer() {}
  virtual void Render() = 0;
  virtual void AddRenderable(Renderable *r) = 0;
};

// Builds appropriate renderer
class RendererBuilder {};

} // namespace renderer

#endif // B_RENDERERS_RENDERER_H_
