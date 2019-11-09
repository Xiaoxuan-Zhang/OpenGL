#version 330 core
#define EPSILON 0.0001
#define PI 3.1415926

uniform sampler2D texture_diffuse;
uniform sampler2D texture_normal;
uniform sampler2D texture_metallic;
uniform sampler2D texture_roughness;
uniform vec3 cameraPos;

out vec4 color;
in VS_OUT {
    vec3 normal;
    vec3 position;
    vec2 texCoords;
} vs_in;

//Distribution/concentration
float D_GGX(float dotnh, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float dotnh2 = dotnh * dotnh;
    float denom =  max(dotnh2 * (a2 - 1.0) + 1.0, EPSILON);
    return a2 /(PI * denom * denom);
}

float G_SGGX(float dotnv, float roughness)
{
    //for direct lighting
    float r = roughness + 1.0;
    
#ifdef IBL
    float k = roughness * roughness / 2.0;
#else
    float k = (r * r) / 8.0;
#endif
    
    return dotnv / (dotnv * (1.0 - k) + k);
}
//Geometry/shadowing masking
float G_Smith(float dotnv, float dotnl, float roughness)
{
    
    float ggx1 = G_SGGX(dotnv, roughness);
    float ggx2 = G_SGGX(dotnl, roughness);
    return ggx1 * ggx2;
}
//Fresnel/reflectivity
vec3 Fs(float dothv, vec3 f0)
{
    return max(f0 + (1.0 - f0) * pow((1.0 - dothv), 5.0), 0.0);
}

vec3 PBL()
{
    vec3 Lo = vec3(0.0);
    vec3 lights[2];
    vec3 lightColor = vec3(300.0, 300.0, 300.0);
    lights[0] = normalize(vec3(6.0, 8.0, -3.0));
    lights[1] = normalize(vec3(-6.0, 8.0, -3.0));
    
    vec3 viewDir = cameraPos - vs_in.position;
    vec3 V = normalize(viewDir);
    vec3 N = vs_in.normal;
    
    vec3 albedo = texture(texture_diffuse, vs_in.texCoords).rgb;
    float metallic = texture(texture_metallic, vs_in.texCoords).r;
    float roughness = texture(texture_roughness, vs_in.texCoords).r;

    for (int i = 0; i < 2; i++)
    {
        vec3 L = lights[i];
        vec3 H = normalize(V + L);
        
        vec3 radiance = lightColor;
        
        float dothv = max(dot(H, V), 0.0);
        float dotnh = max(dot(N, H), 0.0);
        float dotnv = max(dot(N, V), 0.0);
        float dotnl = max(dot(N, L), 0.0);
        
        //Cook-Torrance specular BRDF term
        vec3 f0 = vec3(0.04);
        f0 = mix(f0, albedo, metallic);
        vec3 F = Fs(dothv, f0);
        
        float D = D_GGX(dotnh, roughness);
        float GS = G_Smith(dotnv, dotnl, roughness);
        
        vec3 nom = D * GS * F;
        float denom = 4.0 * dotnv * dotnl;
        vec3 Fct = nom / max(denom, EPSILON); //avoid zero denom
        
        vec3 Ks = F; //specular
        vec3 Kd = vec3(1.0) - Ks; //diffuse
        Kd *= 1.0 - metallic;
        
        //Lambert
        vec3 Fl = albedo/PI;
        
        //final results
        Lo += (Kd * Fl + Fct) * radiance * dotnl;
    }
    vec3 ambient = vec3(0.03) * albedo;
    return ambient + Lo ;
}

void main() {
    vec3 pbl = PBL();
    pbl = pbl / (pbl + vec3(1.0));
    pbl = pow(pbl, vec3(1.0/2.2));
    color = vec4(pbl, 1.0);
}
