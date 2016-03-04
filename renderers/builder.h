
#ifndef B_RENDERERS_BUILDER_H_
#define B_RENDERERS_BUILDER_H_

#include <functional>

#include "renderers/gl/renderer.h"
#include "renderers/renderer.h"

namespace renderer {

class Builder {
public:
  // Add model matrix
  Builder View(Renderable *v) {
    view = v;
    return *this;
  }

  Builder Projection(std::function<Renderable *(size_t, size_t)> p) {
    projection = p;
    return *this;
  }

  std::unique_ptr<Renderer> Build() {
    return std::unique_ptr<Renderer>(new gl::Renderer(view, projection));
  }

private:
  Renderable *view;
  std::function<Renderable *(size_t, size_t)> projection;
};

} // namespace renderer

#endif // B_RENDERERS_BUILDER_H_
