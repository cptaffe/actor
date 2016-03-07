// Copyright 2016 Connor Taffe

#ifndef SRC_RENDERER_BUILDER_H_
#define SRC_RENDERER_BUILDER_H_

#include <functional>

#include "src/renderer/renderer.h"
#include "src/renderer/renderers/gl/renderer.h"

namespace renderer {

class Builder {
 public:
  // Add model matrix
  Builder View(std::shared_ptr<Renderable> v) {
    view = v;
    return *this;
  }

  Builder Projection(
      std::function<std::shared_ptr<Renderable>(size_t, size_t)> p) {
    projection = p;
    return *this;
  }

  std::shared_ptr<Renderer> Build() {
    return std::shared_ptr<Renderer>(new gl::Renderer(view, projection));
  }

 private:
  std::shared_ptr<Renderable> view;
  std::function<std::shared_ptr<Renderable>(size_t, size_t)> projection;
};

}  // namespace renderer

#endif  // SRC_RENDERER_BUILDER_H_
