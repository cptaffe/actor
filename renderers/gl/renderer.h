
#ifndef B_RENDERERS_GL_RENDERER_H_
#define B_RENDERERS_GL_RENDERER_H_

#include <atomic>
#include <chrono>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <random>
#include <sstream>
#include <stdexcept>
#include <streambuf>
#include <thread>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "renderers/gl/buffer.h"
#include "renderers/gl/shader.h"
#include "renderers/gl/shapes.h"
#include "renderers/gl/window.h"
#include "renderers/renderer.h"

namespace gl {

class Renderer : public renderer::Renderer {
public:
  Renderer(renderer::Renderable *v, renderer::Renderable *p);
  virtual void AddRenderable(renderer::Rasterizable *r,
                             std::vector<renderer::Renderable *> m) override;
  virtual renderer::shapes::Factory *ShapeFactory() override;
  virtual void Render() override;

private:
  bool alive = true;
  static gl::Window *window;
  std::vector<std::vector<renderer::Renderable *>> model;
  std::vector<Rasterizable *> display;
  renderer::Renderable *view, *projection;
  Program *program;
  GLuint mvpHandle;
};

} // namespace gl

#endif // B_RENDERERS_GL_RENDERER_H_
