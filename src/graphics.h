// Copyright 2016 Connor Taffe

#ifndef SRC_GRAPHICS_H_
#define SRC_GRAPHICS_H_

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <chrono>
#include <functional>
#include <random>
#include <sstream>
#include <vector>

#include "src/actor.h"
#include "src/events.h"
#include "src/renderer/builder.h"
#include "src/renderer/renderer.h"

namespace renderables {

class Scale : public renderer::Renderable {
 public:
  explicit Scale(glm::vec3 s);
  std::vector<glm::mat4> Render() const override;

 private:
  glm::vec3 scale;
};

class Translate : public renderer::Renderable {
 public:
  explicit Translate(glm::vec3 t);
  std::vector<glm::mat4> Render() const override;

 private:
  glm::mat4 translation;
};

class Rotate : public renderer::Renderable {
 public:
  Rotate(double a, glm::vec3 v);
  std::vector<glm::mat4> Render() const override;

 private:
  double angle;
  glm::vec3 vec;
};

class Float : public renderer::Renderable {
 public:
  Float(double rad, std::chrono::duration<double> d);
  std::vector<glm::mat4> Render() const override;

 private:
  double radius;
  std::chrono::duration<double> duration;
};

class Spin : public renderer::Renderable {
 public:
  explicit Spin(std::chrono::duration<double> d);
  std::vector<glm::mat4> Render() const override;

 private:
  std::chrono::duration<double> duration;
};

class MatRenderable : public renderer::Renderable {
 public:
  explicit MatRenderable(glm::mat4 m);
  std::vector<glm::mat4> Render() const override { return {matrix}; }

 private:
  glm::mat4 matrix;
};

}  // namespace renderables

class Fps {
 public:
  Fps();
  friend std::ostream &operator<<(const Fps &fps, std::ostream &o);
  void Update();
  double Frame() const;

 private:
  std::chrono::time_point<std::chrono::high_resolution_clock> ticks;
  std::chrono::duration<double> diff;
};

std::ostream &operator<<(std::ostream &o, const Fps &fps);

#endif  // SRC_GRAPHICS_H_
