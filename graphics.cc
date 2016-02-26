
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <streambuf>
#include <vector>

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

  auto programID = ([=](std::string vs, std::string fs) {
    auto vsID = glCreateShader(GL_VERTEX_SHADER);
    auto fsID = glCreateShader(GL_FRAGMENT_SHADER);

    auto readFile = [=](std::string s) {
      auto f = std::ifstream(s);
      return std::string(std::istreambuf_iterator<char>(f),
                         std::istreambuf_iterator<char>());
    };

    auto programID = glCreateProgram();
    for (auto s : ([=](std::vector<std::pair<GLuint, std::string>> v) {
           std::vector<std::pair<GLuint, std::string>> ov;
           for (auto a : v) {
             ov.push_back({a.first, readFile(a.second)});
           }
           return ov;
         })({{vsID, vs}, {fsID, fs}})) {
      auto sc = s.second.c_str();
      glShaderSource(s.first, 1, &sc, nullptr);
      glCompileShader(s.first);
      GLint res;
      glGetShaderiv(s.first, GL_COMPILE_STATUS, &res);
      int ll;
      glGetShaderiv(s.first, GL_INFO_LOG_LENGTH, &ll);
      if (ll > 0) {
        auto v = std::vector<char>(ll + 1);
        glGetShaderInfoLog(s.first, ll, nullptr, v.data());
        throw std::runtime_error("error compiling shader: " +
                                 std::string(v.begin(), v.end()));
      }
      glAttachShader(programID, s.first);
    }
    glLinkProgram(programID);

    GLint res;
    glGetProgramiv(programID, GL_COMPILE_STATUS, &res);
    int ll;
    glGetShaderiv(programID, GL_INFO_LOG_LENGTH, &ll);
    if (ll > 0) {
      auto v = std::vector<char>(ll + 1);
      glGetProgramInfoLog(programID, ll, nullptr, v.data());
      throw std::runtime_error("error linking program: " +
                               std::string(v.begin(), v.end()));
    }

    for (auto s : std::vector<GLuint>({vsID, fsID})) {
      glDetachShader(programID, s);
      glDeleteShader(s);
    }

    return programID;

  })("shaders/triangle.vert", "shaders/triangle.frag");

  do {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, static_cast<void *>(0));
    glUseProgram(programID);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(0);

    glfwSwapBuffers(window);
    glfwPollEvents();

  } // Check if the ESC key was pressed or the window was closed
  while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
         !glfwWindowShouldClose(window));
}
