#version 330 core

in VS_OUT {
  vec2 TexCoords;
  vec3 Normal;
  vec3 FragPosition;
  vec4 FragPosLightSpace;
} fs_in;

out vec4 FragColor;
struct Material {
  sampler2D texture_diffuse0;
  sampler2D texture_specular0;
  sampler2D texture_normal0;
};
uniform sampler2D shadowMap;

uniform Material material;
uniform vec3 lightPos;
uniform vec3 viewPos;

float calculateShadow(vec4 FragPosLightSpace, float bias) {
  //transform from clip space to NDC
  vec3 projCoords = FragPosLightSpace.xyz/FragPosLightSpace.w;
  //transform from NDC to range(0, 1) to align with depth depthMap
  projCoords = projCoords * 0.5 + 0.5;
  float closestDepth = texture(shadowMap, projCoords.xy).r;
  float currentDepth = projCoords.z;
  float shadow = 0.0;
  vec2 texelSize = 1.0 / (textureSize(shadowMap, 0));
  for(int x = -1; x <= 1; ++x)
  {
      for(int y = -1; y <= 1; ++y)
      {
          float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
          shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
      }
  }
  shadow /= 9.0;

  if(projCoords.z > 1.0)
        shadow = 0.0;

  return shadow;
}

void main() {
  vec4 basicTexture = texture(material.texture_diffuse0, fs_in.TexCoords);
  vec3 color = vec3(0.3);
  vec3 ambient = 0.4 * basicTexture.rgb;
  vec3 lightDir = normalize(lightPos - fs_in.FragPosition);
  vec3 viewDir = normalize(viewPos - fs_in.FragPosition);
  vec3 normal = normalize(fs_in.Normal);
  //diffuse
  float diff = max(dot(lightDir, normal), 0.0);
  vec3 diffuse = diff * color;
  //specular
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = 0.0;
  vec3 halfwayDir = normalize(lightDir + viewDir);
  spec = pow(max(dot(halfwayDir, normal), 0.0), 32);
  vec3 specular = spec * vec3(0.3);
  //float bias = 0.005;
  float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
  float shadow = calculateShadow(fs_in.FragPosLightSpace, bias);
  vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
  FragColor = vec4(lighting, 1.0);

}
