#version 330 core

in Varyings {
    vec3 position;
    vec4 color;
    vec2 tex_coord;
    vec3 normal;
} fs_in;

out vec4 frag_color;

uniform sampler2D tex;

void main(){
    //TODO: (Req 5) Change the following line to read the fragment color
    // from the texture at the received texture coordinates

    //texture function takes in a sampler2D (the texture), and a vec2 (the texture coordinates)
    //it returns a vec4 (the color of the texture at the given coordinates)
    frag_color = texture(tex, fs_in.tex_coord);
}