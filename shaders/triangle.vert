#version 330 core

layout(location = 0) in vec4 vPos; // vertex position

uniform mat4 model_view_projection;

void main() {
  gl_Position = model_view_projection * vec4(vPos.xyz, 1);
}
