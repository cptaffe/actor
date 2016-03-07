// Copyright 2016 Connor Taffe

#include "src/renderer/renderers/gl/window.h"

#include <mutex>
#include <stdexcept>
#include <string>

namespace {
static std::mutex glfwInitLock;
static thread_local bool glfw3Inited = false;
}

namespace gl {
Window::Window(std::string title, int w, int h)
    : window{([&] {
        // HACK HACK HACK
        // glfwInit() should only be called from the main thread
        // but here we are just locking and hoping that it doesn't
        // blow up.
        // Works so far, but jank.
        if (!glfw3Inited) {
          std::unique_lock<std::mutex> lock(glfwInitLock);
          if (!glfwInit()) {
            throw std::runtime_error("glfw initialization failed");
          }
          glfw3Inited = true;
        }

        glfwSetErrorCallback([](int err, const char *msg) {
          // TODO(cptaffe): thread-based context-has-been-set boolean
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
                                 [](GLFWwindow *win, int width, int height) {
                                   auto o = glfwGetCurrentContext();
                                   glfwMakeContextCurrent(win);
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

}  // namespace gl
