
#ifndef B_RENDERERS_GL_SHAPES_H_
#define B_RENDERERS_GL_SHAPES_H_

// Shapes defines gl primitives that create the

#include <GL/glew.h>

#include "renderers/gl/buffer.h"
#include "renderers/renderer.h"

namespace gl {

class Rasterizable : public renderer::Rasterizable {
public:
  virtual ~Rasterizable() {}
  virtual void Rasterize(Buffer<GLfloat> *b) = 0;
  virtual GLenum Type() = 0;
};

namespace {

class Cube : public Rasterizable {
public:
  virtual void Rasterize(Buffer<GLfloat> *b) override {
    // write vector of vec3's to buffer
    b->Fill(verts);
  }
  virtual GLenum Type() override { return GL_TRIANGLE_STRIP; };
  virtual std::vector<glm::vec3> Vertices() override { return verts; }

private:
  std::vector<glm::vec3> verts = {// Front face
                                  {-1, -1, 1},
                                  {1, -1, 1},
                                  {-1, 1, 1},
                                  {1, 1, 1},
                                  // Right face
                                  {1, -1, 1},
                                  {1, -1, -1},
                                  {1, 1, 1},
                                  {1, 1, -1},
                                  // Back face
                                  {1, -1, -1},
                                  {-1, -1, -1},
                                  {1, 1, -1},
                                  {-1, 1, -1},
                                  // Left face
                                  {-1, -1, -1},
                                  {-1, -1, 1},
                                  {-1, 1, -1},
                                  {-1, 1, 1},
                                  // Bottom face
                                  {-1, -1, -1},
                                  {1, -1, -1},
                                  {-1, -1, 1},
                                  {1, -1, 1},
                                  // Top face
                                  {-1, 1, 1},
                                  {1, 1, 1},
                                  {-1, 1, -1},
                                  {1, 1, -1}};
};

} // namespace

namespace shapes {
class Factory : public renderer::shapes::Factory {
public:
  virtual renderer::Rasterizable *Cube() override { return new class Cube(); }
};

} // namespace shapes
} // namespace gl

#endif // B_RENDERERS_GL_SHAPES_H_
