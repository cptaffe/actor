
#ifndef B_RENDERERS_GL_WINDOW_H_
#define B_RENDERERS_GL_WINDOW_H_

#include <stdexcept>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace gl {

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

  void Swapiness(int i) { glfwSwapInterval(i); }
  void Swap() { glfwSwapBuffers(window); }

  int Width() { return width; }
  int Height() { return height; }

private:
  GLFWwindow *window;
  int width, height;
};

} // namespace gl

#endif // B_RENDERERS_GL_WINDOW_H_
