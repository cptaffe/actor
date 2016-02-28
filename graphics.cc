
#include <GL/glew.h>
#include <GLFW/glfw3.h>

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

int main() {
  graphics::Window window;

  GLuint vaID;
  glGenVertexArrays(1, &vaID);
  glBindVertexArray(vaID);

  std::atomic_flag verticesSynced;
  std::mutex vmutex;
  std::vector<GLfloat> vertices = {-1, -1, 0, 1, -1, 0, 0, 1, 0};

  GLuint buf;
  glGenBuffers(1, &buf);
  glBindBuffer(GL_ARRAY_BUFFER, buf);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(),
               vertices.data(), GL_STATIC_DRAW);

  auto programHandle = ([=] {
    auto vshader = std::ifstream("shaders/triangle.vert");
    auto fshader = std::ifstream("shaders/triangle.frag");
    return graphics::ProgramBuilder()
        .AddVertexShader(vshader)
        .AddFragmentShader(fshader)
        .Build();
  })();

  // transform the coordinates of the triangle over time.
  auto t = std::thread([&] {
    std::mt19937_64 random;
    std::uniform_real_distribution<double> dist(0.9, 1.1);
    auto rand = std::bind(dist, random);
    for (;;) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      std::unique_lock<std::mutex> lock(vmutex);
      verticesSynced.clear();
      for (auto &v : vertices) {
        v = v * rand();
      }
    }
  });

  for (;;) {
    if (verticesSynced.test_and_set()) {
      std::unique_lock<std::mutex> lock(vmutex);
      glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(),
                   vertices.data(), GL_STATIC_DRAW);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, static_cast<void *>(0));
    glUseProgram(programHandle);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(0);

    window.Swap();
    glfwPollEvents();

    if (window.Key(GLFW_KEY_ESCAPE) == GLFW_PRESS || window.ShouldClose()) {
      break;
    }
  }
}
