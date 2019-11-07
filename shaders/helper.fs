#version 330 core

in vec2 TexCoords;
uniform sampler2D texture_attached;

out vec4 FragColor;

void main() {
  //FragColor = vec4(1.0, 0.0, 0.0, 1.0);
  FragColor = texture(texture_attached, TexCoords);
}