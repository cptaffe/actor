
#ifndef B_RENDERERS_GL_BUFFER_H_
#define B_RENDERERS_GL_BUFFER_H_

#include <map>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <GL/glew.h>
#include <GL/glu.h>
#include <glm/glm.hpp>

namespace gl {

class VertexArray {
public:
  VertexArray() { glGenVertexArrays(1, &handle); }

  GLuint Handle() const { return handle; }
  void Bind() { glBindVertexArray(handle); }
  void Unbind() { glBindVertexArray(0); }

private:
  GLuint handle;
};

extern VertexArray *vertex_array;

template <typename T> class Buffer {
public:
  explicit Buffer(GLenum t = GL_ARRAY_BUFFER) : type(t) {
    if (vertex_array == nullptr) {
      vertex_array = new VertexArray();
      vertex_array->Bind();
    }
    glGenBuffers(1, &handle);
  }

  size_t Size() const { return vec.size(); }
  GLuint Handle() const { return handle; }

  typename std::vector<T>::reference Index(size_t i) { return vec[i]; }

  typename std::vector<T>::reference operator[](size_t i) { return Index(i); }

  // Fills write to memory as they go.

  void Fill(std::vector<glm::mat4> buf) {
    auto w = 3, h = 3;
    for (auto i = 0; i < buf.size(); i++) {
      // skip the last row, it contains nothing
      for (auto j = 0; j < h; j++) {
        for (auto k = 0; k < w; k++) {
          vec.push_back(buf[i][j][k]);
        }
      }
    }
  }

  // fill with copies of the buffer
  void Fill(glm::vec4 buf, size_t copies) {
    for (auto i = 0; i < copies; i++) {
      for (auto j = 0; j < 3; j++) {
        vec.push_back(buf[j]);
      }
    }
  }

  void Fill(std::vector<glm::vec3> buf) {
    auto w = 3;
    for (auto i = 0; i < buf.size(); i++) {
      for (auto j = 0; j < w; j++) {
        vec.push_back(buf[i][j]);
      }
    }
  }

  void Push(T item) { vec.push_back(item); }

  void Clear() { vec.clear(); }

  typename std::vector<T>::iterator Begin() { return vec.begin(); }
  typename std::vector<T>::iterator End() { return vec.end(); }

  void Write() {
    Bind();
    size = vec.size();
    glBufferData(type, sizeof(T) * size, vec.data(), GL_STATIC_DRAW);
  }
  void Bind() { glBindBuffer(type, handle); }

  // STL mapping
  typename std::vector<T>::iterator begin() { return Begin(); }
  typename std::vector<T>::iterator end() { return End(); }

private:
  size_t size = 0;
  GLenum type;
  GLuint handle;
  std::vector<T> vec;
};
} // namespace gl

template <typename T>
std::ostream &operator<<(std::ostream &os, gl::Buffer<T> &buf) {
  os << "{";
  for (auto i = 0; i < buf.Size(); i++) {
    os << buf[i];

    if (i + 1 != buf.Size()) {
      os << ", ";
    }
  }
  os << "}";
  return os;
}

#endif // B_RENDERERS_GL_BUFFER_H_
