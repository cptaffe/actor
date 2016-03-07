// Copyright 2016 Connor Taffe

#include "src/renderer/renderers/gl/renderer.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <stdexcept>
#include <vector>

#include "src/renderer/event/event.h"
#include "src/renderer/renderer.h"
#include "src/renderer/renderers/gl/buffer.h"
#include "src/renderer/renderers/gl/shader.h"
#include "src/renderer/renderers/gl/shapes.h"
#include "src/renderer/renderers/gl/window.h"

namespace gl {
namespace {

template <typename T>
class VertexAttributePointer {
 public:
  VertexAttributePointer(Buffer<T> *b, GLuint i, size_t size, GLenum type,
                         std::ptrdiff_t stride)
      : buffer{b}, handle{i} {
    buffer->Bind();
    // TODO(cptaffe): figure out type from T
    glVertexAttribPointer(i, size, type, false, stride, nullptr);
    glEnableVertexAttribArray(i);
  }
  VertexAttributePointer(const VertexAttributePointer &other) = delete;
  ~VertexAttributePointer() { glDisableVertexAttribArray(handle); }

 private:
  Buffer<T> *buffer;
  GLuint handle;
};

}  // namespace

RenderPass::RenderPass(
    std::shared_ptr<Rasterizable> rend, std::vector<size_t> ind,
    std::shared_ptr<renderer::Renderable> view,
    std::shared_ptr<renderer::Renderable> projection,
    const std::vector<std::vector<std::shared_ptr<renderer::Renderable>>>
        &model,
    GLint h)
    : rasterizable{rend}, indices{ind}, mvpHandle{h} {
  auto v = glm::mat4(1.0), p = glm::mat4(1.0);
  for (auto i : view->Render()) {
    v *= i;
  }
  for (auto i : projection->Render()) {
    p *= i;
  }
  for (auto i : indices) {
    for (auto m : ([&] {
           std::vector<glm::mat4> matrices = {glm::mat4(1.0)};
           for (auto r : model[i]) {
             matrices = r->Apply(matrices);
           }
           return matrices;
         })()) {
      mvp.push_back(p * v * m);
    }
  }
}

void RenderPass::Render() {
  glUniformMatrix4fv(mvpHandle, mvp.size(), false, &mvp.data()[0][0][0]);
  auto random = std::bind(std::uniform_real_distribution<GLfloat>(0, 1),
                          std::mt19937_64());
  vertices.clear();
  rasterizable->Rasterize(&vertices);
  colors.clear();
  for (size_t i = 0; i < vertices.size(); i++) {
    for (size_t j = 0; j < 3; j++) {
      colors.push_back(random());
    }
  }

  auto cb = Buffer<GLfloat>(colors), vb = Buffer<GLfloat>(vertices);

  auto vptrs = std::vector<std::shared_ptr<VertexAttributePointer<GLfloat>>>();
  {
    auto buffers = std::vector<Buffer<GLfloat> *>({&vb, &cb});
    for (size_t i = 0; i < buffers.size(); i++) {
      vptrs.push_back(std::shared_ptr<VertexAttributePointer<GLfloat>>(
          new VertexAttributePointer<GLfloat>(
              buffers[i], static_cast<GLuint>(i), 3, GL_FLOAT, 0)));
    }
  }

  glDrawArraysInstanced(rasterizable->Type(), 0, vb.Size() * sizeof(GLfloat),
                        indices.size());
}

RenderThread::RenderThread(
    std::function<std::vector<RenderPass>(Window *, GLint)> renderf)
    : window{"basilisk", 400, 400},
      program{([=] {
        auto b = window.Bind();  // bind gl for scope
        auto vshader =
            std::ifstream("src/renderer/renderers/gl/shaders/triangle.vert");
        auto fshader =
            std::ifstream("src/renderer/renderers/gl/shaders/triangle.frag");
        return *ProgramBuilder()
                    .AddVertexShader(&vshader)
                    .AddFragmentShader(&fshader)
                    .Build();
      })()},
      mvpHandle{([&] {
        auto b = window.Bind();
        return program.UniformLocation("model_view_projection");
      })()},
      renderFunc(renderf) {
  window.Swapiness(0);
}

void RenderThread::Run() {
  for (;;) {
    Render(renderFunc(&window, mvpHandle));
  }
}

void RenderThread::Render(std::vector<RenderPass> renders) {
  auto b = window.Bind();

  // pre-rendering
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  program.Use();

  for (auto &r : renders) {
    r.Render();
  }

  window.Swap();
  glfwPollEvents();
  if (window.Key(GLFW_KEY_ESCAPE) == GLFW_PRESS || window.ShouldClose()) {
    throw std::runtime_error(
        "gl::Renderer: Renderer is dead (possibly because "
        "gl::Window was closed, etc.)");
  }
}

Renderer::Renderer(
    std::shared_ptr<renderer::Renderable> v,
    std::function<std::shared_ptr<renderer::Renderable>(size_t w, size_t h)> p)
    : view{v}, projection{p}, renderThread{[&] {
        renderer::Timer::Start();
        renderer = new RenderThread{[&](Window *win, GLint mvpHandle) {
          renderer::Timer::Instance()->Stop();
          // Do rendering
          std::vector<RenderPass> renders;
          {
            std::unique_lock<std::mutex> lock(displayLock);
            displayCondition.wait(lock, [&] { return display.size() > 0; });
            for (auto m : ([&] {
                   std::map<std::shared_ptr<Rasterizable>, std::vector<size_t>>
                       map;
                   for (size_t i = 0; i < display.size(); i++) {
                     map[display[i]].push_back(i);
                   }
                   return map;
                 })()) {
              renders.push_back({m.first, m.second, view,
                                 projection(static_cast<uint>(win->Width()),
                                            static_cast<uint>(win->Height())),
                                 model, mvpHandle});
            }
          }
          return renders;
        }};
        renderer->Run();
      }} {
  if (display.size() != model.size()) {
    throw std::runtime_error(static_cast<std::stringstream &>(
                                 std::stringstream()
                                 << "gl::Renderer: Display and model vectors "
                                    "must be the same size, "
                                 << display.size() << " != " << model.size())
                                 .str());
  }
}

std::unique_ptr<renderer::shapes::Factory> Renderer::ShapeFactory() {
  return std::unique_ptr<renderer::shapes::Factory>(new shapes::Factory());
}

void Renderer::Handle(std::shared_ptr<Event> const e) {
  ([&](std::shared_ptr<event::Spawn> spawn) {
    if (spawn != nullptr) {
      auto d = std::dynamic_pointer_cast<Rasterizable>(spawn->Display());
      if (d == nullptr) {
        // TODO(cptaffe): throw rejection event
        throw std::runtime_error(
            "gl::Renderer.Handle: Spawn event contains .Display "
            "renderer::Rasterizable which does not "
            "inherit from gl::Rasterizable");
      }
      std::unique_lock<std::mutex> lock(displayLock);
      model.push_back(spawn->Model());
      display.push_back(std::shared_ptr<Rasterizable>(d));
      displayCondition.notify_one();
    }
  })(std::dynamic_pointer_cast<event::Spawn>(e));
}

}  // namespace gl
