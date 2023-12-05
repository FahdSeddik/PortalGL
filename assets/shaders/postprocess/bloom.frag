#version 330 core
out vec4 FragColor;

in vec2 tex_coord;

uniform sampler2D tex1; //scene;
uniform sampler2D tex2; //bloomBlur;
uniform float exposure;
uniform float bloomIntensity;

void main()
{             
    const float gamma = 2.2;
    vec3 hdrColor = texture(tex1, tex_coord).rgb;
    vec3 bloomColor = texture(tex2, tex_coord).rgb;
    hdrColor += bloomColor * bloomIntensity;
    // vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    // vec3 result = hdrColor / (hdrColor + vec3(1.0));
    // result = pow(result, vec3(1.0 / gamma));
    FragColor = vec4(hdrColor, 1.0);
}