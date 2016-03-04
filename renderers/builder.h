
#ifndef B_RENDERERS_BUILDER_H_
#define B_RENDERERS_BUILDER_H_

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

  Builder Projection(Renderable *p) {
    projection = p;
    return *this;
  }

  Renderer *Build() { return new gl::Renderer(view, projection); }

private:
  Renderable *view, *projection;
};

} // namespace renderer

#endif // B_RENDERERS_BUILDER_H_
