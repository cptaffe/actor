
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
  ProgramBuilder();

  ProgramBuilder AddVertexShader(std::istream &src);
  ProgramBuilder AddVertexShader(std::string src);
  ProgramBuilder AddFragmentShader(std::istream &src);
  ProgramBuilder AddFragmentShader(std::string src);
  Program *Build();

private:
  GLuint programHandle;
  std::map<GLuint, std::string> shaders;
};

} // namespace gl

#endif // B_RENDERERS_GL_SHADER_H_
