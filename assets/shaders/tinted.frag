#version 330 core

in Varyings {
    vec4 color;
} fs_in;

// out vec4 frag_color;
layout (location = 0) out vec4 frag_color;
layout (location = 1) out vec4 BrightColor;


uniform vec4 tint;
uniform bool bloom = false;

void main(){
    //TODO: (Req 7) Modify the following line to compute the fragment color
    // by multiplying the tint with the vertex color
    frag_color = tint * fs_in.color;
    
    BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
    if (!bloom)
        return;
    float brightness = (frag_color.r + frag_color.g + frag_color.b) / 3.0;
    if (brightness > 0.7)
        BrightColor = frag_color;
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}