#version 330 core

uniform sampler2D floorTexture;
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
  vec4 basicTexture = texture(floorTexture, fs_in.TexCoords);
  vec3 color = basicTexture.rgb;
  vec3 ambient = 0.4 * color;
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

  //fragColor = vec4(basicTexture.a, 0.0, 0.0, 1.0);

  if (basicTexture.a < 1.0) {
    discard;
  }
  fragColor = vec4(ambient + diffuse + specular, 1.0);

}
