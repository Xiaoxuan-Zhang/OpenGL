#version 330 core

struct Material {
  sampler2D texture_diffuse1;
  sampler2D texture_specular1;
  sampler2D texture_reflection1;
  float shininess;
};

uniform Material material;
uniform vec3 cameraPos;
uniform samplerCube cubemap;

out vec4 FragColor;

in VS_OUT {
  vec3 Normal;
  vec2 TexCoords;
  vec3 Position;
} fs_in;


void main()
{
  vec3 I = normalize(fs_in.Position - cameraPos);
  //reflection
  vec3 R_R = reflect(I, fs_in.Normal);
  vec4 reflect = texture(cubemap, R_R);

  //refraction
  float ratio = 1.00 / 1.52;
  vec3 R_F = refract(I, normalize(fs_in.Normal), ratio);
  vec4 refract = texture(cubemap, R_F);

  //diffuse
  vec4 diffuse =  texture(material.texture_diffuse1, fs_in.TexCoords);

  //reflection map
  float reflect_intensity = texture(material.texture_reflection1, fs_in.TexCoords).r;
  vec4 reflect_color;
  reflect_color = reflect * reflect_intensity;

  FragColor = reflect_color + diffuse ;
}
