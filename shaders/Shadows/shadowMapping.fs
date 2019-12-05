#version 330 core
#define GAMMA 2.2
#define EXPOSURE 1.0
#define ATT_LINEAR 0.2
#define ATT_QUADRATIC 0.5

struct Material
{
  sampler2D texture_diffuse1;
  sampler2D texture_specular1;
  sampler2D texture_reflection1;
  sampler2D texture_normal1;
  float shininess;
};

uniform Material material;
uniform sampler2D depthMap;
uniform vec3 lightPos;
uniform vec3 cameraPos;

out vec4 FragColor;

in VS_OUT
{
    vec3 Normal;
    vec2 TexCoords;
    vec3 Position;
    vec4 FragPosLightSpace;
} fs_in;

float calcShadow(float bias)
{
    vec3 projectedPos = fs_in.FragPosLightSpace.xyz / fs_in.FragPosLightSpace.w;
    projectedPos = projectedPos * 0.5 + 0.5;
    vec2 texSize = textureSize(depthMap, 0);
    float steps = 0.0;
    float currDepth = projectedPos.z;
    float shadow = 0.0;
    //anti-aliasing
    for (float x = -1.0; x <= 1.0; ++x)
    {
        for (float y = -1.0; y <= 1.0; ++y)
        {
            float closestDepth = texture(depthMap, projectedPos.xy + vec2(x, y)/texSize).r;
            shadow += currDepth - closestDepth > 0.005 ? 1.0 : 0.0;
            steps++;
        }
    }
    shadow /= steps;
    if (projectedPos.z > 1.0)
    {
        shadow = 0.0;
    }
    return shadow;
}

void main()
{
    //diffuse
    vec3 diffuseTex = texture(material.texture_diffuse1, fs_in.TexCoords).rgb;
    vec3 lightCol = vec3(30.0);
    
    vec3 nor = normalize(fs_in.Normal);
    vec3 viewDir = normalize(cameraPos - fs_in.Position);
    vec3 lightDir = normalize(lightPos - fs_in.Position);
    vec3 halfDir = normalize(lightDir + viewDir);
    
    float diff = max(dot(lightDir, nor), 0.0);
    vec3 diffCol = diff * diffuseTex * lightCol;
    
    float specular = pow(max(dot(halfDir, nor), 0.0), 16.0);
    vec3 specCol = specular * diffuseTex * lightCol;
    
    float bias = max(0.05 * (1.0 - dot(nor, lightDir)), 0.005);
    float sd = calcShadow(bias);
    float ambient = 0.05;
    
    float dist = length(lightPos - fs_in.Position);
    float att = 1.0 / (1.0 + ATT_LINEAR * dist + ATT_QUADRATIC * dist * dist);
    
    vec3 col = ambient * diffuseTex + ((1.0 - sd) * (diffCol + specCol) * att);
    
    // tone mapping
    vec3 rst = vec3(1.0) - exp(-col * EXPOSURE);
    
    //gamma correction
    col = pow(col, vec3(1.0/GAMMA));
    FragColor = vec4(col, 1.0);
}
