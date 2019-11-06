#version 330 core
struct Material {
  sampler2D texture_diffuse1;
  sampler2D texture_specular1;
  float shininess;
};
struct Light {
  //vec3 direction;
  vec3 ambient;
  vec3 specular;
  vec3 diffuse;

  float constant;
  float linear;
  float quadratic;
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
  vec3 ambient = light.ambient * texture(material.texture_diffuse1, TexCoords).rgb;

  //diffuse light
  vec3 normal = normalize(Normal);
  vec3 lightDir = normalize(LightPosition - FragPosition);
  float diff = max(dot(lightDir, normal), 0.0);
  vec3 diffuseMap = texture(material.texture_diffuse1, TexCoords).rgb;
  vec3 diffuse = light.diffuse * diff * diffuseMap;

  //specular light
  vec3 viewDir = normalize(- FragPosition);
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(reflectDir, viewDir), 0.0), material.shininess);
  vec3 specularMap = texture(material.texture_specular1, TexCoords).rgb;
  vec3 specular = light.specular * spec * specularMap;

  //vec3 emissionMap = texture(emission, TexCoords).rgb ;

  float distance = length(LightPosition - FragPosition);
  float attenuation = 1.0/(light.constant + light.linear * distance +
    		    light.quadratic * (distance * distance));
  ambient *= attenuation;
  diffuse *= attenuation;
  specular *= attenuation;
  fragColor = vec4((ambient + diffuse + specular) , 1.0);
}
