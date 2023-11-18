#include "movement.hpp"
#include "../ecs/entity.hpp"
#include "../deserialize-utils.hpp"

namespace our {
    // Reads linearVelocity & angularVelocity from the given json object
    void MovementComponent::deserialize(const nlohmann::json& data){
        if(!data.is_object()) return;
        linearVelocity = data.value("linearVelocity", linearVelocity);
        glm::vec3 eulerAngularVel(0, 0, 0);
        eulerAngularVel = glm::radians(data.value("angularVelocity", eulerAngularVel));
        // from euler angular velocity to quat
        angularVelocity = glm::quat(0, eulerAngularVel.x, eulerAngularVel.y, eulerAngularVel.z);
    }
}