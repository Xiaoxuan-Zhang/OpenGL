#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;

out VS_OUT {
    vec3 normal;
    vec3 position;
    vec2 texCoords;
} vs_out;

void main() {
    vec3 worldPos = vec3(model * vec4(aPos, 1.0));
    gl_Position = projection * view * vec4(worldPos, 1.0);
    vs_out.normal = mat3(model) * aNormal;
    vs_out.position = worldPos;
    vs_out.texCoords = aTexCoords;
}
