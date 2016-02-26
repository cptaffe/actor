
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>
#include <streambuf>
#include <vector>

namespace graphics {

class ProgramBuilder {
public:
  ProgramBuilder() : programHandle(glCreateProgram()) {}

  ProgramBuilder AddVertexShader(std::string src) {
    shaders.insert({glCreateShader(GL_VERTEX_SHADER), src});
    return *this;
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
    auto check = [=](void (*f)(GLuint, GLenum, GLint *), GLuint handle,
                     std::string err) {
      GLint res;
      f(handle, GL_COMPILE_STATUS, &res);
      int ll;
      f(handle, GL_INFO_LOG_LENGTH, &ll);
      std::cout << "log length: " << ll << std::endl;
      if (!res) {
        auto v = std::vector<char>(ll + 1);
        glGetShaderInfoLog(handle, ll, nullptr, v.data());
        throw std::runtime_error(err + ": " + std::string(v.begin(), v.end()));
      }
    };

    for (auto s : shaders) {
      auto sc = s.second.c_str();
      glShaderSource(s.first, 1, &sc, nullptr);
      glCompileShader(s.first);
      check(glGetShaderiv, s.first, "error compiling shader");
      glAttachShader(programHandle, s.first);
    }

    glLinkProgram(programHandle);
    check(glGetProgramiv, programHandle, "error linking program");
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
  if (!glfwInit()) {
    throw std::runtime_error("glfw initialization failed");
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  auto window = glfwCreateWindow(1024, 768, "Tutorial 01", nullptr, nullptr);
  if (window == nullptr) {
    glfwTerminate();
    throw std::runtime_error(
        "failed to open glfw window. If you have an Intel GPU, they "
        "are not 3.3 compatible. Try the 2.1 version of the tutorials.");
  }
  glfwMakeContextCurrent(window);
  glewExperimental = true;
  if (glewInit() != GLEW_OK) {
    throw std::runtime_error("failed to initialize glew");
  }

  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

  GLuint vaID;
  glGenVertexArrays(1, &vaID);
  glBindVertexArray(vaID);

  std::vector<GLfloat> vertices = {-1, -1, 0, 1, -1, 0, 0, 1, 0};

  GLuint buf;
  glGenBuffers(1, &buf);
  glBindBuffer(GL_ARRAY_BUFFER, buf);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(),
               vertices.data(), GL_STATIC_DRAW);

  auto readFile = [=](std::string s) {
    auto f = std::ifstream(s);
    return std::string(std::istreambuf_iterator<char>(f),
                       std::istreambuf_iterator<char>());
  };

  auto programHandle = graphics::ProgramBuilder()
                           .AddVertexShader(readFile("shaders/triangle.vert"))
                           .AddFragmentShader(readFile("shaders/triangle.frag"))
                           .Build();

  do {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, static_cast<void *>(0));
    glUseProgram(programHandle);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(0);

    glfwSwapBuffers(window);
    glfwPollEvents();

  } // Check if the ESC key was pressed or the window was closed
  while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
         !glfwWindowShouldClose(window));
}
