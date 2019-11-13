
#version 330 core
out vec4 FragColor;
in vec3 localPos;

uniform samplerCube hdrSkybox;
const float PI = 3.14159265359;
const float sampleDelta = 0.025;

void main()
{
    vec3 normal = normalize(localPos); //unit sphere
    vec3 irradiance = vec3(0.0);
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = cross(up, normal);
    up = cross(normal, right);
    float sampleNum = 0.0;
    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            //spherical to cartesian in tangent space
            vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            //tangent to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;
            irradiance += texture(hdrSkybox, sampleVec).rgb * cos(theta) * sin(theta);
            sampleNum++;
        }
    }
    irradiance = PI * irradiance * (1.0 / sampleNum);
    FragColor = vec4(irradiance, 1.0);
}