#version 330 core

uniform sampler2D texure_gPosition;
uniform sampler2D texure_gNormal;
uniform sampler2D texture_gAlbedoSpec;
uniform vec3 cameraPos;

struct Light {
    vec3 lightPos;
    vec3 lightColor;
    float linear;
    float quadratic;
};
const int LIGHT_NUM = 32;
uniform Light lights[LIGHT_NUM];

in vec2 TexCoords;
out vec4 FragColor;
void main() {
    vec3 normal = texture(texure_gNormal, TexCoords).rgb;
    vec3 fragPos = texture(texure_gPosition, TexCoords).rgb;
    vec3 albedo = texture(texture_gAlbedoSpec, TexCoords).rgb;
    float specular = texture(texture_gAlbedoSpec, TexCoords).a;
    
    vec3 viewDir = normalize(cameraPos - fragPos);
    vec3 color = albedo * 0.2;
    float linear = 0.5;
    float quadratic = 1.0;
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
    FragColor = vec4(color, 1.0);
}
