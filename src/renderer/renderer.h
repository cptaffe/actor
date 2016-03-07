// Copyright 2016 Connor Taffe

#ifndef SRC_RENDERER_RENDERER_H_
#define SRC_RENDERER_RENDERER_H_

#include <glm/glm.hpp>
#include <vector>

#include "src/base.h"
#include "src/renderer/timer.h"

namespace renderer {

class Rasterizable {
 public:
  virtual ~Rasterizable();
  virtual std::vector<double> Vertices() const = 0;
};

// Renderable interface
class Renderable {
 public:
  virtual ~Renderable();
  virtual std::vector<glm::mat4> Render() const = 0;

  std::vector<glm::mat4> Apply(std::vector<glm::mat4> m);
  std::vector<glm::mat4> Apply(Renderable *r);
};

namespace shapes {

class Factory {
 public:
  virtual ~Factory();
  virtual std::shared_ptr<Rasterizable> Cube() = 0;
};

}  // namespace shapes

// Renderer interface
class Renderer : public Actor {
 public:
  virtual ~Renderer();
  virtual void Render() = 0;
  virtual std::unique_ptr<shapes::Factory> ShapeFactory() = 0;
};

}  // namespace renderer

#endif  // SRC_RENDERER_RENDERER_H_
