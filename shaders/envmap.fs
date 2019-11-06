#version 330 core

uniform samplerCube skybox;
uniform vec3 cameraPos;

in vec3 Position;
in vec3 Normal;

in VS_OUT {
  vec3 Normal;
  vec3 Position;
} fs_in;

out vec4 FragColor;

void main()
{
    //reflect
    vec3 I = normalize(fs_in.Position - cameraPos);
    vec3 R_R = reflect(I, fs_in.Normal);
    vec3 reflect = texture(skybox, R_R).rgb;

    //refraction
    float ratio = 1.00 / 1.52;
    vec3 R_F = refract(I, normalize(fs_in.Normal), ratio);
    vec3 refract = texture(skybox, R_F).rgb;

    FragColor = vec4(refract, 1.0);


}
