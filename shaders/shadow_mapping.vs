#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;


layout (std140) uniform Matrices {
  mat4 projection;
  mat4 view;
};
uniform mat4 model;
uniform mat4 lightSpaceMatrix;

out VS_OUT {
  vec2 TexCoords;
  vec3 Normal;
  vec3 FragPosition;
  vec4 FragPosLightSpace;
} vs_out;


void main() {
  gl_Position = projection * view * model * vec4(aPos,1.0);
  vs_out.Normal = mat3(transpose(inverse(model))) * aNormal; //calculate in world space
  vs_out.FragPosition = vec3(model * vec4(aPos,1.0)); //calculate in world space
  vs_out.TexCoords = aTexCoords;
  vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPosition, 1.0);
}
