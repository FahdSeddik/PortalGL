#include "material.hpp"

#include "../asset-loader.hpp"
#include "deserialize-utils.hpp"

namespace portal {

    // This function should setup the pipeline state and set the shader to be used
    void Material::setup() const {
        //TODO: (Req 7) Write this function
        //setting up the pipeline state 
        pipelineState.setup();
        //setting the shader to be used
        shader->use();
    }

    // This function read the material data from a json object
    void Material::deserialize(const nlohmann::json& data){
        if(!data.is_object()) return;

        if(data.contains("pipelineState")){
            pipelineState.deserialize(data["pipelineState"]);
        }
        shader = AssetLoader<ShaderProgram>::get(data["shader"].get<std::string>());
        transparent = data.value("transparent", false);
    }

    // This function should call the setup of its parent and
    // set the "tint" uniform to the value in the member variable tint 
    void TintedMaterial::setup() const {
         //TODO: (Req 7) Write this function
         //calling the setup of its parent
        Material::setup();
        //setting the "tint" uniform to the value in the member variable tint
        shader->set("tint", tint);
    }

    // This function read the material data from a json object
    void TintedMaterial::deserialize(const nlohmann::json& data){
        Material::deserialize(data);
        if(!data.is_object()) return;
        tint = data.value("tint", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    // This function should call the setup of its parent and
    // set the "alphaThreshold" uniform to the value in the member variable alphaThreshold
    // Then it should bind the texture and sampler to a texture unit and send the unit number to the uniform variable "tex" 
    void TexturedMaterial::setup() const {
        //TODO: (Req 7) Write this function
        //calling the setup of its parent
        TintedMaterial::setup();
        //setting the "alphaThreshold" uniform to the value in the member variable alphaThreshold
        shader->set("alphaThreshold", alphaThreshold);
        glActiveTexture(GL_TEXTURE0); // You need to activate the texture unit before binding the texture to it
        //binding the texture and sampler to a texture unit and sending the unit number to the uniform variable "tex"
        texture->bind();
        if(sampler)
            sampler->bind(0);
        shader->set("tex", 0);
    }

    // This function read the material data from a json object
    void TexturedMaterial::deserialize(const nlohmann::json& data){
        TintedMaterial::deserialize(data);
        if(!data.is_object()) return;
        alphaThreshold = data.value("alphaThreshold", 0.0f);
        texture = AssetLoader<Texture2D>::get(data.value("texture", ""));
        sampler = AssetLoader<Sampler>::get(data.value("sampler", ""));
    }

    // This function should call the setup of its parent and
    // bind the textures and sampler to texture units and send the unit numbers to the uniform variables
    // "albedo", "specular", "roughness", "ambient_occlusion" and "emission"
    void LitMaterial::setup() const {

        //calling the setup of its parent
        TintedMaterial::setup();
        
        //binding the textures and sampler to texture units and sending the unit numbers to the uniform variables
        // "albedo", "specular", "roughness", "ambient_occlusion" and "emission"

        glActiveTexture(GL_TEXTURE0); // You need to activate the texture unit before binding the texture to it
        //binding the texture and sampler to a texture unit and sending the unit number to the uniform variable "albedo"
        albedo->bind(); // You need to bind the texture to the texture unit
        if(sampler) // You need to bind the sampler to the texture unit
            sampler->bind(0);
        shader->set("albedoMap", 0); // You need to send the texture unit number to the uniform variable "albedo"

        // You need to repeat the same process for the other textures

        glActiveTexture(GL_TEXTURE1); 
        specular->bind(); 
        if(sampler) 
            sampler->bind(1);
        shader->set("specularMap", 1); 

        glActiveTexture(GL_TEXTURE2); 
        roughness->bind();
        if(sampler)
            sampler->bind(2);
        shader->set("roughnessMap", 2);

        glActiveTexture(GL_TEXTURE3); // You need to activate the texture unit before binding the texture to it
        ambient_occlusion->bind();
        if(sampler)
            sampler->bind(3);
        shader->set("ambient_occlusionMap", 3);

        glActiveTexture(GL_TEXTURE4); // You need to activate the texture unit before binding the texture to it
        emission->bind();
        if(sampler)
            sampler->bind(4);
        shader->set("emissionMap", 4);
    }

    // This function read the material data from a json object
    void LitMaterial::deserialize(const nlohmann::json& data){
        TintedMaterial::deserialize(data);
        if(!data.is_object()) return;
        if(data.contains("albedo")) {
            albedo = AssetLoader<Texture2D>::get(data.value("albedo", ""));
        } else {
            albedo = AssetLoader<Texture2D>::get("default_albedo");
        } 
        if(data.contains("specular")) {
            specular = AssetLoader<Texture2D>::get(data.value("specular", ""));
        } else {
            specular = AssetLoader<Texture2D>::get("default_specular");
        } 
        if(data.contains("roughness")) {
            roughness = AssetLoader<Texture2D>::get(data.value("roughness", ""));
        } else {
            roughness = AssetLoader<Texture2D>::get("default_roughness");
        } 
        if(data.contains("ambient_occlusion")) {
            ambient_occlusion = AssetLoader<Texture2D>::get(data.value("ambient_occlusion", ""));
        } else {
            ambient_occlusion = AssetLoader<Texture2D>::get("default_ambient_occlusion");
        } 
        if(data.contains("emission")) {
            emission = AssetLoader<Texture2D>::get(data.value("emission", ""));
        } else {
            emission = AssetLoader<Texture2D>::get("default_emission");
        } 
        sampler = AssetLoader<Sampler>::get(data.value("sampler", ""));
    }
}