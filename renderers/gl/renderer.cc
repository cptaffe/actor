
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
#include "renderers/gl/renderer.h"
#include "renderers/gl/shader.h"
#include "renderers/gl/shapes.h"
#include "renderers/gl/window.h"
#include "renderers/renderer.h"

namespace gl {
namespace {

class vec3less {
public:
  bool operator()(Rasterizable *ra, Rasterizable *rb) {
    std::vector<glm::vec3> a = ra->Vertices();
    std::vector<glm::vec3> b = rb->Vertices();
    if (a.size() != b.size()) {
      return a.size() > b.size();
    }
    return memcmp(&a.data()[0][0], &b.data()[0][0],
                  a.size() * 3 * sizeof(float)) < 0;
  }
};

class RenderPass {
public:
  RenderPass(Rasterizable *rasterizable, std::vector<int> indices,
             renderer::Renderable *view, renderer::Renderable *projection,
             std::vector<std::vector<renderer::Renderable *>> &model,
             GLuint mvpHandle)
      : _rasterizable{rasterizable}, _indices{indices},
        _colors{Buffer<GLfloat>()}, _vertices{Buffer<GLfloat>()},
        _mvpHandle{mvpHandle} {
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
        _mvp.push_back(p * v * m);
      }
    }
  }

  template <typename T> class VertexAttributePointer {
  public:
    VertexAttributePointer(Buffer<T> *b, GLuint i, size_t size, GLenum type,
                           std::ptrdiff_t stride)
        : buffer{b}, handle{i} {
      buffer->Bind();
      // TODO: figure out type from T
      glVertexAttribPointer(i, size, type, false, stride, nullptr);
      glEnableVertexAttribArray(i);
    }
    VertexAttributePointer(const VertexAttributePointer &other) = delete;
    ~VertexAttributePointer() { glDisableVertexAttribArray(handle); }

  private:
    Buffer<T> *buffer;
    GLuint handle;
  };

  void Render() {
    // buffer all vertices & colors before drawd
    auto buffers = std::vector<Buffer<GLfloat> *>({&_vertices, &_colors});
    auto vptrs =
        std::vector<std::shared_ptr<VertexAttributePointer<GLfloat>>>();
    {
      for (auto i = 0; i < buffers.size(); i++) {
        vptrs.push_back(std::shared_ptr<VertexAttributePointer<GLfloat>>(
            new VertexAttributePointer<GLfloat>(buffers[i], i, 3, GL_FLOAT,
                                                0)));
      }
    }

    glUniformMatrix4fv(_mvpHandle, _mvp.size(), false, &_mvp.data()[0][0][0]);
    // TODO: why does order matter here?
    auto rand = std::bind(std::uniform_real_distribution<double>(0, 1),
                          std::mt19937_64());
    _colors.Clear();
    for (auto i = 0; i < _vertices.Size(); i++) {
      for (auto j = 0; j < 3; j++) {
        _colors.Push(rand());
      }
    }
    _colors.Write();
    _vertices.Clear();
    _rasterizable->Rasterize(&_vertices);
    _vertices.Write();
    glDrawArraysInstanced(_rasterizable->Type(), 0,
                          _vertices.Size() * sizeof(GLfloat), _indices.size());
  }

private:
  Rasterizable *_rasterizable;
  std::vector<int> _indices;
  Buffer<GLfloat> _colors, _vertices;
  GLuint _mvpHandle;
  std::vector<glm::mat4> _mvp;
};

} // namespace

Renderer::Renderer(renderer::Renderable *v, renderer::Renderable *p)
    : view{([=] {
        // NOTE: first constructor initializer list entry,
        // here we can do work before the other items are initlialized
        if (window == nullptr) {
          window = new gl::Window("basilisk", 400, 400);
          window->Swapiness(0);
        }
        return v;
      })()},
      projection{p}, program{([=] {
        auto vshader = std::ifstream("shaders/triangle.vert");
        auto fshader = std::ifstream("shaders/triangle.frag");
        return ProgramBuilder()
            .AddVertexShader(vshader)
            .AddFragmentShader(fshader)
            .Build();
      })()},
      mvpHandle{program->UniformLocation("model_view_projection")} {
  if (display.size() != model.size()) {
    throw std::runtime_error(static_cast<std::stringstream &>(
                                 std::stringstream()
                                 << "gl::Renderer: Display and model vectors "
                                    "must be the same size, "
                                 << display.size() << " != " << model.size())
                                 .str());
  }
}

void Renderer::AddRenderable(renderer::Rasterizable *r,
                             std::vector<renderer::Renderable *> m) {
  display.push_back(([=] {
    Rasterizable *gr{dynamic_cast<Rasterizable *>(r)};
    if (gr == nullptr) {
      throw std::runtime_error("gl::Renderer.AddRenderable: Rasterizable must "
                               "be obtained from gl::Renderer.ShapeFactory "
                               "functions");
    }
    return gr;
  })());
  model.push_back(m);
}

renderer::shapes::Factory *Renderer::ShapeFactory() {
  return new shapes::Factory();
}

void Renderer::Render() {
  if (!alive) {
    throw std::runtime_error("gl::Renderer: Renderer is dead (possibly because "
                             "gl::Window was closed, etc.)");
  }

  // pre-rendering
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  program->Use();

  // Do rendering
  for (auto m : ([&] {
         std::map<Rasterizable *, std::vector<int>, vec3less> map;
         for (auto i = 0; i < display.size(); i++) {
           map[display[i]].push_back(i);
         }
         return map;
       })()) {
    RenderPass(m.first, m.second, view, projection, model, mvpHandle).Render();
  }

  window->Swap();
  glfwPollEvents();
  if (window->Key(GLFW_KEY_ESCAPE) == GLFW_PRESS || window->ShouldClose()) {
    alive = false;
  }
}

} // gl

gl::Window *gl::Renderer::window;
