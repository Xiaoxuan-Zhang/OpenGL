#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

out VS_OUT {
    vec3 Normal;
    vec2 TexCoords;
    vec3 Position;
    vec4 FragPosLightSpace;
} vs_out;


void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    vs_out.Normal = mat3(transpose(inverse(model))) * aNormal;
    vs_out.Position = worldPos.xyz;
    vs_out.TexCoords = aTexCoords;
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(worldPos.xyz, 1.0);
    gl_Position = projection * view * worldPos;
}
