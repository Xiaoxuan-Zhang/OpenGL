#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormals;
layout (location = 2) in vec2 aTexCoords;


uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out VS_OUT {
  vec3 Normal;
  vec3 Position;
} vs_out;

void main() {
  vs_out.Normal = mat3(transpose(inverse(model))) * aNormals;
  vs_out.Position = vec3(model * vec4(aPos, 1.0));
  gl_Position = projection * view * model * vec4(aPos,1.0);

}
