
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <atomic>
#include <chrono>
#include <cmath>
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
  Window() {
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

    window = glfwCreateWindow(1024, 768, "Tutorial 01", nullptr, nullptr);
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

private:
  GLFWwindow *window;
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
      std::cout << "compiling shader: " << std::endl
                << "```" << std::endl
                << s.second << "```" << std::endl;
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

} // namespace graphics

class Timer {
public:
  Timer() {}
  std::chrono::duration<double> Delta() {
    return std::chrono::high_resolution_clock::now() - last;
  }
  static void Update() { last = std::chrono::high_resolution_clock::now(); }

private:
  static std::chrono::time_point<std::chrono::high_resolution_clock> last;
};

class Renderable {
public:
  virtual ~Renderable(){};

  class Point {
  public:
    double x, y, z;
  };

  virtual std::vector<Point> Render() = 0;
};

class EquilateralTriangle : public Renderable {
public:
  EquilateralTriangle(Point o, double r)
      : vertices({{o.x - (r * 1), o.y - (r * 1), o.z},
                  {o.x + (r * 1), o.y - (r * 1), o.z},
                  {o.x, o.y + (r * 1), o.z}}) {}
  virtual std::vector<Point> Render() { return vertices; }

private:
  std::vector<Point> vertices;
};

class Float : public Renderable {
public:
  Float(Renderable *r, double rad, std::chrono::duration<double> d)
      : renderable(r), radius(rad), duration(d) {}
  virtual std::vector<Point> Render() {
    auto v = renderable->Render();
    for (auto &p : v) {
      p.y += radius * cos(timer.Delta() / duration * 2);
    }
    return v;
  }

private:
  Renderable *renderable;
  double radius;
  std::chrono::duration<double> duration;
  Timer timer;
};

class Spin : public Renderable {
public:
  Spin(Renderable *r, std::chrono::duration<double> d)
      : renderable(r), duration(d) {}
  virtual std::vector<Point> Render() {
    auto v = renderable->Render();
    Point op;
    for (auto &p : v) {
      op.x += p.x;
      op.y += p.y;
      op.z += p.z;
    }
    op.x /= v.size();
    op.y /= v.size();
    op.z /= v.size();
    for (auto &p : v) {
      p.x = ((p.x - op.x) * cos(timer.Delta() / duration * 2)) + op.x;
      p.z = ((p.z - op.z) * sin(timer.Delta() / duration * 2)) + op.z;
    }
    return v;
  }

private:
  Renderable *renderable;
  std::chrono::duration<double> duration;
  Timer timer;
};

std::chrono::time_point<std::chrono::high_resolution_clock> Timer::last;

int main() {
  graphics::Window window;

  auto buf = ([=](std::vector<GLfloat> v) {
    ([] {
      GLuint vaID;
      glGenVertexArrays(1, &vaID);
      glBindVertexArray(vaID);
    })();

    GLuint buf;
    glGenBuffers(1, &buf);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * v.size(), v.data(),
                 GL_STATIC_DRAW);
    return buf;
  })({-1, -1, 0, 1, -1, 0, 0, 1, 0});

  auto vertices =
      static_cast<GLfloat *>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));

  auto programHandle = ([=] {
    auto vshader = std::ifstream("shaders/triangle.vert");
    auto fshader = std::ifstream("shaders/triangle.frag");
    return graphics::ProgramBuilder()
        .AddVertexShader(vshader)
        .AddFragmentShader(fshader)
        .Build();
  })();

  auto display = ([=] {
    std::vector<Renderable *> vec;
    auto rand = ([] {
      std::mt19937_64 random;
      std::uniform_real_distribution<double> dist(0.25, 0.75);
      return std::bind(dist, random);
    })();
    vec.push_back(new Float(
        new Spin(new EquilateralTriangle({rand(), rand(), rand()}, 0.2),
                 std::chrono::seconds(4)),
        0.25, std::chrono::seconds(10)));
    return vec;
  })();
  auto render = [&] {
    // Render items in display
    for (auto r : display) {
      auto v = r->Render();
      for (auto i = 0; i < 3; i++) {
        vertices[(i * 3) + 0] = v[i].x;
        vertices[(i * 3) + 1] = v[i].y;
        vertices[(i * 3) + 2] = v[i].z;
      }
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, static_cast<void *>(0));
    glUseProgram(programHandle);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(0);

    window.Swap();
  };

  for (;;) {
    render();

    glfwPollEvents();

    if (window.Key(GLFW_KEY_ESCAPE) == GLFW_PRESS || window.ShouldClose()) {
      break;
    }
  }
}
