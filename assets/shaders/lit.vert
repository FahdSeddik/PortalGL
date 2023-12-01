
#version 330 core

// Input vertex attributes
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 tex_coord;
layout(location = 3) in vec3 normal;

// Output vertex attributes
// out vec3 FragPos;
// out vec3 Normal;
out Varyings {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
    // vec3 camPos; // view direction
} vs_out;

// Uniforms
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
// uniform mat4 transform;

void main()
{
    // Transform the vertex position and normal
    vec4 worldPos = model * vec4(position, 1.0);
    vs_out.FragPos = vec3(worldPos);
    vs_out.Normal = normalize(transpose(inverse(model))* vec4(normal, 0.0)).xyz;

    // Calculate the final position of the vertex
    gl_Position = projection * view * worldPos;

    // calculate the view direction
    // inverse(view) * vec4(0.0, 0.0, 0.0, 1.0)  => calculates the position of the camera in world space
    // vs_out.ViewDir = normalize(vec3(inverse(view) * vec4(0.0, 0.0, 0.0, 1.0) - worldPos));

    vs_out.TexCoord = tex_coord;
}
