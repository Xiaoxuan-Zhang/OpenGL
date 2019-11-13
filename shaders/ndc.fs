#version 330 core
uniform sampler2D texture_diffuse;

in vec2 TexCoords;
out vec4 FragColor;

void main(){
    vec3 tex = texture(texture_diffuse, TexCoords).rgb;
    FragColor = vec4(tex, 1.0);
}
