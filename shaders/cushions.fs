#version 330 core
struct Material {
  sampler2D texture_diffuse1;
  sampler2D texture_specular1;
  float shininess;
};
uniform Material material;
uniform vec3 viewPosition;
uniform vec3 lightPosition;
uniform bool blinn;
uniform float shininess;

in VS_OUT {
  vec2 TexCoords;
  vec3 Normal;
  vec3 FragPosition;
} fs_in;

out vec4 fragColor;

void main() {

  vec3 color = texture(material.texture_diffuse1, fs_in.TexCoords).rgb;
  vec3 ambient = 0.05 * color;
  vec3 lightDir = normalize(lightPosition - fs_in.FragPosition);
  vec3 viewDir = normalize(viewPosition - fs_in.FragPosition);
  vec3 normal = normalize(fs_in.Normal);
  //diffuse
  float diff = max(dot(lightDir, normal), 0.0);
  vec3 diffuse = diff * color;
  //specular
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = 0.0;
  if (blinn) {
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(halfwayDir, normal), 0.0), 32);
  } else {
     spec = pow(max(dot(reflectDir, viewDir), 0.0), 8);
  }
  vec3 specular = spec * vec3(0.3);
  fragColor = vec4(ambient + diffuse + specular, 1.0);

}
