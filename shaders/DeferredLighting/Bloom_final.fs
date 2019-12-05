#version 330 core
#define GAMMA 2.2
#define EXPOSURE 1.0

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_color;
uniform sampler2D texture_bright;
uniform sampler2D depthmap;
uniform float nearPlane;
uniform float farPlane;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC
    return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}

void main()
{
    vec3 color = texture(texture_color, TexCoords).rgb;
    vec3 bright = texture(texture_bright, TexCoords).rgb;
    float depth = texture(depthmap, TexCoords).r;
    depth = LinearizeDepth(depth);
    color += bright;
    
    // tone mapping
    vec3 rst = vec3(1.0) - exp(-color * EXPOSURE);
    
    // gamma correction
    rst = pow(rst, vec3(1.0 / GAMMA));
    FragColor = vec4(color, 1.0);
}
