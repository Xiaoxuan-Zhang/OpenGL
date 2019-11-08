#version 330 core

uniform vec3 cubeColor;
out vec4 color;

void main() {
  color = vec4(cubeColor, 1.0);
}
