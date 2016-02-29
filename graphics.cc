
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <atomic>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <random>
#include <stdexcept>
#include <streambuf>
#include <thread>
#include <vector>

namespace graphics {

class Window {
public:
  Window(std::string title, int w, int h) : width(w), height(h) {
    if (!glfwInit()) {
      throw std::runtime_error("glfw initialization failed");
    }

    glfwSetErrorCallback([](int l, const char *msg) {
      throw std::runtime_error("glfw3 error: " + std::string(msg, l));
    });

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (window == nullptr) {
      throw std::runtime_error("glfw create window failed");
    }
    glfwMakeContextCurrent(window);

    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
      throw std::runtime_error("failed to initialize glew");
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
  }

  bool ShouldClose() { return glfwWindowShouldClose(window); }

  int Key(int key) { return glfwGetKey(window, key); }

  void Swap() { glfwSwapBuffers(window); }

  int Width() { return width; }
  int Height() { return height; }

private:
  GLFWwindow *window;
  int width, height;
};

class ProgramBuilder {
public:
  ProgramBuilder() : programHandle(glCreateProgram()) {
    if (!([=] {
          GLboolean support;
          glGetBooleanv(GL_SHADER_COMPILER, &support);
          return support;
        })()) {
      throw std::runtime_error("no shader support");
    }
  }

  ProgramBuilder AddVertexShader(std::istream &src) {
    return AddVertexShader(std::string(std::istreambuf_iterator<char>(src),
                                       std::istreambuf_iterator<char>()));
  }

  ProgramBuilder AddVertexShader(std::string src) {
    shaders.insert({glCreateShader(GL_VERTEX_SHADER), src});
    return *this;
  }

  ProgramBuilder AddFragmentShader(std::istream &src) {
    return AddFragmentShader(std::string(std::istreambuf_iterator<char>(src),
                                         std::istreambuf_iterator<char>()));
  }

  ProgramBuilder AddFragmentShader(std::string src) {
    shaders.insert({glCreateShader(GL_FRAGMENT_SHADER), src});
    return *this;
  }

  GLuint Build() {
    CompileShaders();
    DeleteShaders();
    return programHandle;
  }

private:
  GLuint programHandle;
  std::map<GLuint, std::string> shaders;

  void CompileShaders() {
    // Error checking wrapper function
    auto check = [=](void (*f)(GLuint, GLenum, GLint *),
                     void (*l)(GLenum, GLsizei, GLsizei *, GLchar *),
                     GLenum status, GLuint handle, std::string err) {
      GLint res;
      f(handle, status, &res);
      int ll;
      f(handle, GL_INFO_LOG_LENGTH, &ll);
      if (res == GL_FALSE) {
        std::vector<char> v(ll + 1);
        l(handle, ll + 1, &ll, v.data());
        throw std::runtime_error(err + ": " +
                                 std::string(reinterpret_cast<const char *>(
                                     gluErrorString(glGetError()))) +
                                 ", " + std::string(v.begin(), v.end()));
      }
    };

    for (auto s : shaders) {
      auto sc = s.second.c_str();
      glShaderSource(s.first, 1, &sc, nullptr);
      glCompileShader(s.first);
      check(glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS, s.first,
            "error compiling shader");
      glAttachShader(programHandle, s.first);
    }

    glLinkProgram(programHandle);
    check(glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS, programHandle,
          "error linking program");
  }

  void DeleteShaders() {
    for (auto s : shaders) {
      glDetachShader(programHandle, s.first);
      glDeleteShader(s.first);
    }
  }
};

class Timer {
public:
  Timer() {}

  std::chrono::duration<double> Since() { return stop - last; }

  static void Start() { last = std::chrono::high_resolution_clock::now(); }
  static void Stop() { stop = std::chrono::high_resolution_clock::now(); }

private:
  static std::chrono::time_point<std::chrono::high_resolution_clock> stop;
  static std::chrono::time_point<std::chrono::high_resolution_clock> last;
};

class Renderable {
public:
  virtual ~Renderable(){};

  virtual std::vector<glm::mat4> Render() = 0;

  std::vector<glm::mat4> Apply(std::vector<glm::mat4> m) {
    std::vector<glm::mat4> nm;
    auto rendering = Render();
    for (auto n : m) {
      for (auto r : rendering) {
        nm.push_back(n * r);
      }
    }
    return nm;
  }
};

class Renderer {
public:
  Renderer(Window w, std::vector<std::vector<Renderable *>> m,
           std::vector<Renderable *> d, glm::mat4 v, glm::mat4 p)
      : window(w), model(m), display(d), view(v), projection(p), program(([=] {
          auto vshader = std::ifstream("shaders/triangle.vert");
          auto fshader = std::ifstream("shaders/triangle.frag");
          return graphics::ProgramBuilder()
              .AddVertexShader(vshader)
              .AddFragmentShader(fshader)
              .Build();
        })()),
        mvpHandle(glGetUniformLocation(program, "model_view_projection")),
        vertexArrayHandle(([] {
          GLuint vaID;
          glGenVertexArrays(1, &vaID);
          return vaID;
        })()),
        buffers({

            ([=] {
              glBindVertexArray(vertexArrayHandle);
              GLuint buf;
              glGenBuffers(1, &buf);
              glBindBuffer(GL_ARRAY_BUFFER, buf);
              glBufferData(GL_ARRAY_BUFFER,
                           sizeof(GLfloat) * RenderDisplay().size() * 12,
                           nullptr, GL_STATIC_DRAW);
              return buf;
            })(),
            ([=] {
              glBindVertexArray(vertexArrayHandle);
              GLuint buf;
              glGenBuffers(1, &buf);
              glBindBuffer(GL_ARRAY_BUFFER, buf);
              glBufferData(GL_ARRAY_BUFFER,
                           sizeof(GLfloat) * RenderDisplay().size() * 12,
                           nullptr, GL_STATIC_DRAW);
              return buf;
            })(),
            ([=] {
              glBindVertexArray(vertexArrayHandle);
              GLuint buf;
              glGenBuffers(1, &buf);
              glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf);
              glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                           sizeof(GLfloat) * RenderDisplay().size() * 3,
                           nullptr, GL_STATIC_DRAW);
              return buf;
            })()}),
        vertices(([=] {
          glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
          auto p = static_cast<GLfloat *>(
              glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));
          if (p == nullptr) {
            throw std::runtime_error("glMapBuffer error: " +
                                     std::string(reinterpret_cast<const char *>(
                                         gluErrorString(glGetError()))));
          }
          return p;
        })()),
        colors(([=] {
          glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
          auto p = static_cast<GLfloat *>(
              glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));
          if (p == nullptr) {
            throw std::runtime_error("glMapBuffer error: " +
                                     std::string(reinterpret_cast<const char *>(
                                         gluErrorString(glGetError()))));
          }
          return p;
        })()),
        elements(([=] {
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[2]);
          auto p = static_cast<GLuint *>(
              glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_WRITE));
          if (p == nullptr) {
            throw std::runtime_error("glMapBuffer error: " +
                                     std::string(reinterpret_cast<const char *>(
                                         gluErrorString(glGetError()))));
          }
          return p;
        })()) {
    if (display.size() != model.size()) {
      throw std::runtime_error(
          "display and model arrays must be the same size");
    }
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

  std::vector<glm::mat4> RenderDisplay() {
    std::vector<glm::mat4> matrices;
    for (auto d : display) {
      auto matrixes = d->Render();
      matrices.insert(std::end(matrices), std::begin(matrixes),
                      std::end(matrixes));
    }
    return matrices;
  }

  class Mat4Less {
  public:
    bool operator()(std::vector<glm::mat4> a, std::vector<glm::mat4> b) {
      if (a.size() != b.size()) {
        return a.size() > b.size();
      }
      for (auto i = 0; i < a.size(); i++) {
        for (auto j = 0; j < 4; j++) {
          for (auto k = 0; k < 4; k++) {
            if (a[i][j][k] != b[i][j][k]) {
              return a[i][j][k] > b[i][j][k];
            }
          }
        }
      }
      return false;
    }
  };

  // Render drawables,
  // returns map of drawable's matrices to a vector of indices
  std::map<std::vector<glm::mat4>, std::vector<int>, Mat4Less> MapDisplay() {
    std::map<std::vector<glm::mat4>, std::vector<int>, Mat4Less> map;
    for (auto i = 0; i < display.size(); i++) {
      map[display[i]->Render()].push_back(i);
    }
    return map;
  }

  void Render() {
    // Render items in display

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (auto i = 0; i < buffers.size(); i++) {
      glEnableVertexAttribArray(i);
      glBindBuffer(GL_ARRAY_BUFFER, buffers[i]);
      glVertexAttribPointer(i, 4, GL_FLOAT, false, 0, static_cast<void *>(0));
    }
    glUseProgram(program);

    // Render and write to vertex mapping
    auto rand = std::bind(std::uniform_real_distribution<double>(0, 1),
                          std::mt19937_64());
    auto models = RenderModel();
    for (auto m : MapDisplay()) {
      std::vector<glm::mat4> mvp;
      auto j = 0;
      for (auto i : m.second) {
        mvp.push_back(projection * view * models[i]);
        elements[j++] = i;
      }
      auto color = glm::vec4(rand(), rand(), rand(), 1);
      for (auto i = 0; i < m.first.size(); i++) {
        for (auto j = 0; j < 4; j++) {
          for (auto k = 0; k < 4; k++) {
            vertices[(i * 12) + (j * 4) + k] = m.first[i][j][k];
            colors[(i * 12) + (j * 4) + k] = color[k];
          }
        }
      }
      glUniformMatrix4fv(mvpHandle, mvp.size(), GL_FALSE, &mvp.data()[0][0][0]);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[2]);
      glDrawElementsInstanced(GL_TRIANGLES, m.first.size() * 4, GL_UNSIGNED_INT,
                              nullptr, m.second.size());
    }

    for (auto i = 0; i < buffers.size(); i++) {
      glDisableVertexAttribArray(i);
    }

    window.Swap();
  }

  Window Window() { return window; }

private:
  // Must initialize first!
  graphics::Window window;
  std::vector<std::vector<Renderable *>> model;
  std::vector<Renderable *> display;
  glm::mat4 view, projection;
  GLuint program, mvpHandle;
  GLuint vertexArrayHandle;
  std::vector<GLuint> buffers;
  GLfloat *vertices, *colors;
  GLuint *elements;
};

} // namespace graphics

class Scale : public graphics::Renderable {
public:
  Scale(glm::vec3 s) : scale(s) {}
  virtual std::vector<glm::mat4> Render() override {
    return {glm::scale(scale)};
  }

private:
  glm::vec3 scale;
};

class Translate : public graphics::Renderable {
public:
  Translate(glm::vec3 t) : translation(glm::translate(t)) {}
  virtual std::vector<glm::mat4> Render() override { return {translation}; }

private:
  glm::mat4 translation;
};

class Rotate : public graphics::Renderable {
public:
  Rotate(double a, glm::vec3 v) : angle(a), vec(v) {}
  virtual std::vector<glm::mat4> Render() override {
    return {glm::rotate(static_cast<float>(angle), vec)};
  }

private:
  double angle;
  glm::vec3 vec;
};

class Triangle : public graphics::Renderable {
public:
  Triangle(glm::vec3 a, glm::vec3 b, glm::vec3 c)
      : vertices(glm::vec4(a, 1), glm::vec4(b, 1), glm::vec4(c, 1),
                 glm::vec4(1)) {}
  virtual std::vector<glm::mat4> Render() override { return {vertices}; }

private:
  glm::mat4 vertices;
};

class EquilateralTriangle : public graphics::Renderable {
public:
  EquilateralTriangle()
      : triangle(glm::vec3(1, -1, 0), glm::vec3(-1, -1, 0),
                 glm::vec3(0, 1, 0)) {}
  virtual std::vector<glm::mat4> Render() override { return triangle.Render(); }

private:
  Triangle triangle;
};

class Square : public graphics::Renderable {
public:
  Square()
      : triangles(([=] {
          auto triangles = std::vector<Triangle>(
              {{glm::vec3(1, -1, 0), glm::vec3(-1, 1, 0), glm::vec3(1, 1, 0)},
               {glm::vec3(1, -1, 0), glm::vec3(-1, 1, 0),
                glm::vec3(-1, -1, 0)}});
          std::vector<glm::mat4> v;
          for (auto t : triangles) {
            for (auto m : t.Render()) {
              v.push_back(m);
            }
          }
          return v;
        })()) {}
  virtual std::vector<glm::mat4> Render() override { return triangles; }

private:
  std::vector<glm::mat4> triangles;
};

class Cube : public graphics::Renderable {

public:
  Cube()
      : sides(([=] {
          auto s0 = Square().Apply(Translate({0, 0, 1}).Render());
          auto s1 = Square().Apply(Translate({0, 0, 1}).Apply(
              Rotate(0.5 * glm::pi<double>(), glm::vec3(0, 1, 0)).Render()));

          auto s2 = Square().Apply(Translate({0, 0, 1}).Apply(
              Rotate(-0.5 * glm::pi<double>(), glm::vec3(0, 1, 0)).Render()));
          auto s3 = Square().Apply(Translate({0, 0, -1}).Render());
          auto s4 = Square().Apply(Translate({0, 0, 1}).Apply(
              Rotate(0.5 * glm::pi<double>(), glm::vec3(1, 0, 0)).Render()));
          auto s5 = Square().Apply(Translate({0, 0, 1}).Apply(
              Rotate(-0.5 * glm::pi<double>(), glm::vec3(1, 0, 0)).Render()));
          s0.insert(std::end(s0), std::begin(s1), std::end(s1));
          s0.insert(std::end(s0), std::begin(s2), std::end(s2));
          s0.insert(std::end(s0), std::begin(s3), std::end(s3));
          s0.insert(std::end(s0), std::begin(s4), std::end(s4));
          s0.insert(std::end(s0), std::begin(s5), std::end(s5));
          return s0;
        })()) {}
  virtual std::vector<glm::mat4> Render() override { return sides; }

private:
  std::vector<glm::mat4> sides;
};

class Float : public graphics::Renderable {

public:
  Float(double rad, std::chrono::duration<double> d)
      : radius(rad), duration(d) {}
  virtual std::vector<glm::mat4> Render() override {
    return Translate(glm::vec3(0, radius * glm::cos(timer.Since() / duration *
                                                    2 * glm::pi<double>()),
                               0))
        .Render();
  }

private:
  double radius;
  std::chrono::duration<double> duration;
  graphics::Timer timer;
};

class Spin : public graphics::Renderable {

public:
  Spin(std::chrono::duration<double> d) : duration(d) {}
  virtual std::vector<glm::mat4> Render() {
    return Rotate(timer.Since() / duration * 2 * glm::pi<double>(),
                  glm::vec3(0, 1, 0))
        .Render();
  }

private:
  std::chrono::duration<double> duration;
  graphics::Timer timer;
};

std::chrono::time_point<std::chrono::high_resolution_clock>
    graphics::Timer::stop;
std::chrono::time_point<std::chrono::high_resolution_clock>
    graphics::Timer::last;

int main() {
  graphics::Window w("basilisk", 400, 400);
  auto cubes = 200;
  graphics::Renderer renderer(
      w, ([=] {
        std::mt19937_64 random;
        auto rand =
            std::bind(std::uniform_real_distribution<double>(-5, 5), random);
        std::vector<std::vector<graphics::Renderable *>> vec;
        auto cube = [&] {
          return std::vector<graphics::Renderable *>(
              {new Translate({rand(), rand(), rand()}),
               new Scale({0.25, 0.25, 0.25}),
               new Float(0.25, std::chrono::milliseconds(
                                   std::uniform_int_distribution<long>(
                                       1000, 5000)(random))),
               new Spin(std::chrono::milliseconds(
                   std::uniform_int_distribution<long>(1000, 6000)(random)))});
        };
        for (auto i = 0; i < cubes; i++) {
          vec.push_back(cube());
        }
        return vec;
      })(),
      ([=] {
        std::vector<graphics::Renderable *> v;
        for (auto i = 0; i < cubes; i++) {
          v.push_back(new Cube());
        }
        return v;
      })(),
      glm::lookAt(glm::vec3(4, 3, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)),
      glm::perspective(0.5 * glm::pi<double>(),
                       static_cast<double>(w.Width()) /
                           static_cast<double>(w.Height()),
                       0.1, 100.0));

  graphics::Timer::Start();
  for (;;) {
    graphics::Timer::Stop();
    renderer.Render();
    glfwPollEvents();
    if (renderer.Window().Key(GLFW_KEY_ESCAPE) == GLFW_PRESS ||
        renderer.Window().ShouldClose()) {
      break;
    }
  }
}
