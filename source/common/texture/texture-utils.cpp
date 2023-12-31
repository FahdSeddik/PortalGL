#include "texture-utils.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <iostream>

portal::Texture2D* portal::texture_utils::empty(GLenum format, glm::ivec2 size){
    portal::Texture2D* texture = new portal::Texture2D();
    //TODO: (Req 11) Finish this function to create an empty texture with the given size and format
    texture->bind();
    glTexStorage2D(GL_TEXTURE_2D, 1, format, size.x, size.y);
    texture->unbind();
    return texture;
}

portal::Texture2D* portal::texture_utils::loadImage(const std::string& filename, bool generate_mipmap) {
    glm::ivec2 size;
    int channels;
    //Since OpenGL puts the texture origin at the bottom left while images typically has the origin at the top left,
    //We need to till stb to flip images vertically after loading them
    stbi_set_flip_vertically_on_load(true);
    //Load image data and retrieve width, height and number of channels in the image
    //The last argument is the number of channels we want and it can have the following values:
    //- 0: Keep number of channels the same as in the image file
    //- 1: Grayscale only
    //- 2: Grayscale and Alpha
    //- 3: RGB
    //- 4: RGB and Alpha (RGBA)
    //Note: channels (the 4th argument) always returns the original number of channels in the file
    unsigned char* pixels = stbi_load(filename.c_str(), &size.x, &size.y, &channels, 4);
    if(pixels == nullptr){
        std::cerr << "Failed to load image: " << filename << std::endl;
        return nullptr;
    }
    // Create a texture
    portal::Texture2D* texture = new portal::Texture2D();
    //Bind the texture such that we upload the image data to its storage
    //TODO: (Req 5) Finish this function to fill the texture with the data found in "pixels"

    //Bind the texture
    texture->bind();
    //Load the image data into the texture using glTexImage2D
    //The arguments are:
    //- GL_TEXTURE_2D: The texture target
    //- 0: The mipmap level we want to load the image into (0 is the base image level)
    //- GL_RGBA: The internal format of the texture (RGBA with 8 bits per channel)
    //- size.x, size.y: The width and height of the image
    //- 0: Border size (must be 0)
    //- GL_RGBA: The format of the pixel data we are uploading
    //- GL_UNSIGNED_BYTE: The type of the pixel data we are uploading
    //- pixels: The actual pixel data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    //Generate mipmaps if needed 
    glGenerateMipmap(GL_TEXTURE_2D);
    //Unbind the texture
    texture->unbind();
    
    stbi_image_free(pixels); //Free image data after uploading to GPU
    return texture;
}