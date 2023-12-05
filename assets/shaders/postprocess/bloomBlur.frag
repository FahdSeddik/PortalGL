#version 330 core
out vec4 FragColor;

in vec2 tex_coord;

uniform sampler2D tex1; // the texture of the ping-pong framebuffer [1]
uniform sampler2D tex2; // the texture of the ping-pong framebuffer [0]

uniform bool horizontal;
uniform float weight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

void main()
{             

     vec3 result = vec3(0.0);
     if(horizontal)
     {
        vec2 tex_offset = 1.0 / textureSize(tex2, 0); // gets size of single texel
        result += texture(tex2, tex_coord).rgb * weight[0];
         for(int i = 1; i < 5; ++i)
         {
            result += texture(tex2, tex_coord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(tex2, tex_coord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
         }
     }
     else
     {
        vec2 tex_offset = 1.0 / textureSize(tex1, 0); // gets size of single texel
        result += texture(tex1, tex_coord).rgb * weight[0];
         for(int i = 1; i < 5; ++i)
         {
             result += texture(tex1, tex_coord + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
             result += texture(tex1, tex_coord - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
         }
     }
    FragColor = vec4(result, 1.0);
    // FragColor = vec4(1.0, 1.0, 0.0, 1.0);
}