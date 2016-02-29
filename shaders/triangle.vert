#version 330 core

layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 color;

out vec4 fragColor;

uniform mat4 model_view_projection[1024];

void main() {
  fragColor = color;
  gl_Position = model_view_projection[gl_InstanceID] * pos;
}
