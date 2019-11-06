#version 330 core
#define NUM_POINT_LIGHTS 4

struct Material {
  sampler2D texture_diffuse1;
  sampler2D texture_specular1;
  float shininess;
};
struct DirLight {
  vec3 direction;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  vec3 strength;
};

struct PointLight {
  vec3 position;
  float constant;
  float linear;
  float quadratic;
  vec3 ambient;
  vec3 specular;
  vec3 diffuse;
  float strength;
};

struct SpotLight {
  vec3 position;
  vec3 direction;
  float cutOff;
  float outerCutOff;
  vec3 ambient;
  vec3 specular;
  vec3 diffuse;
  float constant;
  float linear;
  float quadratic;
};

uniform vec3 viewPosition;
uniform Material material;
uniform DirLight directionalLight;
uniform PointLight pointLights[NUM_POINT_LIGHTS];
uniform SpotLight spotLight;

in vec3 Normal;
in vec3 FragPosition;
in vec2 TexCoords;

out vec4 FragColor;

vec3 calcDirLight(DirLight dirLight, vec3 normal, vec3 viewDir);
vec3 calcPointLight(PointLight pointLight, vec3 fragPosition, vec3 normal, vec3 viewDir);
vec3 calcSpotLight(SpotLight spotLight, vec3 fragPosition, vec3 normal, vec3 viewDir);
float calcSpecFactor(vec3 lightDir, vec3 viewDir, vec3 normal, float shininess);
float calcBlinnSpecFactor(vec3 lightDir, vec3 viewDir, float shininess);
void main()
{
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPosition-FragPosition);
    vec3 result = vec3(0.0);
    result = calcDirLight(directionalLight, normal, viewDir);

    for (int i =0; i < NUM_POINT_LIGHTS; i ++) {
      result += calcPointLight(pointLights[i], FragPosition, normal, viewDir);
    }
    result += calcSpotLight(spotLight, FragPosition, normal, viewDir);
    FragColor = vec4(result, 1.0);
}
float calcSpecFactor(vec3 lightDir, vec3 viewDir, vec3 normal, float shininess) {
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(reflectDir, viewDir), 0.0), shininess);
  return spec;
}
float calcBlinnSpecFactor(vec3 lightDir, vec3 viewDir, vec3 normal, float shininess) {
  vec3 halfwayDir = normalize(lightDir + viewDir);
  float spec = pow(max(dot(halfwayDir, normal), 0.0), shininess);
  return spec;
}
vec3 calcDirLight(DirLight dirLight, vec3 normal, vec3 viewDir) {
  //ambient
  vec3 ambient = dirLight.ambient * texture(material.texture_diffuse1, TexCoords).rgb;

  //diffuse
  vec3 lightDir = normalize(-dirLight.direction); //pointing towards light

  float diff = max(dot(lightDir, normal), 0.0);
  vec3 diffuse = dirLight.diffuse * diff * texture(material.texture_diffuse1, TexCoords).rgb;

  //specular
  float spec = calcSpecFactor(lightDir, viewDir, normal, material.shininess);
  //vec3 reflectDir = reflect(-lightDir, normal);
  //float spec = pow(max(dot(reflectDir, viewDir) , 0.0), material.shininess);
  vec3 specular = dirLight.specular * spec * texture(material.texture_specular1, TexCoords).rgb;

  return (ambient + diffuse + specular) * dirLight.strength;
}

vec3 calcPointLight(PointLight pointLight, vec3 fragPosition, vec3 normal, vec3 viewDir) {

  //ambient
  vec3 ambient = pointLight.ambient * texture(material.texture_diffuse1, TexCoords).rgb;

  //diffuse
  vec3 lightDir = normalize(pointLight.position - fragPosition);
  float diff = max(dot(lightDir, normal), 0.0);
  vec3 diffuse = pointLight.diffuse * diff * texture(material.texture_diffuse1, TexCoords).rgb;

  //specular
  float spec = calcSpecFactor(lightDir, viewDir, normal, material.shininess);
  //vec3 reflectDir = reflect(-lightDir, normal);
  //float spec = pow(max(dot(reflectDir, viewDir) , 0.0), material.shininess);
  vec3 specular = pointLight.specular * spec * texture(material.texture_specular1, TexCoords).rgb;

  float distance = length(pointLight.position - fragPosition);
  float attenuation = 1.0/(pointLight.constant + pointLight.linear * distance +
    		    pointLight.quadratic * (distance * distance));
  ambient *= attenuation;
  diffuse *= attenuation;
  specular *= attenuation;

  return (ambient + diffuse + specular) * pointLight.strength;
}

vec3 calcSpotLight(SpotLight spotLight, vec3 fragPosition, vec3 normal, vec3 viewDir) {
  vec3 resultColor = vec3(0.0);
  vec3 lightDir = normalize(spotLight.position-fragPosition);
  vec3 spotDir = normalize(-spotLight.direction);
  float theta = dot(lightDir, spotDir);
  float epsilon = abs(spotLight.cutOff - spotLight.outerCutOff);
  float intensity = clamp((theta - spotLight.outerCutOff)/epsilon, 0.0, 1.0);
  //ambient
  vec3 ambient = spotLight.ambient * texture(material.texture_diffuse1, TexCoords).rgb;
  if (theta > spotLight.outerCutOff) {
    //light up

    //diffuse
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = spotLight.diffuse * diff * texture(material.texture_diffuse1, TexCoords).rgb;

    //specular
    float spec = calcSpecFactor(lightDir, viewDir, normal, material.shininess);
    //vec3 reflectDir = reflect(-lightDir, normal);
    //float spec = pow(max(dot(reflectDir, viewDir) , 0.0), material.shininess);
    vec3 specular = spotLight.specular * spec * texture(material.texture_specular1, TexCoords).rgb;

    float distance = length(spotLight.position - fragPosition);
    float attenuation = 1.0/(spotLight.constant + spotLight.linear * distance +
      		    spotLight.quadratic * (distance * distance));
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    resultColor = (ambient + diffuse + specular);
  }
  return resultColor;
}
