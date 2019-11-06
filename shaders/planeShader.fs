#version 330 core

uniform sampler2D texture1;

in vec2 TexCoords;

out vec4 FragColor;


void main()
{
    vec4 texColor = texture(texture1, TexCoords);
    FragColor = texColor;
}
