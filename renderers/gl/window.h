
#ifndef B_RENDERERS_GL_WINDOW_H_
#define B_RENDERERS_GL_WINDOW_H_

#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace gl {

class Window {
public:
  Window(std::string title, int w, int h);
  Window(GLFWwindow *const w);
  Window(const Window &other) = delete;
  ~Window();

  class Binding {
  public:
    Binding(GLFWwindow *n) : owin{glfwGetCurrentContext()} {
      glfwMakeContextCurrent(n);
    }
    Binding(const Binding &other) = delete;
    ~Binding() { glfwMakeContextCurrent(owin); }

  private:
    GLFWwindow *owin;
  };

  std::unique_ptr<Binding> Bind() const {
    return std::unique_ptr<Binding>(new Binding(window));
  }
  bool ShouldClose() {
    auto b = Bind();
    return glfwWindowShouldClose(window);
  }
  int Key(int key) {
    auto b = Bind();
    return glfwGetKey(window, key);
  }
  void Swapiness(int i) {
    auto b = Bind();
    glfwSwapInterval(i);
  }
  void Swap() {
    auto b = Bind();
    glfwSwapBuffers(window);
  }
  int Width() const;
  int Height() const;

private:
  GLFWwindow *const window;
};

} // namespace gl

#endif // B_RENDERERS_GL_WINDOW_H_
