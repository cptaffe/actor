
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
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
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), &vertices[0],
               GL_STATIC_DRAW);

  do {
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, static_cast<void *>(0));
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(0);

    glfwSwapBuffers(window);
    glfwPollEvents();

  } // Check if the ESC key was pressed or the window was closed
  while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
         !glfwWindowShouldClose(window));
}
