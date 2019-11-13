
#version 330 core
out vec4 FragColor;

in vec3 localPos;

uniform sampler2D hdrTexture;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 sampleSphereicalMap(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}
void main()
{
    vec2 uv = sampleSphereicalMap(localPos);
    FragColor = vec4(texture(hdrTexture, uv).rgb, 1.0);
}
