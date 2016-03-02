
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
  Renderer(glm::mat4 v, glm::mat4 p)
      : view(([=] {
          // NOTE: first constructor initializer list entry,
          // here we can do work before the other items are initlialized
          if (window == nullptr) {
            window = new gl::Window("basilisk", 400, 400);
            window->Swapiness(0);
          }
          return v;
        })()),
        projection(p), program(([=] {
          auto vshader = std::ifstream("shaders/triangle.vert");
          auto fshader = std::ifstream("shaders/triangle.frag");
          return ProgramBuilder()
              .AddVertexShader(vshader)
              .AddFragmentShader(fshader)
              .Build();
        })()),
        mvpHandle(program->UniformLocation("model_view_projection")),
        elements(GL_ELEMENT_ARRAY_BUFFER) {
    if (display.size() != model.size()) {
      std::stringstream ss;
      ss << "gl::Renderer: Display and model vectors must be the same size, "
         << display.size() << " != " << model.size();
      throw std::runtime_error(ss.str());
    }
  }

  virtual void AddRenderable(renderer::Rasterizable *r,
                             std::vector<renderer::Renderable *> m) override {
    Rasterizable *gr = dynamic_cast<Rasterizable *>(r);
    if (gr == nullptr) {
      throw std::runtime_error("gl::Renderer.AddRenderable: Rasterizable must "
                               "be obtained from gl::Renderer.ShapeFactory "
                               "functions");
    }
    display.push_back(gr);
    model.push_back(m);
  }

  virtual renderer::shapes::Factory *ShapeFactory() override {
    return new shapes::Factory();
  }

  std::vector<glm::mat4> RenderModel() {
    std::vector<glm::mat4> matrices;
    for (auto m : model) {
      std::vector<glm::mat4> matrixes = {glm::mat4(1.0)};
      for (auto r : m) {
        matrixes = r->Apply(matrixes);
      }
      matrices.insert(std::end(matrices), std::begin(matrixes),
                      std::end(matrixes));
    }
    return matrices;
  }

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

  // Render drawables,
  // returns map of drawable's matrices to a vector of indices
  std::map<Rasterizable *, std::vector<int>, vec3less> MapDisplay() {
    std::map<Rasterizable *, std::vector<int>, vec3less> map;
    for (auto i = 0; i < display.size(); i++) {
      map[display[i]].push_back(i);
    }
    return map;
  }

  virtual void Render() override {
    if (!alive) {
      throw std::runtime_error(
          "gl::Renderer: Renderer is dead (possibly because "
          "gl::Window was closed, etc.)");
    }
    // Render items in display

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    auto buffers = std::vector<Buffer<GLfloat> *>({&vertices, &colors});
    auto i = 0;
    for (auto b : buffers) {
      glEnableVertexAttribArray(i);
      b->Bind();
      glVertexAttribPointer(i, 3, GL_FLOAT, false, 0, static_cast<void *>(0));
      i++;
    }
    program->Use();

    // Render and write to vertex mapping
    auto rand = std::bind(std::uniform_real_distribution<double>(0, 1),
                          std::mt19937_64());
    auto models = RenderModel();
    for (auto m : MapDisplay()) {
      std::vector<glm::mat4> mvp;
      elements.Clear();
      for (auto i : m.second) {
        mvp.push_back(projection * view * models[i]);
        elements.Push(i);
      }
      // Create single vertices & color entry

      m.first->Rasterize(&vertices);
      colors.Clear();
      for (auto i = 0; i < vertices.Size(); i++) {
        for (auto j = 0; j < 3; j++) {
          colors.Push(rand());
        }
      }
      std::cout << "vertices: " << vertices << std::endl;
      std::cout << "colors: " << colors << std::endl;
      std::cout << "elements: " << elements << std::endl;
      glUniformMatrix4fv(mvpHandle, mvp.size(), GL_FALSE, &mvp.data()[0][0][0]);
      elements.Bind();
      glDrawElementsInstanced(m.first->Type(), vertices.Size(), GL_UNSIGNED_INT,
                              nullptr, elements.Size());
    }

    for (auto i = 0; i < buffers.size(); i++) {
      glDisableVertexAttribArray(i);
    }

    window->Swap();

    // Stop conditions
    glfwPollEvents();
    if (window->Key(GLFW_KEY_ESCAPE) == GLFW_PRESS || window->ShouldClose()) {
      alive = false;
    }
  }

  Window *Window() { return window; }

private:
  bool alive = true;
  static gl::Window *window;
  std::vector<std::vector<renderer::Renderable *>> model;
  std::vector<Rasterizable *> display;
  glm::mat4 view, projection;
  Program *program;
  GLuint mvpHandle;
  Buffer<GLfloat> vertices, colors;
  Buffer<GLuint> elements;
};

} // namespace gl

#endif // B_RENDERERS_GL_RENDERER_H_
