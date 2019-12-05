#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

uniform vec3 lightColor;

void main() {
    FragColor = vec4(lightColor, 1.0);
    //brightness
    float brightness = dot(lightColor, vec3(0.1126, 0.5152, 0.0722));
    if (brightness > 1.0) {
        BrightColor = vec4(lightColor, 1.0);
    } else {
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
