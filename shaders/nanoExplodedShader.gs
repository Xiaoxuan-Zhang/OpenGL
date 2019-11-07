#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
  vec3 Normal;
  vec2 TexCoords;
  vec3 Position;
} gs_in[];

uniform float time;
out vec2 texCoords;

vec3 GetNormal() {
   vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
   vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
   return normalize(cross(a, b));
}

vec4 explode(vec4 position, vec3 normal){
  float magnitude = 2.0;
  vec3 direction = normal * abs(sin(0.2 * time)) * magnitude;
  return position + vec4(direction, 0.0);
}

void main() {
    //build_house(gl_in[0].gl_Position);
    vec3 normal = GetNormal(); //gs_in[0].Normal
    gl_Position = explode(gl_in[0].gl_Position, normal);
    texCoords = gs_in[0].TexCoords;
    EmitVertex();
    gl_Position = explode(gl_in[1].gl_Position, normal);
    texCoords = gs_in[1].TexCoords;
    EmitVertex();
    gl_Position = explode(gl_in[2].gl_Position, normal);
    texCoords = gs_in[2].TexCoords;
    EmitVertex();
    EndPrimitive();
}