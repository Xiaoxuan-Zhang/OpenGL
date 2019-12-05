#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

uniform sampler2D texure_gPosition;
uniform sampler2D texure_gNormal;
uniform sampler2D texture_gAlbedoSpec;
uniform vec3 cameraPos;

struct Light {
    vec3 lightPos;
    vec3 lightColor;
    //float linear;
    //float quadratic;
};
const int LIGHT_NUM = 32;
uniform Light lights[LIGHT_NUM];

in vec2 TexCoords;

void main() {
    vec3 normal = texture(texure_gNormal, TexCoords).rgb;
    vec3 fragPos = texture(texure_gPosition, TexCoords).rgb;
    vec3 albedo = texture(texture_gAlbedoSpec, TexCoords).rgb;
    float specular = texture(texture_gAlbedoSpec, TexCoords).a;
    
    vec3 viewDir = normalize(cameraPos - fragPos);
    vec3 color = albedo * 0.3;
    float linear = 0.5;
    float quadratic = 1.5;
    for (int i = 0; i < LIGHT_NUM; ++i) {
        vec3 lightDir = normalize(lights[i].lightPos);
        vec3 lightCol = lights[i].lightColor;
        vec3 halfWayDir = normalize(lightDir + viewDir);

        vec3 diffCol = max(dot(normal, lightDir), 0.0) * albedo * lightCol;
        vec3 specCol = pow(max(dot(normal, halfWayDir), 0.0), 16.0) * lightCol * specular;

        float dist = length(lights[i].lightPos - fragPos);
        float att = 1.0 / (1.0 + linear * dist + quadratic * dist * dist);
        color += (diffCol + specCol) * att;
    }
    // color
    FragColor = vec4(color, 1.0);
    BrightColor = vec4(1.0);
    //brightness
    vec3 threshold = vec3(0.9126, 0.7152, 0.8722);
    float brightness = dot(color, threshold);
    if (brightness > 1.0) {
        BrightColor = FragColor;
    } else {
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
