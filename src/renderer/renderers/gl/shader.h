// Copyright 2016 Connor Taffe

#ifndef SRC_RENDERER_RENDERERS_GL_SHADER_H_
#define SRC_RENDERER_RENDERERS_GL_SHADER_H_

#include <GL/glew.h>

#include <iterator>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace gl {

class Program {
 public:
  explicit Program(GLuint p) : program(p) {}
  Program(const Program &) = delete;
  ~Program() { glDeleteProgram(program); }
  void Use() { glUseProgram(program); }
  GLint UniformLocation(std::string s) {
    auto u = glGetUniformLocation(program, s.c_str());
    if (u == -1) {
      throw std::runtime_error(static_cast<std::stringstream &>(
                                   std::stringstream()
                                   << "gl::Program.UnformLocation: Uniform '"
                                   << s << "' does not exist in this program")
                                   .str());
    }
    return u;
  }

 private:
  GLuint program;
};

class ProgramBuilder {
 public:
  ProgramBuilder();

  ProgramBuilder AddVertexShader(std::istream *src);
  ProgramBuilder AddVertexShader(std::string src);
  ProgramBuilder AddFragmentShader(std::istream *src);
  ProgramBuilder AddFragmentShader(std::string src);
  std::shared_ptr<Program> Build();

 private:
  GLuint programHandle;
  std::map<GLuint, std::string> shaders;
};

}  // namespace gl

#endif  // SRC_RENDERER_RENDERERS_GL_SHADER_H_
