// Copyright 2016 Connor Taffe

#ifndef SRC_RENDERER_RENDERERS_GL_RENDERER_H_
#define SRC_RENDERER_RENDERERS_GL_RENDERER_H_

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "src/renderer/renderer.h"
#include "src/renderer/renderers/gl/shader.h"
#include "src/renderer/renderers/gl/shapes.h"
#include "src/renderer/renderers/gl/window.h"

namespace gl {

class RenderPass {
 public:
  RenderPass(std::shared_ptr<Rasterizable> rend, std::vector<size_t> ind,
             std::shared_ptr<renderer::Renderable> view,
             std::shared_ptr<renderer::Renderable> projection,
             const std::vector<
                 std::vector<std::shared_ptr<renderer::Renderable>>> &model,
             GLint h);
  void Render();

 private:
  std::shared_ptr<Rasterizable> rasterizable;
  std::vector<size_t> indices;
  std::vector<GLfloat> colors, vertices;
  GLint mvpHandle;
  std::vector<glm::mat4> mvp;
};

class RenderThread {
 public:
  RenderThread(
      std::function<std::vector<RenderPass>(Window *, GLint)> renderFunc);
  [[noreturn]] void Run();

 private:
  gl::Window window;
  Program program;
  GLint mvpHandle;
  std::function<std::vector<RenderPass>(Window *, GLint)> renderFunc;

  void Render(std::vector<RenderPass> renders);
};

class Renderer : public renderer::Renderer {
 public:
  Renderer(
      std::shared_ptr<renderer::Renderable> v,
      std::function<std::shared_ptr<renderer::Renderable>(size_t, size_t)>);
  std::unique_ptr<renderer::shapes::Factory> ShapeFactory() override;
  void Render() override {}
  void Handle(std::shared_ptr<Event> const e) override;

 private:
  std::shared_ptr<renderer::Renderable> view;
  std::function<std::shared_ptr<renderer::Renderable>(size_t w, size_t h)>
      projection;
  std::vector<std::vector<std::shared_ptr<renderer::Renderable>>> model;
  std::mutex displayLock;
  std::condition_variable displayCondition;
  std::vector<std::shared_ptr<Rasterizable>> display;
  std::thread renderThread;
  RenderThread *renderer = nullptr;
};

}  // namespace gl

#endif  // SRC_RENDERER_RENDERERS_GL_RENDERER_H_
