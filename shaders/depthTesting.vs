#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormals;

uniform mat4 model;
layout (std140) uniform Matrices {
  mat4 projection;
  mat4 view;
};


out vec2 TexCoords;

void main() {
  gl_Position = projection * view * model * vec4(aPos,1.0);
  TexCoords = aTexCoords;
}
