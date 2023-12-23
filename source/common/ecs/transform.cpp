#include "entity.hpp"
#include "../deserialize-utils.hpp"

#include <glm/gtx/euler_angles.hpp>
#include<glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include<glm/common.hpp>
#include <glm/gtc/type_ptr.inl>
namespace portal {

    // This function computes and returns a matrix that represents this transform
    // Remember that the order of transformations is: Scaling, Rotation then Translation
    // HINT: to convert euler angles to a rotation matrix, you can use glm::yawPitchRoll
    glm::mat4 Transform::toMat4() const {
        //TODO: (Req 3) Write this function
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
        float matrix[16];
        transform.getOpenGLMatrix(matrix);
        glm::mat4 transformMatrix = glm::make_mat4(matrix);
        return transformMatrix * scaleMatrix;
    }

     // Deserializes the entity data and components from a json object
    void Transform::deserialize(const nlohmann::json& data){
        glm::vec3 rotation(0,0,0), position(0,0,0);
        position = data.value("position", position);
        rotation = glm::radians(data.value("rotation", glm::degrees(rotation)));
        scale    = data.value("scale", scale);
        transform.setPosition(r3d::Vector3(position.x, position.y, position.z));
        // axis angle for yaw, pitch, roll
        glm::quat yawQuat = glm::angleAxis(rotation.y, glm::vec3(0,1,0));
        glm::quat pitchQuat = glm::angleAxis(rotation.x, glm::vec3(1,0,0));
        glm::quat rollQuat = glm::angleAxis(rotation.z, glm::vec3(0,0,1));
        glm::quat rotationQuat = yawQuat * pitchQuat * rollQuat;
        // r3d takes x,y,z, w
        transform.setOrientation(r3d::Quaternion(rotationQuat.x, rotationQuat.y, rotationQuat.z, rotationQuat.w));
    }

    r3d::Transform Transform::interpolate(const Transform& a, const Transform& b, float t) {
        r3d::Transform transform;
        transform.setPosition(r3d::Vector3(glm::mix(a.getPosition().x, b.getPosition().x, t), glm::mix(a.getPosition().y, b.getPosition().y, t), glm::mix(a.getPosition().z, b.getPosition().z, t)));
        transform.setOrientation(r3d::Quaternion(glm::mix(a.getRotation().x, b.getRotation().x, t), glm::mix(a.getRotation().y, b.getRotation().y, t), glm::mix(a.getRotation().z, b.getRotation().z, t), glm::mix(a.getRotation().w, b.getRotation().w, t)));
        return transform;
    }

    void Transform::setRotation(const r3d::Quaternion rotation) {
        transform.setOrientation(rotation);
    }

    void Transform::setRotation(const glm::quat rotation) {
        transform.setOrientation(r3d::Quaternion(rotation.x, rotation.y, rotation.z, rotation.w));
    }

    void Transform::setPosition(const r3d::Vector3 position) {
        transform.setPosition(position);
    }

    void Transform::setPosition(const glm::vec3 position) {
        transform.setPosition(r3d::Vector3(position.x, position.y, position.z));
    }

    const r3d::Quaternion& Transform::getRotation() const {
        return transform.getOrientation();
    }

    const r3d::Vector3& Transform::getPosition() const {
        return transform.getPosition();
    }
}