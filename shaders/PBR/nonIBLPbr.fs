#version 330 core
#define PI 3.14159265359
#define MAX_REFLECTION_LOD 4.0

uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;
uniform samplerCube irradianceMap;
uniform samplerCube env_mipmap;
uniform sampler2D brdf_LUT;
uniform vec3 cameraPos;

out vec4 FragColor;

in VS_OUT {
    vec3 normal;
    vec3 position;
    vec2 texCoords;
} vs_in;

//Distribution/concentration
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
    
    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;
    
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 PBL()
{
    vec3 Lo = vec3(0.0);
    vec3 lights[2];
    vec3 lightColor = vec3(300.0, 300.0, 300.0);
    lights[0] = vec3(10.0, 20.0, 10.0);
    lights[1] = vec3(-10.0, 20.0, 15.0);
    
    vec3 viewDir = cameraPos - vs_in.position;
    vec3 V = normalize(viewDir);
    vec3 N = normalize(vs_in.normal);
    vec3 R = reflect(-V, N);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    
    for (int i = 0; i < 2; i++)
    {
        vec3 lightDir = lights[i] - vs_in.position;
        vec3 L = normalize(lightDir);
        vec3 H = normalize(V + L);
        
        float dist = length(lightDir);
        float att = 1.0 / (dist * dist);
        vec3 radiance = lightColor * att;
        
        //Cook-Torrance BRDF term
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F    = FresnelSchlick(max(dot(H, V), 0.0), F0);
        
        vec3 nom = NDF * G * F;
        float denom = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; //avoid zero denom
        vec3 specular = nom / denom;
        
        vec3 Ks = F; //specular
        vec3 Kd = 1.0 - Ks; //diffuse
        Kd *= 1.0 - metallic;
        
        float NdotL = max(dot(N, L), 0.0);
        //final results
        Lo += (Kd * albedo / PI + specular) * radiance * NdotL;
    }
    
    //ambient light
    float NdotV = max(dot(N, V), 0.0);
    vec3 F = FresnelSchlickRoughness(NdotV, F0, roughness);
    vec3 Ks = F;
    vec3 Kd = 1.0 - Ks;
    Kd *= 1.0 - metallic;
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;
    vec3 prefilteredCol = textureLod(env_mipmap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(brdf_LUT, vec2(NdotV, roughness)).rg;
    vec3 specular = prefilteredCol * (F * brdf.x + brdf.y);
    float ao = 1.0; //replace with a texture 
    vec3 ambient = (Kd * diffuse + specular) * ao;
    return ambient + Lo;
}

vec3 basicShading() {
    vec3 albedo = vec3(1.0, 0.0, 0.0);
    vec3 lightDir = normalize(vec3(1.0, 10.0, 5.0));
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    vec3 viewDir = normalize(cameraPos - vs_in.position);
    vec3 nor = normalize(vs_in.normal);
    vec3 refl = reflect(-lightDir, nor);
    float diff = max(dot(nor, lightDir), 0.0);
    vec3 diffCol = lightColor * diff;
    float spec = pow(max(dot(viewDir, refl), 0.0), 32.0);
    vec3 specCol = spec * lightColor;
    return (diffCol + 0.5*specCol) * albedo;
}
void main() {
    vec3 pbl = PBL();
    //vec3 phongCol = basicShading();
    pbl = pbl / (pbl + vec3(1.0));
    pbl = pow(pbl, vec3(1.0/2.2));
    FragColor = vec4(pbl, 1.0);
}
