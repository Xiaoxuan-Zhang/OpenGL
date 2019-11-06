#version 330 core

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT {
  vec3 normal;
} gs_in[];

uniform float time;
const float MAGNITUDE = 0.1;

vec4 explode(vec4 position, vec3 normal){
  float magnitude = 2.0;
  vec3 direction = normal * abs(sin(0.2 * time)) * magnitude;
  return position + vec4(direction, 0.0);
}

void generateLine(int index) {
  gl_Position = gl_in[index].gl_Position;    // 1:bottom-left
  gl_Position = explode(gl_Position, gs_in[index].normal);
  EmitVertex();
  vec4 direction = vec4(gs_in[index].normal, 0.0) * MAGNITUDE;
  gl_Position = gl_in[index].gl_Position + direction;
  gl_Position = explode(gl_Position, gs_in[index].normal);
  EmitVertex();
  EndPrimitive();
}

void main() {
    generateLine(0);
    generateLine(1);
    generateLine(2);

}
