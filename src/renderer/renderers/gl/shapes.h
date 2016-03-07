// Copyright 2016 Connor Taffe

#ifndef SRC_RENDERER_RENDERERS_GL_SHAPES_H_
#define SRC_RENDERER_RENDERERS_GL_SHAPES_H_

#include <GL/glew.h>

#include <vector>

#include "src/renderer/renderer.h"
#include "src/renderer/renderers/gl/buffer.h"

namespace gl {

class Rasterizable : public renderer::Rasterizable {
 public:
  virtual ~Rasterizable();
  virtual void Rasterize(std::vector<GLfloat> *b) const = 0;
  virtual GLenum Type() = 0;
  friend bool operator<(std::shared_ptr<gl::Rasterizable>,
                        std::shared_ptr<gl::Rasterizable>);
};

namespace shapes {
class Factory : public renderer::shapes::Factory {
 public:
  std::shared_ptr<renderer::Rasterizable> Cube() override;
};

}  // namespace shapes
}  // namespace gl

bool operator<(std::shared_ptr<gl::Rasterizable>,
               std::shared_ptr<gl::Rasterizable>);

#endif  // SRC_RENDERER_RENDERERS_GL_SHAPES_H_
