
#include <stdexcept>
#include <string>

#include "renderers/gl/window.h"

namespace gl {
Window::Window(std::string title, int w, int h)
    : window{([&] {
        if (!glfwInit()) {
          throw std::runtime_error("glfw initialization failed");
        }

        glfwSetErrorCallback([](int err, const char *msg) {
          // TODO: thread-based context-has-been-set boolean
          if (err == GLFW_NO_CURRENT_CONTEXT) {
            return;
          }
          throw std::runtime_error("glfw3 error: " + std::string(msg));
        });

        glfwWindowHint(GLFW_SAMPLES, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Return creation function
        return &glfwCreateWindow;
      })()(w, h, title.c_str(), nullptr, nullptr)} {
  if (window == nullptr) {
    throw std::runtime_error("glfw create window failed");
  }

  glfwSetFramebufferSizeCallback(window,
                                 [](GLFWwindow *window, int width, int height) {
                                   auto o = glfwGetCurrentContext();
                                   glfwMakeContextCurrent(window);
                                   glViewport(0, 0, width, height);
                                   glfwMakeContextCurrent(o);
                                 });

  // Initialize window for the first time
  auto b = Bind();
  glewExperimental = true;
  if (glewInit() != GLEW_OK) {
    throw std::runtime_error("failed to initialize glew");
  }
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
}
Window::Window(GLFWwindow *const w) : window{w} {}

Window::~Window() { glfwDestroyWindow(window); }

int Window::Width() const {
  auto b = Bind();
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  return width;
}

int Window::Height() const {
  auto b = Bind();
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  return height;
}

} // namespace gl
