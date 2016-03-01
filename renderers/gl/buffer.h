
#ifndef B_RENDERERS_GL_BUFFER_H_
#define B_RENDERERS_GL_BUFFER_H_

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
  VertexArray() { glGenVertexArrays(1, &handle); }

  GLuint Handle() const { return handle; }
  void Bind() { glBindVertexArray(handle); }

private:
  GLuint handle;
};

extern VertexArray *vertex_array;

template <typename T> class Buffer {
public:
  explicit Buffer(GLenum t = GL_ARRAY_BUFFER) : type(t) {
    if (vertex_array == nullptr) {
      vertex_array = new VertexArray();
    }
    glGenBuffers(1, &handle);
  }

  size_t Size() const { return len; }
  GLuint Handle() const { return handle; }

  class Iterator {
  public:
    Iterator(Buffer *b, size_t i) : buffer(b), index(i) {}
    bool operator==(Iterator &other) {
      return buffer == other.buffer && index == other.index;
    }
    bool operator!=(Iterator &other) { return !operator==(other); }
    void operator++() {
      if (index + 1 < buffer->Size()) {
        index++;
      }
    }
    T operator*() { return buffer->buffer[index]; }
    void operator=(T item) { buffer->buffer[index] = item; }

  private:
    Buffer *buffer;
    size_t index;
  };

  Iterator Index(size_t i) {
    if (i > len) {
      std::stringstream s;
      s << i << " > " << len << ", out of bounds";
      throw std::out_of_range(s.str());
    }
    return Iterator(this, i);
  }

  Iterator operator[](size_t i) { return Index(i); }

  // fill buffer with vector of matrices
  // assumes a vec3 layout
  void Fill(std::vector<glm::mat4> buf) {
    auto w = 3, h = 3;
    Allocate(buf.size() * w * h);
    for (auto i = 0; i < buf.size(); i++) {
      // skip the last row, it contains nothing
      for (auto j = 0; j < h; j++) {
        for (auto k = 0; k < w; k++) {
          Index((i * w * h) + (j * h) + k) = buf[i][j][k];
        }
      }
    }
  }

  // fill with copies of the buffer
  void Fill(glm::vec4 buf, size_t copies) {
    Allocate(copies * 3);
    for (auto i = 0; i < copies; i++) {
      for (auto j = 0; j < 3; j++) {
        Index((i * 3) + j) = buf[j];
      }
    }
  }

  void Push(T item) {
    auto i = len;
    Allocate(len + 1);
    Index(i) = item;
  }

  void Clear() { ForceAllocate(0); }

  Iterator Begin() { return Iterator(this, 0); }
  Iterator End() { return Iterator(this, len - 1); }

  void Bind() { glBindBuffer(type, handle); }

  // STL mapping
  Iterator begin() { return Begin(); }
  Iterator end() { return End(); }

private:
  size_t len = 0;
  GLenum type;
  GLuint handle;
  T *buffer;

  void ForceAllocate(size_t l) {
    T *v = new T[len];
    memcpy(v, buffer, len * sizeof(T));
    len = l;
    vertex_array->Bind();
    Bind();
    glBufferData(type, sizeof(T) * len, nullptr, GL_STATIC_DRAW);
    if (l == 0) {
      buffer = nullptr;
    } else {
      buffer = static_cast<T *>(glMapBuffer(type, GL_READ_WRITE));
      if (buffer == nullptr) {
        throw std::runtime_error("glMapBuffer error: " +
                                 std::string(reinterpret_cast<const char *>(
                                     gluErrorString(glGetError()))));
      }
      memcpy(buffer, v, len * sizeof(T));
    }
    delete[] v;
  }

  void Allocate(size_t l) {
    if (len < l) {
      ForceAllocate(l);
    }
  }
};
} // namespace gl

template <typename T>
std::ostream &operator<<(std::ostream &os, gl::Buffer<T> &buf) {
  os << "{";
  for (auto i = 0; i < buf.Size(); i++) {
    os << *buf[i];

    if (i + 1 != buf.Size()) {
      os << ", ";
    }
  }
  os << "}";
  return os;
}

#endif // B_RENDERERS_GL_BUFFER_H_
