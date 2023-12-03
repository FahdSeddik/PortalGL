#pragma once

#include "../ecs/component.hpp"

#include <glm/glm.hpp>

namespace portal {


    class LightComponent : public Component {
    public:
        // color of the light
        glm::vec3 color = { 1, 1, 1 };
        // type of the light
        enum class Type {
            Directional,
            Point,
            Spot
        } type = Type::Directional;
        // for spot lights, the angle of the cone
        float innerCutOff = 45.0f;
        // for spot lights, the falloff of the cone
        float outerCutOff = 50.0f;
        // for directional lights and spot lights, the direction of the light in world space
        glm::vec3 direction = { 0, -1, 0 };
        // attenuation
        glm::vec3 attenuation = { 1.0f, 0.09f, 0.032f };

        // The ID of this component type is "Light"
        static std::string getID() { return "Light"; }

        // Reads lighting data from the given json object
        void deserialize(const nlohmann::json& data) override;
        
    };

}