#version 330 core

layout(location = 0) in vec3 vPos; // vertex position

void main() {
  gl_Position.xyz = vPos;
  gl_Position.w = 1.0;
  
}
