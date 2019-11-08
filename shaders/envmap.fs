#version 330 core

uniform samplerCube skybox;
uniform vec3 cameraPos;

in VS_OUT {
  vec3 Normal;
  vec3 Position;
} fs_in;

out vec4 FragColor;

void main()
{
    //reflect
    vec3 I = normalize(fs_in.Position - cameraPos);
    vec3 nor = normalize(fs_in.Normal);
    vec3 R_R = reflect(I, nor);
    vec3 reflCol = texture(skybox, R_R).rgb;

    //refraction
    float ratio = 1.00 / 1.1;
    vec3 R_F = refract(I, nor, ratio);
    vec3 refrCol = texture(skybox, R_F).rgb;

    FragColor = vec4(reflCol, 1.0);


}
