#version 450 core

layout (location = 0) in vec2 pos_in;

layout (std140, binding = 0) uniform CameraInfo {
  mat4 view;
  mat4 proj;
};

void main() {
  gl_Position = proj * view * vec4(pos_in, 0.0f, 1.0f);
}
