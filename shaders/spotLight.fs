#version 330 core
struct Material {
  sampler2D diffuse;
  sampler2D specular;
  float shininess;
};
struct Light {
  //vec3 position; //replaced with LightPosition since we need to operate in view space
  vec3 spotDirection;
  float cutOff;
  float outerCutOff;

  float constant;
  float linear;
  float quadratic;
  vec3 ambient;
  vec3 specular;
  vec3 diffuse;
};

uniform Material material;
uniform Light light;

in vec3 Normal;
in vec3 FragPosition;
in vec3 LightPosition;
in vec2 TexCoords;

out vec4 fragColor;

void main() {
  vec3 lightDir = normalize(-FragPosition); //light from the camera
  float theta = dot(lightDir, normalize(-light.spotDirection));
  float epsilon = abs(light.cutOff - light.outerCutOff);
  float intensity = clamp((theta - light.outerCutOff)/epsilon, 0.0, 1.0);
  if (theta > light.outerCutOff) {
    //ambient light
    vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;

    //diffuse light
    vec3 normal = normalize(Normal);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuseMap = texture(material.diffuse, TexCoords).rgb;
    vec3 diffuse = light.diffuse * diff * diffuseMap;

    //specular light
    vec3 viewDir = normalize(- FragPosition);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(reflectDir, viewDir), 0.0), material.shininess);
    vec3 specularMap = texture(material.specular, TexCoords).rgb;
    vec3 specular = light.specular * spec * specularMap;

    float distance = length(LightPosition - FragPosition);
    float attenuation = 1.0/(light.constant + light.linear * distance +
              light.quadratic * (distance * distance));

    diffuse *= attenuation;
    specular *= attenuation;

    diffuse *= intensity;
    specular *= intensity;

    fragColor = vec4((ambient + diffuse + specular) , 1.0);

  } else {
    fragColor = vec4(light.ambient * texture(material.diffuse, TexCoords).rgb, 1.0);
  }
}
