#version 330 core
#define GAMMA 2.2
#define EXPOSURE 1.0

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_color;
uniform sampler2D texture_bright;

void main()
{
    vec3 color = texture(texture_color, TexCoords).rgb;
    vec3 bright = texture(texture_bright, TexCoords).rgb;
    color += bright;
    
    // tone mapping
    vec3 rst = vec3(1.0) - exp(-color * EXPOSURE);
    
    // gamma correction
    rst = pow(rst, vec3(1.0 / GAMMA));
    FragColor = vec4(rst, 1.0);
}
