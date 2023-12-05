#pragma once

#include "pipeline-state.hpp"
#include "../texture/texture2d.hpp"
#include "../texture/sampler.hpp"
#include "../shader/shader.hpp"

#include <glm/vec4.hpp>
#include <json/json.hpp>

namespace portal {

    // This is the base class for all the materials
    // It contains the 3 essential components required by any material
    // 1- The pipeline state when drawing objects using this material
    // 2- The shader program used to draw objects using this material
    // 3- Whether this material is transparent or not
    // Materials that send uniforms to the shader should inherit from the is material and add the required uniforms
    class Material {
    public:
        PipelineState pipelineState;
        ShaderProgram* shader;
        bool transparent;
        bool bloom;

        // This function does 2 things: setup the pipeline state and set the shader program to be used
        virtual void setup() const;
        // This function read a material from a json object
        virtual void deserialize(const nlohmann::json& data);
    };

    // This material adds a uniform for a tint (a color that will be sent to the shader)
    // An example where this material can be used is when the whole object has only color which defined by tint
    class TintedMaterial : public Material {
    public:
        glm::vec4 tint;

        void setup() const override;
        void deserialize(const nlohmann::json& data) override;
    };

    // This material adds two uniforms (besides the tint from Tinted Material)
    // The uniforms are:
    // - "tex" which is a Sampler2D. "texture" and "sampler" will be bound to it.
    // - "alphaThreshold" which defined the alpha limit below which the pixel should be discarded
    // An example where this material can be used is when the object has a texture
    class TexturedMaterial : public TintedMaterial {
    public:
        Texture2D* texture;
        Sampler* sampler;
        float alphaThreshold;

        void setup() const override;
        void deserialize(const nlohmann::json& data) override;
    };

    // This material adds 5 uniforms (besides the tint from Tinted Material)
    // The uniforms are:
    // - "albedo" which is a Sampler2D. "albedo" and "sampler" will be bound to it.
    // - "specular" which is a Sampler2D. "specular" and "sampler" will be bound to it.
    // - "roughness" which is a Sampler2D. "roughness" and "sampler" will be bound to it.
    // - "ambient_occlusion" which is a Sampler2D. "ambient_occlusion" and "sampler" will be bound to it.
    // - "emission" which is a Sampler2D. "emission" and "sampler" will be bound to it.
    class LitMaterial : public TintedMaterial {
    public:
        Sampler* sampler; 
        Texture2D* albedo;
        Texture2D* specular;
        Texture2D* roughness;
        Texture2D* ambient_occlusion;
        Texture2D* emission;
        Texture2D* metallic;
        float alphaThreshold;

        void setup() const override;
        void deserialize(const nlohmann::json& data) override;
    };

    // This Material has 2 uniforms 
    // - "tex1" which is a Sampler2D. "texture1" and "sampler1" will be bound to it.
    // - "tex2" which is a Sampler2D. "texture2" and "sampler2" will be bound to it.
    class MultiTextureMaterial : public Material {
    public:
        Texture2D* texture1;
        Texture2D* texture2;
        Sampler* sampler;
        // Sampler* sampler2;

        void setup() const override;
        void deserialize(const nlohmann::json& data) override;
    };


    // This function returns a new material instance based on the given type
    inline Material* createMaterialFromType(const std::string& type){
        if(type == "tinted"){
            return new TintedMaterial();
        } else if(type == "textured"){
            return new TexturedMaterial();
        } else if(type == "lit"){
            return new LitMaterial();
        } else {
            return new Material();
        }
    }

}