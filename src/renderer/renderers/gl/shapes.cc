// Copyright 2016 Connor Taffe

#include "src/renderer/renderers/gl/shapes.h"

#include <algorithm>
#include <cstring>
#include <memory>
#include <vector>

bool operator<(std::shared_ptr<gl::Rasterizable> ra,
               std::shared_ptr<gl::Rasterizable> rb) {
  auto a = ra->Vertices();
  auto b = rb->Vertices();
  if (a.size() != b.size()) {
    return a.size() > b.size();
  }
  // HACK #jank
  return std::memcmp(a.data(), b.data(), a.size() * sizeof(float)) < 0;
}

namespace gl {

Rasterizable::~Rasterizable() {}

namespace {

class Cube : public Rasterizable {
 public:
  void Rasterize(std::vector<GLfloat> *b) const override {
    auto v = *b;
    std::transform(verts.begin(), verts.end(), std::back_inserter(v),
                   [](double a) -> GLfloat { return static_cast<GLfloat>(a); });
    *b = v;
  }
  GLenum Type() override { return GL_TRIANGLE_STRIP; }
  std::vector<double> Vertices() const override { return verts; }

 private:
  const std::vector<double> verts = {// Front face
                                     -1, -1, 1, 1, -1, 1, -1, 1, 1, 1, 1, 1,
                                     // Right face
                                     1, -1, 1, 1, -1, -1, 1, 1, 1, 1, 1, -1,
                                     // Back face
                                     1, -1, -1, -1, -1, -1, 1, 1, -1, -1, 1, -1,
                                     // Left face
                                     -1, -1, -1, -1, -1, 1, -1, 1, -1, -1, 1, 1,
                                     // Bottom face
                                     -1, -1, -1, 1, -1, -1, -1, -1, 1, 1, -1, 1,
                                     // Top face
                                     -1, 1, 1, 1, 1, 1, -1, 1, -1, 1, 1, -1};
};

}  // namespace

namespace shapes {

std::shared_ptr<renderer::Rasterizable> Factory::Cube() {
  return std::shared_ptr<renderer::Rasterizable>(new class Cube());
}

}  // namespace shapes
}  // namespace gl
