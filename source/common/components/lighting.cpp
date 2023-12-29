#include "lighting.hpp"
#include "../ecs/entity.hpp"
#include "../deserialize-utils.hpp"

namespace portal{
    // Reads lighting data from the given json object
    void LightComponent::deserialize(const nlohmann::json& data){
        if(!data.is_object()) return;
        color = data.value("color", color);
        std::string typeString = data.value("lightType", "directional");
        if(typeString == "directional"){
            type = Type::Directional;
        } else if(typeString == "point"){
            type = Type::Point;
        } else if(typeString == "spot"){
            type = Type::Spot;
            // Spot light properties
            innerCutOff = glm::cos(glm::radians(data.value("innerCutOff", innerCutOff)));
            outerCutOff = glm::cos(glm::radians(data.value("outerCutOff", outerCutOff)));
        }
        // Attenuation properties (for point lights and spot lights)
        attenuation = data.value("attenuation", attenuation);
        // Directional light properties (for spot lights and directional lights)
        direction = this->getOwner()->getLocalToWorldMatrix() * glm::vec4(0, 0, 1, 0);
        // direction = data.value("direction", direction);
        worldSpacePosition = this->getOwner()->getLocalToWorldMatrix() * glm::vec4(0, 0, 0, 1);
    }


}