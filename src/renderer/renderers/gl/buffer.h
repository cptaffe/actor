// Copyright 2016 Connor Taffe

#ifndef SRC_RENDERER_RENDERERS_GL_BUFFER_H_
#define SRC_RENDERER_RENDERERS_GL_BUFFER_H_

#include <GL/glew.h>
#include <GL/glu.h>
#include <glm/glm.hpp>

#include <map>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace gl {

class VertexArray {
 public:
  VertexArray() {
    glGenVertexArrays(1, &handle);
    glBindVertexArray(handle);
  }
  GLuint Handle() const { return handle; }

 private:
  GLuint handle;
};

template <typename T>
class Buffer {
 public:
  explicit Buffer(GLenum t = GL_ARRAY_BUFFER) : type{t} {
    if (va == nullptr) {
      va = new VertexArray{};
    }
    glGenBuffers(1, &handle);
  }
  explicit Buffer(std::vector<T> values, GLenum t = GL_ARRAY_BUFFER)
      : Buffer(t) {
    Write(values);
  }
  Buffer(const Buffer &) = delete;
  ~Buffer() { glDeleteBuffers(1, &handle); }

  size_t Size() const { return size; }
  GLuint Handle() const { return handle; }

  void Write(std::vector<T> vec) {
    Bind();
    size = vec.size();
    glBufferData(type, sizeof(T) * size, vec.data(), GL_STATIC_DRAW);
  }
  void Bind() { glBindBuffer(type, handle); }

 private:
  static thread_local VertexArray *va;
  size_t size = {0};
  GLenum type;
  GLuint handle;
};

template <typename T>
thread_local VertexArray *Buffer<T>::va = nullptr;

}  // namespace gl

template <typename T>
std::ostream &operator<<(std::ostream &os, const gl::Buffer<T> &buf) {
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

#endif  // SRC_RENDERER_RENDERERS_GL_BUFFER_H_
