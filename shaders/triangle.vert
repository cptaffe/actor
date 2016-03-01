#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;

out vec3 fragColor;

uniform mat4 model_view_projection[1024];

void main() {
  fragColor = color;
  gl_Position = model_view_projection[gl_InstanceID] * vec4(pos, 1);
}
