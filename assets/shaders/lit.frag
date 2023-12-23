#version 330 core

// Uniforms
uniform sampler2D albedoMap;
uniform sampler2D specularMap;
uniform sampler2D roughnessMap;
uniform sampler2D ambient_occlusionMap;
uniform sampler2D emissionMap;
uniform sampler2D metallicMap;

#define DIRECTIONAL 0
#define POINT 1
#define SPOT 2

struct Light {
    int type;
    vec3 position;
    vec3 color;
    vec3 direction;
    float innerCutOff;
    float outerCutOff;
    vec3 attenuation;
};

in Varyings {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
} fs_in;

uniform Light lights[8];
uniform int numLights;
uniform float alphaThreshold;
uniform vec3 viewPos;
uniform bool bloom = false;


layout (location = 0) out vec4 frag_color;
layout (location = 1) out vec4 BrightColor;


const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
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
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------

void main() {
    if(texture(albedoMap, fs_in.TexCoord).a < alphaThreshold) discard; // Discard the fragment if the alpha value of the albedo map is less than 0.1 (transparent)
    vec3 albedo = texture(albedoMap, fs_in.TexCoord).rgb; // Retrieve the albedo (color) of the material from the albedo map
    vec3 specular = texture(specularMap, fs_in.TexCoord).rgb; // Retrieve the specular reflection color from the specular map
    float roughness = texture(roughnessMap, fs_in.TexCoord).r; // Retrieve the roughness value from the roughness map
    vec3 ao = texture(ambient_occlusionMap, fs_in.TexCoord).rgb; // Retrieve the ambient occlusion value from the ambient occlusion map
    vec3 emission = texture(emissionMap, fs_in.TexCoord).rgb; // Retrieve the emission color from the emission map
    float metallic = texture(metallicMap, fs_in.TexCoord).r; // Retrieve the metallic value from the metallic map
    
    vec3 N = normalize(fs_in.Normal); // Normalize the surface normal
    vec3 V = normalize(viewPos - fs_in.FragPos); // Calculate the view vector
    vec3 R = reflect(-V, N); // Calculate the reflection vector

    vec3 F0 = vec3(0.04); // Set the base reflectance value
    F0 = mix(F0, albedo, metallic); // (Optional) Mix the base reflectance with the albedo based on a metallic value

    vec3 Lo = vec3(0.0); // Initialize the outgoing light color
    for(int i = 0; i < numLights; ++i) // Loop through each light source
    {
        Light light = lights[i]; // Get the current light source
        vec3 L;
        float attenuation = 1.0;
        if (light.type == DIRECTIONAL) {
            L = normalize(-light.direction); // Calculate the light direction for a directional light
        } else if (light.type == POINT) {
            L = normalize(light.position - fs_in.FragPos); // Calculate the light direction for a point light
            float distance = length(light.position - fs_in.FragPos); // Calculate the distance between the fragment and the light source
            attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * (distance * distance)); // Calculate the attenuation factor based on the light's attenuation properties
        } else if (light.type == SPOT) {
            L = normalize(light.position - fs_in.FragPos); // Calculate the light direction for a spot light
            float distance = length(light.position - fs_in.FragPos); // Calculate the distance between the fragment and the light source
            attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * (distance * distance)); // Calculate the attenuation factor based on the light's attenuation properties
        
            float theta = dot(L, normalize(-light.direction)); // Calculate the angle between the light direction and the spotlight direction
            float epsilon = light.innerCutOff - light.outerCutOff; // Calculate the angle between the inner and outer cutoff angles of the spotlight
            float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0); // Calculate the intensity of the spotlight based on the angle
            attenuation *= intensity; // Apply the spotlight intensity to the attenuation factor
        
        }
        vec3 H = normalize(V + L); // Calculate the halfway vector between the view vector and the light vector

        vec3 radiance = light.color * attenuation; // Calculate the radiance (light color multiplied by the attenuation factor)

        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0); // Calculate the Fresnel term using the Schlick approximation
        float NDF = DistributionGGX(N, H, roughness); // Calculate the Normal Distribution Function (NDF) using the GGX distribution
        float G   = GeometrySmith(N, V, L, roughness); // Calculate the Geometry term using the Smith method      
        vec3 nominator    = NDF * G * F; // Calculate the numerator of the specular reflection equation
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0); // Calculate the denominator of the specular reflection equation
        vec3 specular = nominator / max(denominator, 0.001); // Calculate the specular reflection color

        vec3 kS = F; // Set the specular reflection color as the specular reflection coefficient
        vec3 kD = vec3(1.0) - kS; // Calculate the diffuse reflection coefficient
        // Adjust the diffuse reflection coefficient based on the metalness
        kD *= 1.0 - metallic;
        
        float NdotL = max(dot(N, L), 0.0); // Calculate the dot product between the surface normal and the light direction

        Lo += (kD * albedo / PI + specular) * radiance * NdotL; // Calculate the outgoing light color for this light source
    }

    vec3 ambient = vec3(0.1) * albedo * ao; // Calculate the ambient light color
    vec3 color = ambient + Lo + emission; // Calculate the final color by adding the ambient light, outgoing light, and emission color

    BrightColor = vec4(0.0, 0.0, 0.0, 0.0);
    if (bloom) {
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 0.9)
        BrightColor = vec4(color, texture(albedoMap, fs_in.TexCoord).a);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, texture(albedoMap, fs_in.TexCoord).a);
    }

    frag_color = vec4(color, texture(albedoMap, fs_in.TexCoord).a); // Set the fragment color to the final color
}
    