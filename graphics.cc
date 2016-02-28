
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

  std::chrono::duration<double> Since() {
    return std::chrono::high_resolution_clock::now() - last;
  }

  static void Start() { last = std::chrono::high_resolution_clock::now(); }

private:
  static std::chrono::time_point<std::chrono::high_resolution_clock> last;
};

class Renderable {
public:
  virtual ~Renderable(){};

  virtual glm::mat4 Render() = 0;
};

class Renderer {
public:
  Renderer(Window w, std::vector<std::vector<Renderable *>> d, glm::mat4 v,
           glm::mat4 p)
      : window(w), display(d), view(v), projection(p), program(([=] {
          auto vshader = std::ifstream("shaders/triangle.vert");
          auto fshader = std::ifstream("shaders/triangle.frag");
          return graphics::ProgramBuilder()
              .AddVertexShader(vshader)
              .AddFragmentShader(fshader)
              .Build();
        })()),
        mvpHandle(glGetUniformLocation(program, "model_view_projection")),
        buffer(([=] {
          ([] {
            GLuint vaID;
            glGenVertexArrays(1, &vaID);
            glBindVertexArray(vaID);
          })();

          GLuint buf;
          glGenBuffers(1, &buf);
          glBindBuffer(GL_ARRAY_BUFFER, buf);
          glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * display.size() * 12,
                       nullptr, GL_STATIC_DRAW);
          return buf;
        })()),
        vertices(([=] {
          auto p = static_cast<GLfloat *>(
              glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));
          if (p == nullptr) {
            throw std::runtime_error("glMapBuffer error: " +
                                     std::string(reinterpret_cast<const char *>(
                                         gluErrorString(glGetError()))));
          }
          return p;
        })()) {}

  std::vector<glm::mat4> CreateMatrices() {
    std::vector<glm::mat4> matrices;
    for (auto d : display) {
      glm::mat4 p(1.0);
      for (auto r : d) {
        p *= r->Render();
      }
      matrices.push_back(p);
    }
    return matrices;
  }

  void Render() {
    // Render items in display
    auto m = CreateMatrices();
    for (auto i = 0; i < m.size(); i++) {
      for (auto j = 0; j < 4; j++) {
        for (auto k = 0; k < 3; k++) {
          vertices[(i * 12) + (j * 4) + k] = m[i][j][k];
        }
      }
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glVertexAttribPointer(0, 4, GL_FLOAT, false, 0, static_cast<void *>(0));
    glUseProgram(program);
    auto mvp = projection * view;
    glUniformMatrix4fv(mvpHandle, 1, GL_FALSE, &mvp[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, 4 * display.size());
    glDisableVertexAttribArray(0);

    window.Swap();
  }

  Window Window() { return window; }

private:
  // Must initialize first!
  graphics::Window window;
  std::vector<std::vector<Renderable *>> display;
  glm::mat4 view, projection;
  GLuint program, mvpHandle, buffer;
  GLfloat *vertices;
};

} // namespace graphics

class Scale : public graphics::Renderable {
public:
  Scale(glm::vec3 s) : scale(s) {}
  virtual glm::mat4 Render() { return glm::scale(scale); }

private:
  glm::vec3 scale;
};

class Translate : public graphics::Renderable {
public:
  Translate(glm::vec3 t) : translation(t) {}
  virtual glm::mat4 Render() { return glm::translate(translation); }

private:
  glm::vec3 translation;
};

class EquilateralTriangle : public graphics::Renderable {
public:
  EquilateralTriangle(double r)
      : vertices(glm::vec4(glm::vec3(1, -1, 0), 1),
                 glm::vec4(glm::vec3(-1, -1, 0), 1),
                 glm::vec4(glm::vec3(0, 1, 0), 1), glm::vec4(1)) {}
  virtual glm::mat4 Render() { return vertices; }

private:
  glm::mat4 vertices;
};

class Float : public graphics::Renderable {
public:
  Float(double rad, std::chrono::duration<double> d)
      : radius(rad), duration(d) {}
  virtual glm::mat4 Render() {
    return glm::translate(glm::vec3(
        0.2,
        radius * glm::cos(timer.Since() / duration * 2 * glm::pi<double>()),
        0.2));
  }

private:
  double radius;
  std::chrono::duration<double> duration;
  graphics::Timer timer;
};

class Spin : public graphics::Renderable {
public:
  Spin(std::chrono::duration<double> d) : duration(d) {}
  virtual glm::mat4 Render() {
    return glm::rotate(
        static_cast<float>(timer.Since() / duration * 2 * glm::pi<double>()),
        glm::vec3(0, 1, 0));
  }

private:
  std::chrono::duration<double> duration;
  graphics::Timer timer;
};

std::chrono::time_point<std::chrono::high_resolution_clock>
    graphics::Timer::last;

int main() {
  graphics::Window w("basilisk", 400, 400);
  graphics::Renderer renderer(
      w, ([=] {
        auto rand = ([] {
          std::mt19937_64 random;
          std::uniform_real_distribution<double> dist(-1, 1);
          return std::bind(dist, random);
        })();
        auto triangle = [&] {
          return std::vector<graphics::Renderable *>(
              {new Translate({rand(), rand(), rand()}),
               new Scale({0.25, 0.25, 0.25}),
               new Float(0.25, std::chrono::seconds(1)),
               new Spin(std::chrono::seconds(6)),
               new EquilateralTriangle(0.2)});
        };
        std::vector<std::vector<graphics::Renderable *>> vec;
        for (auto i = 0; i < 20; i++) {
          vec.push_back(triangle());
        }
        return vec;
      })(),
      glm::lookAt(glm::vec3(4, 3, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)),
      glm::perspective(0.5 * glm::pi<double>(),
                       static_cast<double>(w.Width()) /
                           static_cast<double>(w.Height()),
                       0.1, 100.0));

  graphics::Timer::Start();
  for (;;) {
    renderer.Render();
    glfwPollEvents();
    if (renderer.Window().Key(GLFW_KEY_ESCAPE) == GLFW_PRESS ||
        renderer.Window().ShouldClose()) {
      break;
    }
  }
}
