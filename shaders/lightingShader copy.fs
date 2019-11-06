#version 330 core
struct Material {
  sampler2D diffuse;
  sampler2D specular;
  float shininess;
};
struct Light {
  vec3 ambient;
  vec3 specular;
  vec3 diffuse;
};

uniform Material material;
uniform Light light;
uniform sampler2D emission;
uniform float time;

in vec3 Normal;
in vec3 FragPosition;
in vec3 LightPosition;
in vec2 TexCoords;

out vec4 fragColor;

void main() {
  //ambient light
  vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;

  //diffuse light
  vec3 normal = normalize(Normal);
  vec3 lightDir = normalize(LightPosition - FragPosition);
  float diff = max(dot(lightDir, normal), 0.0);
  vec3 diffuseMap = texture(material.diffuse, TexCoords).rgb;
  vec3 diffuse = light.diffuse * diff * diffuseMap;

  //specular light
  vec3 viewDir = normalize(- FragPosition);
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(reflectDir, viewDir), 0.0), material.shininess);
  vec3 specularMap = texture(material.specular, TexCoords).rgb;
  vec3 specular = light.specular * spec * specularMap;

  vec3 emissionMap = texture(emission, TexCoords).rgb * abs(sin(0.5*time));

  fragColor = vec4((ambient + diffuse + specular + emissionMap) , 1.0);
}
