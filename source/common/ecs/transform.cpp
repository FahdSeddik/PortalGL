#include "entity.hpp"
#include "../deserialize-utils.hpp"

#include <glm/gtx/euler_angles.hpp>
#include<glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include<glm/common.hpp>

namespace our {

    // This function computes and returns a matrix that represents this transform
    // Remember that the order of transformations is: Scaling, Rotation then Translation
    // HINT: to convert euler angles to a rotation matrix, you can use glm::yawPitchRoll
    glm::mat4 Transform::toMat4() const {
        //TODO: (Req 3) Write this function
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
        const r3d::Quaternion& q = transform.getOrientation();
        glm::mat4 rotationMatrix = glm::toMat4(glm::quat(q.w, q.x, q.y, q.z));
        const r3d::Vector3& pos = transform.getPosition();
        glm::vec3 position(pos.x, pos.y, pos.z);
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
        return translationMatrix * rotationMatrix * scaleMatrix;
    }

     // Deserializes the entity data and components from a json object
    void Transform::deserialize(const nlohmann::json& data){
        glm::vec3 rotation(0,0,0), position(0,0,0);
        position = data.value("position", position);
        rotation = glm::radians(data.value("rotation", glm::degrees(rotation)));
        scale    = data.value("scale", scale);
        transform.setPosition(r3d::Vector3(position.x, position.y, position.z));
        transform.setOrientation(r3d::Quaternion::fromEulerAngles(rotation.x, rotation.y, rotation.z));
    }

}