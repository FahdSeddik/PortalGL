#version 330 core

// Uniforms
uniform sampler2D albedoMap;
uniform sampler2D specularMap;
uniform sampler2D roughnessMap;
uniform sampler2D ambient_occlusionMap;
uniform sampler2D emissionMap;

#define DIRECTIONAL 0
#define POINT 1
#define SPOT 2

struct Light {
    int type;
    vec3 position;
    vec3 color;
    vec3 direction;
    float innerCutoff;
    float outerCutoff;
    vec3 attenuation;
};

in Varyings {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
    // vec3 ViewDir; // view direction
} fs_in;

uniform Light lights[50];
uniform int numLights;

uniform vec3 viewPos;

out vec4 frag_color;

void main() {
    // Retrieve texture values
    vec3 albedo = texture(albedoMap, fs_in.TexCoord).rgb;
    vec3 specular = texture(specularMap, fs_in.TexCoord).rgb;
    vec3 roughness = texture(roughnessMap, fs_in.TexCoord).rgb;
    vec3 ao = texture(ambient_occlusionMap, fs_in.TexCoord).rgb;
    vec3 emission = texture(emissionMap, fs_in.TexCoord).rgb;
    
    vec3 normal = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);

    // Calculate final color
    vec3 finalColor = vec3(0.0);
   
    for (int i = 0; i < numLights; i++) {
        Light light = lights[i];
        
        // Calculate light direction and distance
        vec3 lightDir; // the vector pointing from the fragment to the light source.
        float attenuation = 1.0;
        
        if (light.type == DIRECTIONAL) {
            lightDir = normalize(-light.direction); 
        } else if (light.type == POINT) {
            lightDir = normalize(light.position - fs_in.FragPos);
            float distance = length(light.position - fs_in.FragPos);
            attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * (distance * distance));
        } else if (light.type == SPOT) {
            lightDir = normalize(light.position - fs_in.FragPos);
            float distance = length(light.position - fs_in.FragPos);
            attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * (distance * distance));
            
            float theta = dot(lightDir, normalize(-light.direction)); // angle between light direction and spotlight direction
            float epsilon = light.innerCutoff - light.outerCutoff; // angle between inner and outer cutoff
            float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0); // intensity of spotlight
            attenuation *= intensity;
           
        }
        
        // Calculate diffuse and specular components
        vec3 reflectDir = reflect(-lightDir, normal);
        
        float diffuseFactor = max(dot(normal, lightDir), 0.0); // diffuse factor is the angle between the normal and the light direction
        vec3 diffuse = light.color * albedo.rgb * diffuseFactor;
        
        float specularFactor = pow(max(dot(viewDir, reflectDir), 0.0), roughness.r);
        vec3 specular = light.color * specular.rgb * specularFactor;
        
        // Add light contribution to final color
        finalColor += (diffuse + specular) * attenuation;
        // finalColor = diffuse;
    }
    
    // Add ambient occlusion and emission
    finalColor *= ao.rgb;
    finalColor += emission.rgb;
    
    // Output final color
    frag_color = vec4(finalColor, 1.0);



}
    