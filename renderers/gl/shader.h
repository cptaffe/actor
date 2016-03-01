
#ifndef B_RENDERERS_GL_SHADER_H_
#define B_RENDERERS_GL_SHADER_H_

#include <iterator>
#include <map>
#include <string>
#include <vector>

#include <GL/glew.h>

namespace gl {

class Program {
public:
  Program(GLuint p) : program(p) {}
  void Use() { glUseProgram(program); }
  GLuint UniformLocation(std::string s) {
    return glGetUniformLocation(program, s.c_str());
  }

private:
  GLuint program;
};

class ProgramBuilder {
public:
  ProgramBuilder() : programHandle(glCreateProgram()) {
    if (!([=] {
          GLboolean support;
          glGetBooleanv(GL_SHADER_COMPILER, &support);
          return support;
        })()) {
      throw std::runtime_error("no shader support");
    }
  }

  ProgramBuilder AddVertexShader(std::istream &src) {
    return AddVertexShader(std::string(std::istreambuf_iterator<char>(src),
                                       std::istreambuf_iterator<char>()));
  }

  ProgramBuilder AddVertexShader(std::string src) {
    shaders.insert({glCreateShader(GL_VERTEX_SHADER), src});
    return *this;
  }

  ProgramBuilder AddFragmentShader(std::istream &src) {
    return AddFragmentShader(std::string(std::istreambuf_iterator<char>(src),
                                         std::istreambuf_iterator<char>()));
  }

  ProgramBuilder AddFragmentShader(std::string src) {
    shaders.insert({glCreateShader(GL_FRAGMENT_SHADER), src});
    return *this;
  }

  Program *Build() {
    CompileShaders();
    DeleteShaders();
    return new Program(programHandle);
  }

private:
  GLuint programHandle;
  std::map<GLuint, std::string> shaders;

  void CompileShaders() {
    // Error checking wrapper function
    auto check = [=](void (*f)(GLuint, GLenum, GLint *),
                     void (*l)(GLenum, GLsizei, GLsizei *, GLchar *),
                     GLenum status, GLuint handle, std::string err) {
      GLint res;
      f(handle, status, &res);
      int ll;
      f(handle, GL_INFO_LOG_LENGTH, &ll);
      if (res == GL_FALSE) {
        std::vector<char> v(ll + 1);
        l(handle, ll + 1, &ll, v.data());
        throw std::runtime_error(err + ": " +
                                 std::string(reinterpret_cast<const char *>(
                                     gluErrorString(glGetError()))) +
                                 ", " + std::string(v.begin(), v.end()));
      }
    };

    for (auto s : shaders) {
      auto sc = s.second.c_str();
      glShaderSource(s.first, 1, &sc, nullptr);
      glCompileShader(s.first);
      check(glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS, s.first,
            "error compiling shader");
      glAttachShader(programHandle, s.first);
    }

    glLinkProgram(programHandle);
    check(glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS, programHandle,
          "error linking program");
  }

  void DeleteShaders() {
    for (auto s : shaders) {
      glDetachShader(programHandle, s.first);
      glDeleteShader(s.first);
    }
  }
};

} // namespace gl

#endif // B_RENDERERS_GL_SHADER_H_
