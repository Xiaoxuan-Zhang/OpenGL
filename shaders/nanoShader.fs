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
    //diffuse
    vec4 diffuse = texture(material.texture_diffuse1, fs_in.TexCoords);
    
    vec3 I = normalize(fs_in.Position - cameraPos);
    vec3 nor = normalize(fs_in.Normal);
    
    //reflection
    vec3 R_R = reflect(I, nor);
    vec4 reflCol = texture(cubemap, R_R);

    //reflection map
    float reflect_intensity = texture(material.texture_reflection1, fs_in.TexCoords).r;
    vec4 reflect_color;
    reflect_color = reflCol * reflect_intensity;

    FragColor = diffuse + reflect_color;
}
