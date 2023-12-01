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
    
// #version 330

// #define DIRECTIONAL 0
// #define POINT       1
// #define SPOT        2


// // Uniforms
// uniform sampler2D albedo;
// uniform sampler2D specular;
// uniform sampler2D roughness;
// uniform sampler2D ambient_occlusion;
// uniform sampler2D emission;


// struct Light {
//     int type;
//     vec3 position;
//     vec3 color;
//     vec3 direction;
//     float innerCutoff;
//     float outerCutoff;
//     vec3 attenuation;
// };

// in Varyings {
//     vec3 FragPos;
//     vec3 Normal;
//     vec2 TexCoord;
//     // vec3 ViewDir; // view direction
// } fs_in;

// uniform Light lights[50];
// uniform int numLights;

// uniform vec3 viewPos;

// // de el 7aga elly bn5rgha mn el fragment shader bt3na
// out vec4 frag_color;

// // de bt3br 3n el phong model. 
// float lambert(vec3 normal, vec3 world_to_light_direction) {
//     return max(0.0, dot(normal, world_to_light_direction));
// }


// float phong(vec3 reflected, vec3 view, float shininess) {
//     return pow(max(0.0, dot(reflected, view)), shininess);
// }

// void main() {
//     vec3 normal = normalize(fs_in.Normal);
//     vec3 view = normalize(viewPos - fs_in.FragPos);
    
//     vec3 ambient_light = vec3(0.1, 0.1, 0.1);

//     vec3 diffuse1 = texture(albedo, fs_in.TexCoord).rgb;
//     vec3 specular1 = texture(specular, fs_in.TexCoord).rgb;
//     float roughness1 = texture(roughness, fs_in.TexCoord).r;
//     vec3 ambient = diffuse1 * texture(ambient_occlusion, fs_in.TexCoord).r;
//     vec3 emissive = texture(emission, fs_in.TexCoord).rgb;

//     float shininess = 2.0 / pow(clamp(roughness1, 0.001, 0.999), 4.0) - 2.0;
    
//     vec3 world_to_light_dir;
//     vec3 color = emissive + ambient_light * ambient;

//     for(int light_idx = 0; light_idx < numLights; light_idx++){
//         Light light = lights[light_idx];
//         float attenuation = 1.0;
//         if(light.type == DIRECTIONAL){
//             world_to_light_dir = -light.direction;
//         } else {
//             world_to_light_dir = light.position - fs_in.FragPos;
//             float d = length(world_to_light_dir);
//             world_to_light_dir /= d;
//             attenuation = 1.0 / dot(light.attenuation, vec3(1.0, d, d*d));
//             if(light.type == SPOT){
//                 float angle = acos(dot(light.direction, -world_to_light_dir));
//                 attenuation *= smoothstep(light.outerCutoff, light.innerCutoff, angle);
//             }
//         }

//         vec3 computed_diffuse1 = light.color * diffuse1 * lambert(normal, world_to_light_dir);

//         vec3 reflected = reflect(-world_to_light_dir, normal);
//         vec3 computed_specular1 = light.color * specular1 * phong(reflected, view, shininess);

//         color += (computed_diffuse1 + computed_specular1) * attenuation;

//     }
    
//     frag_color = vec4(color, 1.0);
//     // frag_color = vec4(diffuse1.rgb, 1.0); // Debug output for albedo texture
//     // // or
//     frag_color = vec4(specular1.rgb, 1.0); // Debug output for specular texture

// }