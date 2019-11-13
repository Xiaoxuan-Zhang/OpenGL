#version 330 core

struct Material {
  sampler2D texture_diffuse1;
  sampler2D texture_specular1;
  sampler2D texture_reflection1;
  sampler2D texture_normal1;
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
    vec3 diffuse = texture(material.texture_diffuse1, fs_in.TexCoords).rgb;
    vec3 specular = texture(material.texture_specular1, fs_in.TexCoords).rgb;
    
    vec3 I = normalize(fs_in.Position - cameraPos);
    vec3 nor = normalize(fs_in.Normal);
    
    //reflection
    vec3 R_R = reflect(I, nor);
    vec3 reflCol = texture(cubemap, R_R).rgb;

    //reflection map
    float reflect_intensity = texture(material.texture_reflection1, fs_in.TexCoords).r;
    vec3 reflect_color;
    reflect_color = reflCol * reflect_intensity;

    FragColor = vec4(diffuse + reflect_color, 1.0);
}
