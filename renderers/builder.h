
#ifndef B_RENDERERS_BUILDER_H_
#define B_RENDERERS_BUILDER_H_

#include "renderers/gl/renderer.h"
#include "renderers/renderer.h"

#include <glm/glm.hpp>

namespace renderer {

class Builder {
public:
  // Add model matrix
  Builder View(glm::mat4 v) {
    view = v;
    return *this;
  }

  Builder Projection(glm::mat4 p) {
    projection = p;
    return *this;
  }

  Renderer *Build() { return new gl::Renderer(view, projection); }

private:
  glm::mat4 view, projection;
};

} // namespace renderer

#endif // B_RENDERERS_BUILDER_H_
