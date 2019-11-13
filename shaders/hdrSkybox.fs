
#version 330 core
out vec4 FragColor;

in vec3 localPos;

uniform samplerCube hdrSkybox;

void main()
{
    //vec3 envCol = texture(hdrSkybox, localPos).rgb;
    vec3 envCol = textureLod(hdrSkybox, localPos, 0.0).rgb;
    envCol = envCol / (envCol + vec3(1.0));
    envCol = pow(envCol, vec3(1.0/2.2));
    FragColor = vec4(envCol, 1.0);
}
