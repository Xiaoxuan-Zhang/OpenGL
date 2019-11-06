#version 330 core
#define NUM_POINT_LIGHTS 4
out vec4 FragColor;

in VS_OUT {
  vec3 FragPos;
  vec3 Normal;
  vec2 TexCoords;
  mat3 TBN;
  vec3 Tangent;
  vec3 Bitangent;
} fs_in;

struct Material {
  sampler2D texture_diffuse0;
  sampler2D texture_specular0;
  sampler2D texture_reflection0;
  sampler2D texture_normal0;
};

uniform samplerCube shadowMap;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float far_plane;
uniform Material material;
uniform vec3 pointLights[4];
float closestDepth = 0.0;
float outputDepth = 0.0;

float ShadowCalculation(vec3 fragPos)
{
  vec3 lightToFrag = fragPos - lightPos;
  closestDepth = texture(shadowMap, lightToFrag).r;
  outputDepth = texture(shadowMap, lightToFrag).r;
  closestDepth *= far_plane;
  float currentDepth = length(lightToFrag);
  float bias = 0.01;
  float shadow = currentDepth - bias > closestDepth? 1.0: 0.0;
  return shadow;
}
vec3 calcLighting(vec3 lightPos, vec3 normal, vec3 lightColor)
{
  vec3 color = texture(material.texture_diffuse0, fs_in.TexCoords).rgb;
  //ambient light
  vec3 ambient = 0.1 * color;
  //diffuse light
  vec3 lightDir = normalize(lightPos - fs_in.FragPos);
  float diff = max(dot(lightDir, normal), 0.0);
  vec3 diffuse = diff * lightColor;
  //specular light
  vec3 viewDir = normalize(viewPos - fs_in.FragPos);
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = 0.0;
  vec3 halfwayDir = normalize(lightDir + viewDir);
  spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
  vec3 specular = spec * lightColor ;

  float shadow = ShadowCalculation(fs_in.FragPos);
  vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
  vec3 lightingNoShadow = ambient + (diffuse + specular) * color;

  return lightingNoShadow;
}
void main()
{
  //vec3 normal = normalize(fs_in.Normal);
  vec3 normal = texture(material.texture_normal0, fs_in.TexCoords).rgb;
  normal = normalize(fs_in.TBN * (normal*2.0 - 1.0));
  vec3 lightColor = vec3(0.3);
  vec3 fragColor = vec3(0.0);
  for (int i = 0; i < NUM_POINT_LIGHTS; i++)
  {
    fragColor += calcLighting(pointLights[i], normal, lightColor);
  }
  FragColor = vec4(normal, 1.0);
  //FragColor = vec4(lightingNoShadow, 1.0);
  //FragColor = vec4(normal, 1.0);
  //FragColor = vec4(vec3(closestDepth/far_plane), 1.0);
  //FragColor = vec4(outputDepth, 0.0, 0.0, 1.0);
}
