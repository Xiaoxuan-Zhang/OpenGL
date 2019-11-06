#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

layout (std140) uniform Matrices {
  mat4 projection;
  mat4 view;
};
uniform mat4 model;
out VS_OUT
{
  vec3 FragPos;
  vec3 Normal;
  vec2 TexCoords;
  mat3 TBN;
  vec3 Tangent;
  vec3 Bitangent;
} vs_out;

void main()
{
  vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
  vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;
  vs_out.TexCoords = aTexCoords;
  gl_Position = projection * view * model * vec4(aPos, 1.0);
  vec3 N = normalize(vec3(model * vec4(aNormal, 0.0)));
  vec3 T = normalize(vec3(model * vec4(aTangent, 0.0)));
  vec3 B = normalize(vec3(model * vec4(aBitangent, 0.0)));
  T = normalize(T - dot(T, N) * N);
  if (dot(cross(N, T), B) < 0.0)
    T = T * -1.0;
  vs_out.TBN = mat3(T, B, N);
}
