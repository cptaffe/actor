
#ifndef B_RENDERERS_TRIANGLE_H_
#define B_RENDERERS_TRIANGLE_H_

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "renderers/renderer.h"

namespace transforms {
class Scale : public renderer::Rasterizable {
public:
  Scale(glm::vec3 s) : scale(s) {}
  virtual std::vector<glm::mat4> Rasterize() override {
    return {glm::scale(scale)};
  }

private:
  glm::vec3 scale;
};

class Translate : public renderer::Rasterizable {
public:
  Translate(glm::vec3 t) : translation(glm::translate(t)) {}
  virtual std::vector<glm::mat4> Rasterize() override { return {translation}; }

private:
  glm::mat4 translation;
};

class Rotate : public renderer::Rasterizable {
public:
  Rotate(double a, glm::vec3 v) : angle(a), vec(v) {}
  virtual std::vector<glm::mat4> Rasterize() override {
    return {glm::rotate(static_cast<float>(angle), vec)};
  }

private:
  double angle;
  glm::vec3 vec;
};
} // namespace

class Triangle : public renderer::Rasterizable {
public:
  Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c)
      : vertices(glm::vec4(a, 1), glm::vec4(b, 1), glm::vec4(c, 1),
                 glm::vec4(1)) {}
  virtual std::vector<glm::mat4> Rasterize() override { return {vertices}; }

private:
  glm::mat4 vertices;
};

#endif // B_RENDERERS_TRIANGLE_H_
