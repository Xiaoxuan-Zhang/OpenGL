#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform vec3 lightPosition;

layout (std140) uniform Matrices {
  mat4 projection;
  mat4 view;
};

out vec3 Normal;
out vec3 FragPosition;
out vec3 LightPosition;
out vec2 TexCoords;

void main() {
  gl_Position = projection * view * model * vec4(aPos,1.0);
  Normal = mat3(transpose(inverse(view * model))) * aNormal; //Transform to view space
  FragPosition = vec3(view * model * vec4(aPos, 1.0)); //Transform to view space
  LightPosition = vec3(view * model * vec4(lightPosition, 1.0)); //Transform to view space
  TexCoords = aTexCoords;
}
